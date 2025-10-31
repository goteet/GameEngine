#pragma once
#include <vector>
#include "VulkanSwapChain.h"
#include "VulkanGraphicDevice.h"



namespace GFXI
{
    SwapChainVulkan::SwapChainVulkan(GraphicDeviceVulkan* belongsTo, VkSwapchainKHR swapChain, CommandQueueVulkan presentQueue)
        : BaseDeviceResourceVulkan(belongsTo)
        , mVulkanSwapChain(swapChain)
        , mVulkanPresentQueue(presentQueue)
    {
        VkFenceCreateInfo VukanFenceCreateInfo;
        VulkanZeroMemory(VukanFenceCreateInfo);
        VukanFenceCreateInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(GetVulkanDevice(), &VukanFenceCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &mNextFrameFence_ForTest);

        VkResult RetGetImages = vkGetSwapchainImagesKHR(GetVulkanDevice(), mVulkanSwapChain, &mNumBackbufferImages, nullptr);
        if (RetGetImages == VkResult::VK_SUCCESS && mNumBackbufferImages > 0)
        {
            mBackbufferImages.resize(mNumBackbufferImages);
            RetGetImages = vkGetSwapchainImagesKHR(GetVulkanDevice(), mVulkanSwapChain, &mNumBackbufferImages, mBackbufferImages.data());
            if (RetGetImages == VkResult::VK_SUCCESS)
            {
                for (VkImage Image : mBackbufferImages)
                {
                    VkImageViewCreateInfo ImageViewCreateInfo;
                    VkImageView ImageView;

                    VulkanZeroMemory(ImageViewCreateInfo);
                    ImageViewCreateInfo.image = Image;
                    ImageViewCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
                    ImageViewCreateInfo.format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
                    //Note: you can use constants like 0, 1 to map the channel.
                    ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                    ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                    ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                    ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                    ImageViewCreateInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
                    ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
                    ImageViewCreateInfo.subresourceRange.levelCount = 1;
                    ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
                    ImageViewCreateInfo.subresourceRange.layerCount = 1;

                    VkResult RetCreateImageView = vkCreateImageView(GetVulkanDevice(), &ImageViewCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &ImageView);

                    BackbufferContext context;

                    if (RetCreateImageView == VkResult::VK_SUCCESS)
                    {
                        VkSemaphoreCreateInfo VulkanSemaphoreCreateInfo;
                        VulkanZeroMemory(VulkanSemaphoreCreateInfo);

                        VkSemaphore Semaphore;
                        VkResult RetCreateImageSemaphore = vkCreateSemaphore(GetVulkanDevice(), &VulkanSemaphoreCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &Semaphore);
                        if (RetCreateImageSemaphore  != VkResult::VK_SUCCESS)
                        {
                            //TODO:
                        }

                        context.ImageView = ImageView;
                        context.RetrieveSemaphore = Semaphore;
                        //VkFramebufferCreateInfo FrameBufferCreateInfo;
                        //VulkanZeroMemory(FrameBufferCreateInfo);
                        //mBackbufferImageViews.renderPass;
                        //mBackbufferImageViews.attachmentCount;
                        //mBackbufferImageViews.pAttachments;
                        //mBackbufferImageViews.width;
                        //mBackbufferImageViews.height;
                        //mBackbufferImageViews.layers = 1;
                    }
                    
                    mBackbufferContext.emplace_back(context);
                }
            }
        }
    }

    SwapChainVulkan::~SwapChainVulkan()
    {
        vkDestroyFence(GetVulkanDevice(), mNextFrameFence_ForTest, GFX_VK_ALLOCATION_CALLBACK);
        for (BackbufferContext context : mBackbufferContext)
        {
            if (context.RetrieveSemaphore != nullptr)
            {
                vkDestroySemaphore(GetVulkanDevice(), context.RetrieveSemaphore, GFX_VK_ALLOCATION_CALLBACK);
            }
            if (context.ImageView != nullptr)
            {
                vkDestroyImageView(GetVulkanDevice(), context.ImageView, GFX_VK_ALLOCATION_CALLBACK);
            }
        }
        mBackbufferContext.clear();
        vkDestroySwapchainKHR(GetVulkanDevice(), mVulkanSwapChain, GFX_VK_ALLOCATION_CALLBACK);
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
        VkPresentInfoKHR PresentInfo;
        VulkanZeroMemory(PresentInfo);
        PresentInfo.waitSemaphoreCount = 0;
        PresentInfo.pWaitSemaphores;
        PresentInfo.swapchainCount = 1;
        PresentInfo.pSwapchains = &mVulkanSwapChain;
        PresentInfo.pImageIndices = &mCurrentBackbufferImageIndex;
        PresentInfo.pResults = nullptr;
        vkQueuePresentKHR(mVulkanPresentQueue.GetVulkanQueue(), &PresentInfo);

        mCurrentBackbufferImageIndex = (mCurrentBackbufferImageIndex + 1) % mNumBackbufferImages;
        //vkWaitForFences(mBelongsTo->GetVulkanDevice(), 1, &mNextFrameFence_ForTest, VK_TRUE, UINT64_MAX);
        //vkResetFences(mBelongsTo->GetVulkanDevice(), 1, &mNextFrameFence_ForTest);
        //vkQueueSubmit()
    }
}
