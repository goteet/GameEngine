#pragma once
#include "PreIncludeFiles.h"
#include "GfxInterface.h"
#include <vector>

namespace engine
{
    class TransientBufferRegistry
    {
    public:
        TransientBufferRegistry(GfxDevice* creator, GfxRenderTarget* defaultBackbufferRT, GfxDepthStencil* tempDS);
        ~TransientBufferRegistry();
        GfxRenderTarget* GetDefaultBackbufferRT() { return mDefaultBackbufferRT; }
        GfxDepthStencil* GetDefaultBackbufferDS() { return mDefaultBackbufferDSTemp; }
        GfxRenderTarget* AllocateRenderTarget(ERenderTargetFormat format, unsigned int width, unsigned int height, bool usedForShader);
        GfxDepthStencil* AllocateDepthStencil(EDepthStencilFormat format, unsigned int width, unsigned int height, bool usedForShader);
        void RecycleRenderTarget(GfxRenderTarget* texture);
        void RecycleDepthStencil(GfxDepthStencil* texture);
        void ReleaseAllBuffers();

    private:
        GfxDevice* mGfxResourceCreator;
        GfxRenderTarget* mDefaultBackbufferRT;
        GfxDepthStencil* mDefaultBackbufferDSTemp;

        std::vector<GfxRenderTarget*> mRenderTargets;
        std::vector<GfxDepthStencil*> mDepthStencils;
    };
}
