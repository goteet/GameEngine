#pragma once
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    struct GraphicModuleVulkan;

    struct SwapChainVulkan : public SwapChain
    {
        SwapChainVulkan(GraphicModuleVulkan* BelongsTo);
        virtual ~SwapChainVulkan();
        virtual void Release();
        virtual RenderTargetView* GetRenderTargetView() override final;
        virtual void Present() override final;;
    private:
        GraphicModuleVulkan* mBelongsTo;
    };
}
