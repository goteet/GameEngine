#pragma once
#include "VulkanSwapChain.h"



namespace GFXI
{
    SwapChainVulkan::SwapChainVulkan(GraphicModuleVulkan* BelongsTo)
        : mBelongsTo(BelongsTo)
    {
    }

    SwapChainVulkan::~SwapChainVulkan()
    {

    }

    void SwapChainVulkan::Release()
    {
        delete this;
    }

    RenderTargetView* SwapChainVulkan::GetRenderTargetView()
    {
        return nullptr;
    }

    void SwapChainVulkan::Present()
    {
    }
}
