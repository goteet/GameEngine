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
            resource.ReadingNodes.clear();
            resource.WritingNodes.clear();
        }
        for (const RFGNode& node : mNodes)
        {
            for (int index : node.ReadingRenderTargets)
            {
                mResources[index].ReadingNodes.emplace_back(node.Index);
            }
            for (int index : node.WritingRenderTargets)
            {
                mResources[index].WritingNodes.emplace_back(node.Index);
            }
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
                for (int intputResourceIndex : node.ReadingRenderTargets)
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
                node.ReadingRenderTargetAliasing.clear();
                node.WritingRenderTargetAliasing.clear();
                for (int nodeIndex : node.WritingRenderTargets)
                {
                    int aliasing = GetAliasingResourceIndex(nodeIndex);
                    node.WritingRenderTargetAliasing.emplace_back(aliasing);
                }
                for (int nodeIndex : node.ReadingRenderTargets)
                {
                    int aliasing = GetAliasingResourceIndex(nodeIndex);
                    node.ReadingRenderTargetAliasing.emplace_back(aliasing);
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
        node.ReadingRenderTargets.emplace_back(resource.Index);
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


    void RenderFrameGraph::BindWritingResources(GfxDeferredContext& context, RenderFrameGraph::RFGNode& node)
    {
        ASSERT(mTransientRenderTargets.size() == 0);
        ASSERT(mTransientDepthStencil == nullptr);

        TransientBufferRegistry* registry = mTransientBufferRegistry;
        GfxRenderTarget* renderTargets[100] = { 0 };
        int rtCount = 0;
        for (int renderTargetIndex : node.WritingRenderTargetAliasing)
        {
            if (renderTargetIndex == mBackbufferRTIndex)
            {
                renderTargets[rtCount] = registry->GetDefaultBackbufferRT();
            }
            else
            {
                RFGResource& depthStencilDesc = mResources[renderTargetIndex];
                ASSERT(depthStencilDesc.RenderTarget == 1);
                renderTargets[rtCount] = registry->AllocateRenderTarget(depthStencilDesc.RenderTargetFormat, depthStencilDesc.Width, depthStencilDesc.Height, depthStencilDesc.ShaderAccess);
                mTransientRenderTargets.emplace_back(renderTargets[rtCount]);
            }
            rtCount++;
        }

        int depthStencilIndex = -1;
        GfxDepthStencil* depthStencil = nullptr;
        if (node.WritingDepthStencil != -1)
        {
            depthStencilIndex = GetAliasingResourceIndex(node.WritingDepthStencil);
            if (depthStencilIndex == mBackbufferDSIndex)
            {
                depthStencil = registry->GetDefaultBackbufferDS();
            }
            else
            {
                RFGResource& depthStencilDesc = mResources[depthStencilIndex];
                ASSERT(depthStencilDesc.RenderTarget == 0);
                depthStencil = registry->AllocateDepthStencil(depthStencilDesc.DepthStencilFormat, depthStencilDesc.Width, depthStencilDesc.Height, depthStencilDesc.ShaderAccess);
                mTransientDepthStencil = depthStencil;
            }
        }

        context.SetRenderTargets(renderTargets, rtCount, depthStencil);
        rtCount = 0;
        for (auto& state : node.RenderTargetBindStates)
        {
            if (state.ClearColor)
                context.ClearRenderTarget(renderTargets[rtCount], state.ClearColorValue);
            rtCount++;
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
        if (node.mExecuteJob)
        {
            BindWritingResources(context, node);
            node.mExecuteJob(context);
            ReleaseTransientResources();
        }

        Printf("RenderPass{%s} is Running.\n", node.DebugName.c_str());
        for (int index : node.ReadingRenderTargets)
        {
            if (index == -1)
                continue;
            int aliasingIndex = GetAliasingResourceIndex(index);
            PrintSubMessage("Map Input Resource From{%s} to {%s}",
                mResources[index].DebugName.c_str(),
                mResources[aliasingIndex].DebugName.c_str());
            index = aliasingIndex;
            RFGResource& resource = mResources[index];
            PrintSubMessage("Reading Resource{%s}", resource.DebugName.c_str());
        }

        for (int index : node.WritingRenderTargets)
        {
            if (index == -1)
                continue;
            int aliasingIndex = GetAliasingResourceIndex(index);
            PrintSubMessage("Map Output Resource From{%s} to {%s}",
                mResources[index].DebugName.c_str(),
                mResources[aliasingIndex].DebugName.c_str());
            index = aliasingIndex;
            RFGResource& resource = mResources[index];
            PrintSubMessage("Writing Resource{%s}", resource.DebugName.c_str());
        }
    }
    void RenderFrameGraph::ReleaseTransientResources()
    {
        for (GfxRenderTarget* renderTarget : mTransientRenderTargets)
        {
            mTransientBufferRegistry->RecycleRenderTarget(renderTarget);
        }
        mTransientRenderTargets.clear();

        if (mTransientDepthStencil != nullptr)
        {
            mTransientBufferRegistry->RecycleDepthStencil(mTransientDepthStencil);
            mTransientDepthStencil = nullptr;
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
