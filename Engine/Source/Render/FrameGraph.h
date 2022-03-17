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
        int GetWidth() const;
        int GetHeigt() const;
        int GetFormat() const;
        bool IsRenderTarget() const;
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
        RFGResourceHandle RequestResource(const std::string& name, int width, int height, ERenderTargetFormat format);
        RFGResourceHandle RequestResource(const std::string& name, int width, int height, EDepthStencilFormat format);
        void Compile();
        void Execute(GfxDeferredContext&);
        bool BindReading(RFGRenderPass, RFGResourceHandle&);
        bool BindWriting(RFGRenderPass, RFGResourceHandle&, const ClearState& state);
        void BindOutput(RFGResourceHandle resource);
        bool AttachJob(RFGRenderPass pass, std::function<void(GfxDeferredContext&)> job);
        void CreateTestFrameGraph();

        int GetBackbufferWidth() const;
        int GetBackbufferHeight() const;
        ERenderTargetFormat GetBackbufferRTFormat() const;
        EDepthStencilFormat GetBackbufferDSFormat() const;
        int GetWidth(const RFGResourceHandle&) const;
        int GetHeigt(const RFGResourceHandle&) const;
        int GetFormat(const RFGResourceHandle&) const;
        bool IsRenderTarget(const RFGResourceHandle&) const;

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
            std::vector<int> ReadingResources;
            std::vector<int> ReadingResourcesAliasing;
            std::vector<int> WritingRenderTargets;
            std::vector<int> WritingRenderTargetAliasing;
            int WritingDepthStencil = -1;
            int WritingDepthStencilAliasing = -1;
            std::vector<ClearState> RenderTargetBindStates;
            ClearState DepthStencilBindState;

            std::function<void(GfxDeferredContext&)> mExecuteJob = nullptr;
        };

        struct RFGResource
        {
            std::string DebugName;
            int Index;
            int AliasingTo = -1;
            int Width;
            int Height;
            int RenderTarget = 1;
            int ReadingCount = 0;
            union
            {
                ERenderTargetFormat RenderTargetFormat;
                EDepthStencilFormat DepthStencilFormat;
            };
            std::vector<int> ReadingNodes;
            std::vector<int> WritingNodes;
            std::vector<int> AliasingResources;

            GfxRenderTarget* GfxRenderTargetPtr = nullptr;
            GfxDepthStencil* GfxDepthStencilPtr = nullptr;
        };

        bool MoveResource(RFGResource& from, RFGResource& to);
        RFGResource& CreateNewResource(const std::string& name, int width, int height, ERenderTargetFormat format);
        RFGResource& CreateNewResource(const std::string& name, int width, int height, EDepthStencilFormat format);
        void BindReadingResources(GfxDeferredContext& context, RFGNode& node);
        void BindWritingResources(GfxDeferredContext& context, RFGNode& node);
        void ExecuteNode(GfxDeferredContext& context, RFGNode& node);
        void ReleaseTransientResources(RFGNode& node);
        int GetAliasingResourceIndex(int);

        std::vector<RFGNode> mNodes;
        std::vector<RFGResource> mResources;
        std::vector<int> mCompiledNodeExecuteOrder;
        TransientBufferRegistry* mTransientBufferRegistry;
        int mBackbufferRTIndex;
        int mBackbufferDSIndex;
    };
}

