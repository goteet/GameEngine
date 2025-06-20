#pragma once
#include <vector>
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    struct GraphicDeviceVulkan;

    struct SwapChainVulkan : public SwapChain
    {
        SwapChainVulkan(GraphicDeviceVulkan*, VkSwapchainKHR);
        virtual ~SwapChainVulkan();
        virtual void Release();
        virtual RenderTargetView* GetRenderTargetView() override final;
        virtual void Present() override final;;
    private:
        GraphicDeviceVulkan* mBelongsTo;
        VkSwapchainKHR mVulkanSwapChain;

        std::vector<VkImageView> mImageViews;
    };
}
