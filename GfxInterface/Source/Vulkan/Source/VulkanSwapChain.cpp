#pragma once
#include <vector>
#include "VulkanSwapChain.h"
#include "VulkanGraphicDevice.h"



namespace GFXI
{
    SwapChainVulkan::SwapChainVulkan(GraphicDeviceVulkan* belongsTo, VkSwapchainKHR swapChain)
        : mBelongsTo(belongsTo)
        , mVulkanSwapChain(swapChain)
    {
        uint32_t NumImages = 0;
        VkDevice VulkanDevice = mBelongsTo->GetVulkanDevice();
        VkResult RetGetImages = vkGetSwapchainImagesKHR(VulkanDevice, mVulkanSwapChain, &NumImages, nullptr);
        if (RetGetImages == VkResult::VK_SUCCESS && NumImages > 0)
        {
            std::vector<VkImage> mSwapChainImages(NumImages);
            RetGetImages = vkGetSwapchainImagesKHR(VulkanDevice, mVulkanSwapChain, &NumImages, mSwapChainImages.data());
            if (RetGetImages == VkResult::VK_SUCCESS)
            {
                for (VkImage Image : mSwapChainImages)
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

                    VkResult RetCreateImageView = vkCreateImageView(VulkanDevice, &ImageViewCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &ImageView);
                    if (RetCreateImageView == VkResult::VK_SUCCESS)
                    {
                        mImageViews.emplace_back(ImageView);
                    }
                    else
                    {
                        mImageViews.emplace_back(nullptr);
                    }
                }
            }
        }
    }

    SwapChainVulkan::~SwapChainVulkan()
    {
        VkDevice VulkanDevice = mBelongsTo->GetVulkanDevice();
        for (VkImageView ImageView : mImageViews)
        {
            if (ImageView != nullptr)
            {
                vkDestroyImageView(VulkanDevice, ImageView, GFX_VK_ALLOCATION_CALLBACK);
            }
        }
        vkDestroySwapchainKHR(VulkanDevice, mVulkanSwapChain, GFX_VK_ALLOCATION_CALLBACK);
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
