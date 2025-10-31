#pragma once
#include <vector>
#include "VulkanInclude.h"

namespace GFXI
{
    struct GraphicDeviceVulkan;

    struct SwapChainVulkan : public SwapChain, public BaseDeviceResourceVulkan
    {
        SwapChainVulkan(GraphicDeviceVulkan*, VkSwapchainKHR, CommandQueueVulkan presentQueue);
        virtual ~SwapChainVulkan();
        virtual void Release();
        virtual RenderTargetView* GetRenderTargetView() override final;
        virtual void Present() override final;;
    private:
        VkSwapchainKHR mVulkanSwapChain;

        struct BackbufferContext
        {
            VkImageView ImageView           = nullptr;
            VkSemaphore RetrieveSemaphore   = nullptr;
        };

        std::vector<VkImage>            mBackbufferImages;
        std::vector<BackbufferContext>  mBackbufferContext;

        uint32_t mNumBackbufferImages           = 0;
        uint32_t mCurrentBackbufferImageIndex   = 0;

        VkFence mNextFrameFence_ForTest;
        CommandQueueVulkan mVulkanPresentQueue;
    };
}
