#include "FrameGraph.h"
#include <stack>
#include <queue>

namespace engine
{
    RenderFrameGraph::RenderFrameGraph()
    {
        RFGResource& BackbufferRT = CreateNewResource("backbuffer.rendertarget");
        mBackbufferRTIndex = BackbufferRT.Index;

        RFGResource& BackbufferDS = CreateNewResource("backbuffer.depthstencil");
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

    RFGResourceHandle RenderFrameGraph::RequestResource(const std::string& name)
    {
        auto it = std::find_if(mResources.begin(),
            mResources.end(),
            [&name](const RFGResource& resource) {
                return resource.DebugName == name;
            });

        return (mResources.end() == it)
            ? RFGResourceHandle{ this, CreateNewResource(name).Index }
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
        //TODO:fix this.
        bool isRenderTarget = true;
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
        RFGResourceHandle resShadowmap = RequestResource("shadow.depthmap");
        RFGResourceHandle resForwardColor = RequestResource("render.forward.color");
        RFGResourceHandle resDepthColor = RequestResource("render.depth.color");
        RFGResourceHandle resFinal = RequestResource("render.final");
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
        nodeFinal.BindReading(resDepthColor);
        nodeFinal.BindReading(resForwardColor);
        nodeFinal.BindWriting(resFinal, defaultState);

        BindOutput(resFinal);
        //BindOutput(resDepthColor);
        //BindOutput(resForwardColor);
    }

    void RenderFrameGraph::MoveResource(RFGResource& from, RFGResource& to)
    {
        from.AliasingTo = to.Index;
        if (to.AliasingResources.size() == 0)
        {
            to.AliasingResources.push_back(from.Index);
        }
        else
        {
            DEBUGPRINT("Moving a resource already aliasing: from:%d, to:%d, alising:%d",
                from.Index, to.Index, to.AliasingResources[0]);
            to.AliasingResources[0] = from.Index;
        }
    }

    RenderFrameGraph::RFGResource& RenderFrameGraph::CreateNewResource(const std::string& name)
    {
        RFGResource resource;
        resource.DebugName = name;
        resource.Index = (int)mResources.size();
        mResources.emplace_back(resource);
        return mResources.back();
    }

    void RenderFrameGraph::ExecuteNode(GfxDeferredContext& context, RenderFrameGraph::RFGNode& node)
    {
        if (node.mExecuteJob)
        {
            node.mExecuteJob(context);
        }

        Printf("RenderPass{%s} is Running.\n", node.DebugName.c_str());
        for (int index : node.ReadingRenderTargets)
        {
            if (index == -1)
                continue;
            for (int aliasingIndex = mResources[index].AliasingTo;
                aliasingIndex != -1; aliasingIndex = mResources[index].AliasingTo)
            {
                PrintSubMessage("Map Input Resource From{%s} to {%s}",
                    mResources[index].DebugName.c_str(),
                    mResources[aliasingIndex].DebugName.c_str());
                index = aliasingIndex;
            }
            RFGResource& resource = mResources[index];
            PrintSubMessage("Reading Resource{%s}", resource.DebugName.c_str());
        }

        for (int index : node.WritingRenderTargets)
        {
            if (index == -1)
                continue;
            for (int aliasingIndex = mResources[index].AliasingTo;
                aliasingIndex != -1; aliasingIndex = mResources[index].AliasingTo)
            {
                PrintSubMessage("Map Output Resource From{%s} to {%s}",
                    mResources[index].DebugName.c_str(),
                    mResources[aliasingIndex].DebugName.c_str());
                index = aliasingIndex;
            }
            RFGResource& resource = mResources[index];
            PrintSubMessage("Writing Resource{%s}", resource.DebugName.c_str());
        }
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
