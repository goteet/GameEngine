#pragma once
#include <vector>
#include <GfxInterface.h>
#include "PreIncludeFiles.h"

namespace engine
{
    class TransientBufferRegistry
    {
    public:
        TransientBufferRegistry(GFXI::GraphicDevice* creatorPtr, GFXI::RenderTargetView* defaultBackBufferRT, GFXI::DepthStencilView* tempDS);
        ~TransientBufferRegistry();
        GFXI::RenderTargetView* GetDefaultBackBufferRT() { return mDefaultBackBufferRT; }
        GFXI::DepthStencilView* GetDefaultBackBufferDS() { return mDefaultBackBufferDSTemp; }
        GFXI::RenderTargetView* AllocateRenderTarget(GFXI::RenderTargetView::EFormat format, unsigned int width, unsigned int height, bool usedByShader);
        GFXI::DepthStencilView* AllocateDepthStencil(GFXI::DepthStencilView::EFormat format, unsigned int width, unsigned int height, bool usedByShader);
        void RecycleRenderTarget(GFXI::RenderTargetView* texture);
        void RecycleDepthStencil(GFXI::DepthStencilView* texture);
        void ReleaseAllBuffers();

    private:
        GFXI::GraphicDevice* mGfxResourceDevice;
        GFXI::RenderTargetView* mDefaultBackBufferRT;
        GFXI::DepthStencilView* mDefaultBackBufferDSTemp;

        std::vector<GFXI::RenderTargetView*> mRenderTargets;
        std::vector<GFXI::DepthStencilView*> mDepthStencils;
    };
}
