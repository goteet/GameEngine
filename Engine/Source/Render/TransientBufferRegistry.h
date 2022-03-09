#pragma once
#include "PreIncludeFiles.h"
#include "GfxInterface.h"
#include <vector>

namespace engine
{
    class TransientBufferRegistry
    {
    public:
        TransientBufferRegistry(GfxDevice* creator, GfxRenderTarget* defaultBackbufferRT);
        ~TransientBufferRegistry();
        GfxRenderTarget* GetDefaultBackbufferRT() { return mDefaultBackbufferRT; }
        GfxRenderTarget* AllocateRenderTarget(ERenderTargetFormat format, unsigned int width, unsigned int height, bool usedForShader);
        GfxDepthStencil* AllocateDepthStencil(EDepthStencilFormat format, unsigned int width, unsigned int height, bool usedForShader);
        void RecycleRenderTarget(GfxRenderTarget* texture);
        void RecycleDepthStencil(GfxDepthStencil* texture);
        void ReleaseAllBuffers();

    private:
        GfxDevice* mGfxResourceCreator;
        GfxRenderTarget* mDefaultBackbufferRT;
        std::vector<GfxRenderTarget*> mRenderTargets;
        std::vector<GfxDepthStencil*> mDepthStencils;
    };
}
