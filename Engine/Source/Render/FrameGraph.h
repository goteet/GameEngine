#pragma once
#include "PreIncludeFiles.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "TransientBufferRegistry.h"
namespace engine
{
    class GfxDeferredContext;
    class RenderFrameGraph;


    struct ClearState
    {
        int ClearColor = 0;
        int ClearDepth = 0;
        int ClearStencil = 0;
        math::float4 ClearColorValue = math::float4(0, 0, 0, 1);
        float ClearDepthValue = 1.0;
        int ClearStencilValue = 0;
    };

    struct RFGResourceHandle
    {
        RenderFrameGraph* Graph;
        int Index;
    };

    struct RFGRenderPass
    {
        RenderFrameGraph* Graph;
        int Index;

        bool BindReading(RFGResourceHandle);
        bool BindWriting(RFGResourceHandle&, const ClearState& state);
        bool AttachJob(std::function<void(GfxDeferredContext&)>);
    };

    class RenderFrameGraph
    {
    public:
        RenderFrameGraph(TransientBufferRegistry* registry);
        RFGRenderPass AddRenderPass(const std::string& name);
        RFGResourceHandle RequestResource(const std::string& name, bool rendertarget);
        void Compile();
        void Execute(GfxDeferredContext&);
        bool BindReading(RFGRenderPass, RFGResourceHandle&);
        bool BindWriting(RFGRenderPass, RFGResourceHandle&, const ClearState& state);
        void BindOutput(RFGResourceHandle resource);
        bool AttachJob(RFGRenderPass pass, std::function<void(GfxDeferredContext&)> job);
        void CreateTestFrameGraph();

        template<typename RenderPassType>
        RenderPassType AddRenderPass(const std::string& name)
        {
            RFGRenderPass basePass = AddRenderPass(name);
            return RenderPassType{ this, basePass.Index };
        }
    private:
        struct RFGNode
        {
            std::string DebugName;
            int Index;
            std::vector<int> ReadingRenderTargets;
            std::vector<int> WritingRenderTargets;
            std::vector<int> ReadingRenderTargetAliasing;
            std::vector<int> WritingRenderTargetAliasing;
            int WritingDepthStencil = -1;
            std::vector<ClearState> RenderTargetBindStates;
            ClearState DepthStencilBindState;

            std::function<void(GfxDeferredContext&)> mExecuteJob = nullptr;
        };

        struct RFGResource
        {
            std::string DebugName;
            int Index;
            int AliasingTo = -1;
            int RenderTarget = 1;
            std::vector<int> ReadingNodes;
            std::vector<int> WritingNodes;
            std::vector<int> AliasingResources;
        };
        void MoveResource(RFGResource& from, RFGResource& to);
        RFGResource& CreateNewResource(const std::string& name, bool rendertarget);
        void BindWritingResources(GfxDeferredContext& context, RFGNode& node);
        void ExecuteNode(GfxDeferredContext& context, RFGNode& node);
        int GetAliasingResourceIndex(int);

        std::vector<RFGNode> mNodes;
        std::vector<RFGResource> mResources;
        std::vector<int> mCompiledNodeExecuteOrder;
        TransientBufferRegistry* mTransientBufferRegistry;
        int mBackbufferRTIndex;
        int mBackbufferDSIndex;
    };
}

