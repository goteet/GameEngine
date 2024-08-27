#pragma once
#include <vector>
#include <GfxInterface.h>
#include "PreIncludeFiles.h"

namespace engine
{
    class TransientBufferRegistry
    {
    public:
        TransientBufferRegistry(GFXI::GraphicDevice* creatorPtr, GFXI::RenderTargetView* defaultBackbufferRT, GFXI::DepthStencilView* tempDS);
        ~TransientBufferRegistry();
        GFXI::RenderTargetView* GetDefaultBackbufferRT() { return mDefaultBackbufferRT; }
        GFXI::DepthStencilView* GetDefaultBackbufferDS() { return mDefaultBackbufferDSTemp; }
        GFXI::RenderTargetView* AllocateRenderTarget(GFXI::RenderTargetView::EFormat format, unsigned int width, unsigned int height, bool usedByShader);
        GFXI::DepthStencilView* AllocateDepthStencil(GFXI::DepthStencilView::EFormat format, unsigned int width, unsigned int height, bool usedByShader);
        void RecycleRenderTarget(GFXI::RenderTargetView* texture);
        void RecycleDepthStencil(GFXI::DepthStencilView* texture);
        void ReleaseAllBuffers();

    private:
        GFXI::GraphicDevice* mGfxResourceDevice;
        GFXI::RenderTargetView* mDefaultBackbufferRT;
        GFXI::DepthStencilView* mDefaultBackbufferDSTemp;

        std::vector<GFXI::RenderTargetView*> mRenderTargets;
        std::vector<GFXI::DepthStencilView*> mDepthStencils;
    };
}
