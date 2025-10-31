#pragma once
#include <vector>
#include "GfxInterface.h"
#include "VulkanInclude.h"
#include "VulkanCommandQueue.h"

namespace GFXI
{
    struct GraphicDeviceVulkan;
    struct SwapChainVulkan;
    struct SemaphoreVulkan;

    struct RenderTargetViewVulkan : public RenderTargetView
    {
        RenderTargetViewVulkan(SwapChainVulkan* parent);
        ~RenderTargetViewVulkan() = default;
        virtual void Release() override { }

        virtual uint32_t    GetWidth()  override;
        virtual uint32_t    GetHeight() override;
        virtual bool        IsUsedByShader() override;
        virtual EFormat     GetFormat() override;
        virtual EDataUsage  GetUsage() override;

        virtual ShaderResourceView* GetShaderResourceView() override { return nullptr; }
        virtual Semaphore*          GetAvailableSemaphore() override;

        VkImage     GetVulkanImage();
        VkImageView GetVulkanImageView();

    private:
        SwapChainVulkan* mParent;
    };


    struct SwapChainVulkan : public SwapChain, public BaseDeviceResourceVulkan
    {
        SwapChainVulkan(GraphicDeviceVulkan*, VkSwapchainKHR, VkSurfaceKHR, VulkanCommandQueue presentQueue
            , RenderTargetView::EFormat format, uint32_t width, uint32_t height);
        virtual ~SwapChainVulkan();
        virtual void Release() override;
        virtual RenderTargetView* GetRenderTargetView() override final;
        virtual void Present() override final;

        SemaphoreVulkan*    GetCurrentImageAvailableSemaphore();
        VkImage             GetCurrentImage();
        VkImageView         GetCurrentImageView();
        RenderTargetView::EFormat GetImageFormat() { return mBackBufferImageFormat; }
        uint32_t GetImageWidth() { return mBackBufferImageWidth; }
        uint32_t GetImageHeight() { return mBackBufferImageHeight; }
        
    private:
        struct BackBufferContext
        {
            VkImage     Image       = VK_NULL_HANDLE;
            VkImageView ImageView   = VK_NULL_HANDLE;
        };

        struct FrameAcquiredContext
        {
            VkSemaphore         VulkanSemaphore;
            SemaphoreVulkan*    Semaphore   = nullptr;
            uint32_t            ImageIndex  = 0xFFFFFFFF;
        };

        void AcquireFrameImageIndex(FrameAcquiredContext& out);

        VkSwapchainKHR  mVulkanSwapChain;
        VkSurfaceKHR    mVulkanSurface;
        RenderTargetViewVulkan mRenderTargetView;
        VulkanCommandQueue mVulkanPresentQueue;

        std::vector<BackBufferContext>      mBackBufferImages;
        std::vector<FrameAcquiredContext>   mFrameAcquiredContext;

        RenderTargetView::EFormat mBackBufferImageFormat;
        uint32_t mBackBufferImageWidth  = 0;
        uint32_t mBackBufferImageHeight = 0;
        uint32_t mNumBackBufferImages   = 0;
        uint32_t mLoopFrameIndex        = 0;
    };
}
