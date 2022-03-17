#include "FrameGraph.h"
#include <stack>
#include <queue>
#include "GfxInterface.h"

namespace engine
{
    int RFGResourceHandle::GetWidth() const
    {
        return Graph->GetWidth(*this);
    }
    int RFGResourceHandle::GetHeigt() const
    {
        return Graph->GetHeigt(*this);
    }
    int RFGResourceHandle::GetFormat() const
    {
        return Graph->GetFormat(*this);
    }
    bool RFGResourceHandle::IsRenderTarget() const
    {
        return Graph->IsRenderTarget(*this);
    }

    RenderFrameGraph::RenderFrameGraph(TransientBufferRegistry* registry)
        : mTransientBufferRegistry(registry)
    {
        RFGResource& BackbufferRT = CreateNewResource("backbuffer.rendertarget",
            registry->GetDefaultBackbufferRT()->GetWidth(),
            registry->GetDefaultBackbufferRT()->GetHeight(),
            registry->GetDefaultBackbufferRT()->mRenderTargetFormat);

        mBackbufferRTIndex = BackbufferRT.Index;

        RFGResource& BackbufferDS = CreateNewResource("backbuffer.depthstencil",
            registry->GetDefaultBackbufferDS()->GetWidth(),
            registry->GetDefaultBackbufferDS()->GetHeight(),
            registry->GetDefaultBackbufferDS()->mDepthStencilFormat);
        mBackbufferDSIndex = BackbufferDS.Index;
    }

    RFGRenderPass RenderFrameGraph::AddRenderPass(const std::string& name)
    {
        RFGNode node;
        node.DebugName = name;
        node.Index = (int)mNodes.size();
        mNodes.emplace_back(node);
        return RFGRenderPass{ this, node.Index };
    }

    RFGResourceHandle RenderFrameGraph::RequestResource(const std::string& name, int width, int height, ERenderTargetFormat format)
    {
        auto it = std::find_if(mResources.begin(), mResources.end(),
            [&](const RenderFrameGraph::RFGResource& resource) {
                return resource.DebugName == name
                    && resource.RenderTarget
                    && resource.Width == width
                    && resource.Height == height
                    && resource.RenderTargetFormat == format;
            });

        return (mResources.end() == it)
            ? RFGResourceHandle{ this, CreateNewResource(name, width, height, format).Index }
        : RFGResourceHandle{ this, it->Index };
    }

    RFGResourceHandle RenderFrameGraph::RequestResource(const std::string& name, int width, int height, EDepthStencilFormat format)
    {
        auto it = std::find_if(mResources.begin(), mResources.end(),
            [&](const RenderFrameGraph::RFGResource& resource) {
                return resource.DebugName == name
                    && !resource.RenderTarget
                    && resource.Width == width
                    && resource.Height == height
                    && resource.RenderTargetFormat == format;
            });

        return (mResources.end() == it)
            ? RFGResourceHandle{ this, CreateNewResource(name, width, height, format).Index }
        : RFGResourceHandle{ this, it->Index };
    }

    void RenderFrameGraph::Compile()
    {
        for (RFGResource& resource : mResources)
        {
            ASSERT(resource.ReadingCount == 0);
            resource.ReadingCount = 0;
            resource.ReadingNodes.clear();
            resource.WritingNodes.clear();
        }
        for (const RFGNode& node : mNodes)
        {
            for (int index : node.WritingRenderTargets)
            {
                mResources[index].WritingNodes.emplace_back(node.Index);
            }

            ASSERT(node.WritingDepthStencil != -1)
            mResources[node.WritingDepthStencil].WritingNodes.emplace_back(node.Index);
        }

        std::queue<int> floodResourceQueue;
        std::stack<int> executeNodes;
        floodResourceQueue.emplace(mBackbufferRTIndex);

        while (!floodResourceQueue.empty())
        {
            int resourceIndex = floodResourceQueue.front();
            floodResourceQueue.pop();

            while (mResources[resourceIndex].AliasingResources.size() > 0)
            {
                resourceIndex = mResources[resourceIndex].AliasingResources.back();
            }

            for (int nodeIndex : mResources[resourceIndex].WritingNodes)
            {
                const RFGNode& node = mNodes[nodeIndex];
                executeNodes.push(node.Index);

                //TODO: Avoid duplicated index
                //check early when binding
                for (int intputResourceIndex : node.ReadingResources)
                {
                    if (intputResourceIndex != -1)
                    {
                        floodResourceQueue.push(intputResourceIndex);
                    }
                }
            }
        }

        while (!executeNodes.empty())
        {
            int nodeIndex = executeNodes.top();
            executeNodes.pop();
            auto it = std::find(
                mCompiledNodeExecuteOrder.begin(),
                mCompiledNodeExecuteOrder.end(),
                nodeIndex);

            if (it == mCompiledNodeExecuteOrder.end())
            {
                mCompiledNodeExecuteOrder.emplace_back(nodeIndex);

                RFGNode& node = mNodes[nodeIndex];
                int aliasing = GetAliasingResourceIndex(node.Index);
                node.ReadingResourcesAliasing.clear();
                node.WritingRenderTargetAliasing.clear();
                node.WritingDepthStencilAliasing = -1;
                for (int nodeIndex : node.WritingRenderTargets)
                {
                    int aliasing = GetAliasingResourceIndex(nodeIndex);
                    node.WritingRenderTargetAliasing.emplace_back(aliasing);
                }
                for (int nodeIndex : node.ReadingResources)
                {
                    int aliasing = GetAliasingResourceIndex(nodeIndex);
                    node.ReadingResourcesAliasing.emplace_back(aliasing);
                    RFGResource& resource = mResources[aliasing];
                    resource.ReadingNodes.emplace_back(node.Index);
                    resource.ReadingCount++;
                }

                if (node.WritingDepthStencil != -1)
                {
                    int aliasing = GetAliasingResourceIndex(node.WritingDepthStencil);
                    node.WritingDepthStencilAliasing = aliasing;
                }
            }
        }
    }

    void RenderFrameGraph::Execute(GfxDeferredContext& context)
    {
        for (int nodeIndex : mCompiledNodeExecuteOrder)
        {
            RFGNode& node = mNodes[nodeIndex];
            ExecuteNode(context, node);
        }
    }

    bool RenderFrameGraph::BindReading(RFGRenderPass pass, RFGResourceHandle& resource)
    {
        if (pass.Graph != this || resource.Graph != this)
            return false;

        RFGNode& node = mNodes[pass.Index];
        node.ReadingResources.emplace_back(resource.Index);
        return true;
    }

    bool RenderFrameGraph::BindWriting(RFGRenderPass pass, RFGResourceHandle& resource, const ClearState& state)
    {
        if (pass.Graph != this || resource.Graph != this)
            return false;

        RFGNode& node = mNodes[pass.Index];
        bool isRenderTarget = mResources[resource.Index].RenderTarget == 1;
        if (isRenderTarget)
        {
            node.WritingRenderTargets.emplace_back(resource.Index);
            node.RenderTargetBindStates.emplace_back(state);
        }
        else
        {
            node.WritingDepthStencil = resource.Index;
            node.DepthStencilBindState = state;
        }
        return true;
    }

    void RenderFrameGraph::BindOutput(RFGResourceHandle resource)
    {
        if (this == resource.Graph)
        {
            RFGResource& resOutput = mResources[resource.Index];
            if (resOutput.RenderTarget)
            {
                RFGResource& resBackbuffer = mResources[mBackbufferRTIndex];
                MoveResource(resOutput, resBackbuffer);
            }
            else
            {
                RFGResource& resBackbuffer = mResources[mBackbufferDSIndex];
                MoveResource(resOutput, resBackbuffer);
            }
        }
    }

    bool RenderFrameGraph::AttachJob(RFGRenderPass pass, std::function<void(GfxDeferredContext&)> job)
    {
        if (pass.Graph != this)
            return false;

        RFGNode& node = mNodes[pass.Index];
        node.mExecuteJob = std::move(job);

        return true;
    }


    void RenderFrameGraph::CreateTestFrameGraph()
    {
        RFGResourceHandle resShadowmap = RequestResource("shadow.depthmap", GetBackbufferWidth(), GetBackbufferHeight(), GetBackbufferDSFormat());
        RFGResourceHandle resForwardColor = RequestResource("render.forward.color", GetBackbufferWidth(), GetBackbufferHeight(), GetBackbufferRTFormat());
        RFGResourceHandle resDepthColor = RequestResource("render.depth.color", GetBackbufferWidth(), GetBackbufferHeight(), GetBackbufferRTFormat());
        RFGResourceHandle resFinal = RequestResource("render.final", GetBackbufferWidth(), GetBackbufferHeight(), GetBackbufferRTFormat());
        RFGRenderPass nodeShadowMapDepth = AddRenderPass("render.shadowmap");
        RFGRenderPass nodeForward = AddRenderPass("render.forward");
        RFGRenderPass nodeDepthToRT = AddRenderPass("render.depth2rt");
        RFGRenderPass nodeFinal = AddRenderPass("render.final");
        ClearState defaultState;
        nodeShadowMapDepth.BindWriting(resShadowmap, defaultState);
        nodeForward.BindReading(resShadowmap);
        nodeForward.BindWriting(resForwardColor, defaultState);
        nodeDepthToRT.BindReading(resShadowmap);
        nodeDepthToRT.BindWriting(resDepthColor, defaultState);
        //nodeFinal.BindReading(resDepthColor);
        nodeFinal.BindReading(resForwardColor);
        nodeFinal.BindWriting(resFinal, defaultState);

        BindOutput(resFinal);
        //BindOutput(resDepthColor);
        //BindOutput(resForwardColor);
    }

    int RenderFrameGraph::GetBackbufferWidth() const
    {
        return mTransientBufferRegistry->GetDefaultBackbufferRT()->GetWidth();
    }

    int RenderFrameGraph::GetBackbufferHeight() const
    {
        return mTransientBufferRegistry->GetDefaultBackbufferRT()->GetHeight();
    }

    ERenderTargetFormat RenderFrameGraph::GetBackbufferRTFormat() const
    {
        return mTransientBufferRegistry->GetDefaultBackbufferRT()->mRenderTargetFormat;
    }

    EDepthStencilFormat RenderFrameGraph::GetBackbufferDSFormat() const
    {
        return mTransientBufferRegistry->GetDefaultBackbufferDS()->mDepthStencilFormat;
    }

    int RenderFrameGraph::GetWidth(const RFGResourceHandle& handle) const
    {
        if (handle.Index == mBackbufferRTIndex)
        {
            return GetBackbufferWidth();
        }
        else if (handle.Index == mBackbufferDSIndex)
        {
            return mTransientBufferRegistry->GetDefaultBackbufferDS()->GetWidth();
        }
        else
        {
            return 0;
        }
    }

    int RenderFrameGraph::GetHeigt(const RFGResourceHandle& handle) const
    {
        if (handle.Index == mBackbufferRTIndex)
        {
            return GetBackbufferHeight();
        }
        else if (handle.Index == mBackbufferDSIndex)
        {
            return mTransientBufferRegistry->GetDefaultBackbufferDS()->GetHeight();
        }
        else
        {
            return 0;
        }
    }

    int RenderFrameGraph::GetFormat(const RFGResourceHandle& handle) const
    {
        if (handle.Index == mBackbufferRTIndex)
        {
            return GetBackbufferRTFormat();
        }
        else if (handle.Index == mBackbufferDSIndex)
        {
            return mTransientBufferRegistry->GetDefaultBackbufferDS()->mDepthStencilFormat;
        }
        else
        {
            return 0;
        }
    }

    bool RenderFrameGraph::IsRenderTarget(const RFGResourceHandle& handle) const
    {
        return mResources[handle.Index].RenderTarget != 0;
    }


    bool RenderFrameGraph::MoveResource(RFGResource& from, RFGResource& to)
    {
        from.AliasingTo = to.Index;
        if (to.AliasingResources.size() == 0)
        {
            //TODO:create a internal blit render pass.
            ASSERT(from.Width == to.Width && from.Height == to.Height && from.RenderTarget == to.RenderTarget);
            if (to.RenderTarget)
            {
                ASSERT(from.RenderTargetFormat == to.RenderTargetFormat);
            }
            else
            {
                ASSERT(from.DepthStencilFormat == to.DepthStencilFormat);
            }
            to.AliasingResources.push_back(from.Index);
            return true;
        }
        else
        {
            DEBUGPRINT("Moving a resource already aliasing: from:%d, to:%d, alising:%d",
                from.Index, to.Index, to.AliasingResources[0]);
            to.AliasingResources[0] = from.Index;
            return false;
        }
    }

    RenderFrameGraph::RFGResource& RenderFrameGraph::CreateNewResource(const std::string& name, int width, int height, ERenderTargetFormat format)
    {
        RFGResource resource;
        resource.DebugName = name;
        resource.Index = (int)mResources.size();
        resource.Width = width;
        resource.Height = height;
        resource.RenderTarget = 1;
        resource.RenderTargetFormat = format;
        mResources.emplace_back(resource);
        return mResources.back();
    }

    RenderFrameGraph::RFGResource& RenderFrameGraph::CreateNewResource(const std::string& name, int width, int height, EDepthStencilFormat format)
    {
        RFGResource resource;
        resource.DebugName = name;
        resource.Index = (int)mResources.size();
        resource.Width = width;
        resource.Height = height;
        resource.RenderTarget = 0;
        resource.DepthStencilFormat = format;
        mResources.emplace_back(resource);
        return mResources.back();
    }

    void RenderFrameGraph::BindReadingResources(GfxDeferredContext& context, RFGNode& node)
    {
        ID3D11ShaderResourceView* views[16];
        int viewCount = 0;
        for (int renderTargetIndex : node.ReadingResourcesAliasing)
        {
            RFGResource& resource = mResources[renderTargetIndex];
            resource.ReadingCount--;
            if (resource.RenderTarget)
            {
                views[viewCount] = resource.GfxRenderTargetPtr == nullptr ? nullptr
                    : resource.GfxRenderTargetPtr->mShaderResourceView.Get();
            }
            else
            {
                views[viewCount] = resource.GfxDepthStencilPtr == nullptr ? nullptr
                    : resource.GfxDepthStencilPtr->mShaderResourceView.Get();
            }
            viewCount++;
        }

        context.mGfxDeviceContext->PSSetShaderResources(0, viewCount, views);
    }

    void RenderFrameGraph::BindWritingResources(GfxDeferredContext& context, RenderFrameGraph::RFGNode& node)
    {
        TransientBufferRegistry* registry = mTransientBufferRegistry;
        GfxRenderTarget* renderTargets[32] = { 0 };
        int rtCount = 0;
        for (int renderTargetIndex : node.WritingRenderTargetAliasing)
        {
            if (renderTargetIndex == mBackbufferRTIndex)
            {
                renderTargets[rtCount] = registry->GetDefaultBackbufferRT();
            }
            else
            {
                RFGResource& resRenderTarget = mResources[renderTargetIndex];
                ASSERT(resRenderTarget.RenderTarget == 1);
                renderTargets[rtCount] = registry->AllocateRenderTarget(resRenderTarget.RenderTargetFormat, resRenderTarget.Width, resRenderTarget.Height, resRenderTarget.ReadingCount > 0);
                resRenderTarget.GfxRenderTargetPtr = renderTargets[rtCount];
            }
            rtCount++;
        }

        GfxDepthStencil* depthStencil = nullptr;
        if (node.WritingDepthStencil != -1)
        {
            node.WritingDepthStencilAliasing = GetAliasingResourceIndex(node.WritingDepthStencil);
            if (node.WritingDepthStencilAliasing == mBackbufferDSIndex)
            {
                depthStencil = registry->GetDefaultBackbufferDS();
            }
            else
            {
                RFGResource& resDepthStencil = mResources[node.WritingDepthStencilAliasing];
                ASSERT(resDepthStencil.RenderTarget == 0);
                depthStencil = registry->AllocateDepthStencil(resDepthStencil.DepthStencilFormat, resDepthStencil.Width, resDepthStencil.Height, resDepthStencil.ReadingCount > 0);
                resDepthStencil.GfxDepthStencilPtr = depthStencil;
            }
        }
        context.SetRenderTargets(renderTargets, rtCount, depthStencil);

        for (int index = 0; index < node.WritingRenderTargetAliasing.size(); index++)
        {
            const auto& state =node.RenderTargetBindStates[index];
            if (state.ClearColor)
            {
                context.ClearRenderTarget(renderTargets[index], state.ClearColorValue);
            }
        }

        if (depthStencil != nullptr)
        {
            context.ClearDepthStencil(depthStencil,
                node.DepthStencilBindState.ClearStencil,
                node.DepthStencilBindState.ClearDepth,
                node.DepthStencilBindState.ClearDepthValue,
                node.DepthStencilBindState.ClearStencilValue);
        }
    }

    void RenderFrameGraph::ExecuteNode(GfxDeferredContext& context, RenderFrameGraph::RFGNode& node)
    {
        BindReadingResources(context, node);
        BindWritingResources(context, node);
        if (node.mExecuteJob)
        {
            node.mExecuteJob(context);
        }
        ReleaseTransientResources(node);
    }
    void RenderFrameGraph::ReleaseTransientResources(RenderFrameGraph::RFGNode& node)
    {
        for (int index : node.ReadingResourcesAliasing)
        {
            RFGResource& resource = mResources[index];

            if (resource.ReadingCount == 0)
            {
                if (resource.RenderTarget)
                {
                    if (resource.GfxRenderTargetPtr)
                    {
                        mTransientBufferRegistry->RecycleRenderTarget(resource.GfxRenderTargetPtr);
                        resource.GfxRenderTargetPtr = nullptr;
                    }
                }
                else
                {
                    if (resource.GfxDepthStencilPtr)
                    {
                        mTransientBufferRegistry->RecycleDepthStencil(resource.GfxDepthStencilPtr);
                        resource.GfxDepthStencilPtr = nullptr;
                    }
                }
            }
        }

        for (int index : node.WritingRenderTargetAliasing)
        {
            RFGResource& resource = mResources[index];
            if (resource.ReadingCount == 0 && resource.GfxRenderTargetPtr)
            {
                mTransientBufferRegistry->RecycleRenderTarget(resource.GfxRenderTargetPtr);
                resource.GfxRenderTargetPtr = nullptr;
            }
        }

        if (node.WritingDepthStencilAliasing != -1)
        {
            RFGResource& resource = mResources[node.WritingDepthStencilAliasing];
            if (resource.ReadingCount == 0 && resource.GfxDepthStencilPtr)
            {
                mTransientBufferRegistry->RecycleDepthStencil(resource.GfxDepthStencilPtr);
                resource.GfxDepthStencilPtr = nullptr;
            }
        }
    }

    int RenderFrameGraph::GetAliasingResourceIndex(int index)
    {
        for (int aliasingIndex = mResources[index].AliasingTo;
            aliasingIndex != -1; aliasingIndex = mResources[index].AliasingTo)
        {
            index = aliasingIndex;
        }
        return index;
    }

    bool RFGRenderPass::BindReading(RFGResourceHandle resource)
    {
        if (Graph != resource.Graph)
            return false;

        return Graph->BindReading(*this, resource);
    }
    bool RFGRenderPass::BindWriting(RFGResourceHandle& resource, const ClearState& state)
    {
        if (Graph != resource.Graph)
            return false;

        return Graph->BindWriting(*this, resource, state);
    }
    bool RFGRenderPass::AttachJob(std::function<void(GfxDeferredContext&)> job)
    {
        return Graph->AttachJob(*this, std::move(job));
    }
}
