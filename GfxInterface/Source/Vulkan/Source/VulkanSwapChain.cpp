#pragma once
#include <vector>
#include "VulkanSwapChain.h"
#include "VulkanGraphicDevice.h"
#include "VulkanSemaphore.h"

namespace GFXI
{
    SwapChainVulkan::SwapChainVulkan(GraphicDeviceVulkan* belongsTo
        , VkSwapchainKHR swapChain, VkSurfaceKHR surface, VulkanCommandQueue presentQueue
        , RenderTargetView::EFormat format, uint32_t width, uint32_t height)
        : BaseDeviceResourceVulkan(belongsTo)
        , mVulkanSwapChain(swapChain)
        , mVulkanSurface(surface)
        , mRenderTargetView(this)
        , mBackBufferImageFormat(format)
        , mBackBufferImageWidth(width)
        , mBackBufferImageHeight(height)
        , mVulkanPresentQueue(presentQueue)
        , mLoopFrameIndex(0)
    {
        VkFenceCreateInfo VukanFenceCreateInfo;
        VulkanZeroMemory(VukanFenceCreateInfo);
        VukanFenceCreateInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;

       VkResult RetGetImages = vkGetSwapchainImagesKHR(GetVulkanDevice(), mVulkanSwapChain, &mNumBackBufferImages, nullptr);
       if (RetGetImages == VkResult::VK_SUCCESS && mNumBackBufferImages > 0)
       {
           std::vector<VkImage> backBufferImages(mNumBackBufferImages);
           RetGetImages = vkGetSwapchainImagesKHR(GetVulkanDevice(), mVulkanSwapChain, &mNumBackBufferImages, backBufferImages.data());
           if (RetGetImages == VkResult::VK_SUCCESS)
           {
               for (VkImage image : backBufferImages)
               {
                   VkImageView imageView = VK_NULL_HANDLE;
       
                   VkImageViewCreateInfo ImageViewCreateInfo;
                   VulkanZeroMemory(ImageViewCreateInfo);
                   ImageViewCreateInfo.image = image;
                   ImageViewCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
                   ImageViewCreateInfo.format = VulkanRenderTargetFormatMapping[static_cast<uint32_t>(mBackBufferImageFormat)];
                   //Note: you can use constants like 0, 1 to map the channel.
                   ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
                   ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
                   ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
                   ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
                   ImageViewCreateInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
                   ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
                   ImageViewCreateInfo.subresourceRange.levelCount = 1;
                   ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
                   ImageViewCreateInfo.subresourceRange.layerCount = 1;
       
                   VkResult RetCreateImageView = vkCreateImageView(GetVulkanDevice(), &ImageViewCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &(imageView));
       
                   if (RetCreateImageView == VkResult::VK_SUCCESS)
                   {
                       //TODO: Create Framebuffer.
                       //context.RetrieveSemaphore = Semaphore;
                       //VkFramebufferCreateInfo FrameBufferCreateInfo;
                       //VulkanZeroMemory(FrameBufferCreateInfo);
                       //mBackBufferImageViews.renderPass;
                       //mBackBufferImageViews.attachmentCount;
                       //mBackBufferImageViews.pAttachments;
                       //mBackBufferImageViews.width;
                       //mBackBufferImageViews.height;
                       //mBackBufferImageViews.layers = 1;
                   }
                   else
                   {
                       imageView = VK_NULL_HANDLE;
                       DebugBreak();
                   }
                   
                   mBackBufferImages.emplace_back(BackBufferContext{ image, imageView });
               }
       
               for (uint32_t index = 0; index < mNumBackBufferImages; index++)
               {
                   FrameAcquiredContext context;
       
                   VkSemaphoreCreateInfo semaphoreCreateInfo;
                   VkSemaphoreTypeCreateInfo binarySemaphoreCreateInfo;
                   VulkanZeroMemory(semaphoreCreateInfo);
                   VulkanZeroMemory(binarySemaphoreCreateInfo);
                   //semaphoreCreateInfo.pNext = &binarySemaphoreCreateInfo;
                   binarySemaphoreCreateInfo.semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_BINARY;
                   binarySemaphoreCreateInfo.initialValue  = 0;
       
                   VkSemaphore semahpore;
                   VkResult RetCreateImageSemaphore = vkCreateSemaphore(GetVulkanDevice(), &semaphoreCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &semahpore);
                   if (RetCreateImageSemaphore == VkResult::VK_SUCCESS)
                   {
                       context.Semaphore = new SemaphoreVulkan(GetParent(), semahpore, false);
                   }
       
                   mFrameAcquiredContext.emplace_back(context);
               }
           }

           AcquireFrameImageIndex(mFrameAcquiredContext[mLoopFrameIndex]);
       }
    }

    SwapChainVulkan::~SwapChainVulkan()
    {
        vkDeviceWaitIdle(GetVulkanDevice());

        std::vector<SemaphoreVulkan*> waitSemaphores;
        for (FrameAcquiredContext& context : mFrameAcquiredContext)
        {
            if (context.Semaphore != nullptr)
            {
                waitSemaphores.emplace_back(context.Semaphore);
            }
        }
        
        for (BackBufferContext& context : mBackBufferImages)
        {
            if (context.ImageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(GetVulkanDevice(), context.ImageView, GFX_VK_ALLOCATION_CALLBACK);
            }
        }
        
        for (SemaphoreVulkan* semaphore : waitSemaphores)
        {
            delete semaphore;
        }
        
        mBackBufferImages.clear();
        mFrameAcquiredContext.clear();
        vkDestroySwapchainKHR(GetVulkanDevice(), mVulkanSwapChain, GFX_VK_ALLOCATION_CALLBACK);
        vkDestroySurfaceKHR(GetParent()->GetVulkanInstance(), mVulkanSurface, GFX_VK_ALLOCATION_CALLBACK);
    }

    void SwapChainVulkan::Release()
    {
        delete this;
    }

    RenderTargetView* SwapChainVulkan::GetRenderTargetView()
    {
        return &mRenderTargetView;
    }

    void SwapChainVulkan::Present()
    {
        //TODO: We need to know Command existance and their finish semaphores.
        std::vector<VkSemaphore> pendingSemaphores;
        std::vector<VkFence> pendingFences;
        for (GraphicCommandBufferVulkan* buffer : GetParent()->mGraphicQueue.PendingCommands)
        {
            pendingSemaphores.emplace_back(buffer->GetCommandBufferWrapper().FinishSemaphore);
            pendingFences.emplace_back(buffer->GetCommandBufferWrapper().FinishFence);
        }

        uint32_t    imageIndex = mFrameAcquiredContext[mLoopFrameIndex].ImageIndex;
        VkPresentInfoKHR presentInfo;
        VulkanZeroMemory(presentInfo);
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(pendingSemaphores.size());
        presentInfo.pWaitSemaphores = pendingSemaphores.data();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &mVulkanSwapChain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;
        VkResult retPresent = vkQueuePresentKHR(mVulkanPresentQueue.GetVulkanQueue(), &presentInfo);
        if (retPresent != VkResult::VK_SUCCESS)
        {
            DebugBreak();
        }

        vkWaitForFences(GetVulkanDevice(), static_cast<uint32_t>(pendingFences.size()), pendingFences.data(), VK_TRUE, UINT64_MAX);
        vkResetFences(GetVulkanDevice(), static_cast<uint32_t>(pendingFences.size()), pendingFences.data());

        for (GraphicCommandBufferVulkan* buffer : GetParent()->mGraphicQueue.PendingCommands)
        {
            buffer->ClearSemaphores();
        }
        GetParent()->mGraphicQueue.PendingCommands.clear();

        mLoopFrameIndex = (mLoopFrameIndex + 1) % mNumBackBufferImages;
        AcquireFrameImageIndex(mFrameAcquiredContext[mLoopFrameIndex]);
    }

    SemaphoreVulkan* SwapChainVulkan::GetCurrentImageAvailableSemaphore()
    {
        return mFrameAcquiredContext[mLoopFrameIndex].Semaphore;
    }

    VkImage SwapChainVulkan::GetCurrentImage()
    {
        uint32_t backBufferImageIndex = mFrameAcquiredContext[mLoopFrameIndex].ImageIndex;
        return mBackBufferImages[backBufferImageIndex].Image;
    }

    VkImageView SwapChainVulkan::GetCurrentImageView()
    {
        uint32_t backBufferImageIndex = mFrameAcquiredContext[mLoopFrameIndex].ImageIndex;
        return mBackBufferImages[backBufferImageIndex].ImageView;
    }


    void SwapChainVulkan::AcquireFrameImageIndex(FrameAcquiredContext& out)
    {
        VkResult retAcquireImageIndex = vkAcquireNextImageKHR(GetVulkanDevice(), mVulkanSwapChain, UINT64_MAX
            , out.Semaphore->GetVulkanSemaphore(), VK_NULL_HANDLE, &out.ImageIndex);

        if (retAcquireImageIndex != VkResult::VK_SUCCESS)
        {
            DebugBreak();
        }
    }

    RenderTargetViewVulkan::RenderTargetViewVulkan(SwapChainVulkan* parent)
        : mParent(parent)
    {

    }

    uint32_t RenderTargetViewVulkan::GetWidth()
    {
        return mParent->GetImageWidth();
    }

    uint32_t RenderTargetViewVulkan::GetHeight()
    {
        return mParent->GetImageHeight();
    }

    bool RenderTargetViewVulkan::IsUsedByShader()
    {
        return true;
    }

    GFXI::RenderTargetView::EFormat RenderTargetViewVulkan::GetFormat()
    {
        return mParent->GetImageFormat();
    }

    EDataUsage RenderTargetViewVulkan::GetUsage()
    {
        return EDataUsage::Default;
    }
    Semaphore* RenderTargetViewVulkan::GetAvailableSemaphore()
    {
        return mParent->GetCurrentImageAvailableSemaphore();
    }
    VkImage RenderTargetViewVulkan::GetVulkanImage()
    {
        return mParent->GetCurrentImage();
    }
    VkImageView RenderTargetViewVulkan::GetVulkanImageView()
    {
        return mParent->GetCurrentImageView();
    }
}
