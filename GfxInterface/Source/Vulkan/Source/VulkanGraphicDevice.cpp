#include "VulkanGraphicDevice.h"
#include "VulkanGraphicModule.h"
#include "VulkanSwapChain.h"


namespace GFXI
{
    GraphicDeviceVulkan::GraphicDeviceVulkan(GraphicModuleVulkan* BelongsTo, VkPhysicalDevice VulkanPhysicalDevice, VkDevice VulkanDevice,
        uint32_t GraphicQueueFamilyIndex, uint32_t GraphicQueueIndex,
        uint32_t ComputeQueueFamilyIndex, uint32_t ComputeQueueIndex,
        uint32_t TransferQueueFamilyIndex, uint32_t TransferQueueIndex)
        : mBelongsTo(BelongsTo)
        , mVulkanPhysicalDevice(VulkanPhysicalDevice)
        , mVulkanDevice(VulkanDevice)
        , mGraphicQueueFamilyIndex(GraphicQueueFamilyIndex),   mGraphicQueueIndex(GraphicQueueIndex)
        , mComputeQueueFamilyIndex(ComputeQueueFamilyIndex),   mComputeQueueIndex(ComputeQueueIndex)
        , mTransferQueueFamilyIndex(TransferQueueFamilyIndex), mTransferQueueIndex(TransferQueueIndex)
    {
        vkGetDeviceQueue(VulkanDevice, GraphicQueueFamilyIndex,  GraphicQueueIndex,  &mVulkanGraphicQueue);
        vkGetDeviceQueue(VulkanDevice, ComputeQueueFamilyIndex,  ComputeQueueIndex,  &mVulkanComputeQueue);
        vkGetDeviceQueue(VulkanDevice, TransferQueueFamilyIndex, TransferQueueIndex, &mVulkanTransferQueue);
    }

    GraphicDeviceVulkan::~GraphicDeviceVulkan()
    {
        vkDestroyDevice(mVulkanDevice, GFX_VK_ALLOCATION_CALLBACK);
    }
    void GraphicDeviceVulkan::Release()
    {
        delete this;
    }

    SwapChain* GraphicDeviceVulkan::CreateSwapChain(void* windowHandle, int windowWidth, int windowHeight, bool isFullscreen)
    {
        VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo;
        VulkanZeroMemory(SurfaceCreateInfo);
        SurfaceCreateInfo.hwnd = static_cast<HWND>(windowHandle);
        SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

        VkSurfaceKHR Surface = nullptr;
        VkResult RetCreateSurface = vkCreateWin32SurfaceKHR(mBelongsTo->GetInstance(), &SurfaceCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &Surface);

        if (RetCreateSurface == VkResult::VK_SUCCESS)
        {

            //TODO: There will be some DeviceQueueFamily that only support Presentation.
            // I think the dedicated presentation queue is aimed for peformance, but I need more study in it.
            // and if dedicated presentation queue need to be used, we should create it in CreateDevice ahead.
            // so we follow the strategy in UE4 use compute/gfx queue for presentation.
            uint32_t PresentQueueFamilyIndex = mComputeQueueFamilyIndex;
            VkBool32 bSupportPresent = false;
            VkResult RetCheckSurfaceSupport = vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice, mComputeQueueFamilyIndex, Surface, &bSupportPresent);
            if (RetCheckSurfaceSupport != VkResult::VK_SUCCESS || !bSupportPresent)
            {
                PresentQueueFamilyIndex = mGraphicQueueFamilyIndex;
                RetCheckSurfaceSupport = vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice, mGraphicQueueFamilyIndex, Surface, &bSupportPresent);
            }

            if (bSupportPresent)
            {
                mVulkanPresentQueue = (PresentQueueFamilyIndex == mComputeQueueFamilyIndex) ? mVulkanComputeQueue : mVulkanGraphicQueue;

                bool bSupportBGRA8_SRGB = false;
                bool bSupportFIFOPresent = false;
                uint32_t NumSurfaceFormats = 0;
                uint32_t NumPresentModes = 0;
                std::vector<VkSurfaceFormatKHR> SurfaceFormats;
                std::vector<VkPresentModeKHR> PresentModes;
                vkGetPhysicalDeviceSurfaceFormatsKHR(mVulkanPhysicalDevice, Surface, &NumSurfaceFormats, nullptr);
                vkGetPhysicalDeviceSurfacePresentModesKHR(mVulkanPhysicalDevice, Surface, &NumPresentModes, nullptr);
                if (NumSurfaceFormats > 0)
                {
                    SurfaceFormats.resize(NumSurfaceFormats);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(mVulkanPhysicalDevice, Surface, &NumSurfaceFormats, SurfaceFormats.data());
                    for (VkSurfaceFormatKHR& Format : SurfaceFormats)
                    {
                        if (Format.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                            && Format.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM)
                        {
                            bSupportBGRA8_SRGB = true;
                            break;
                        }
                    }
                }
                if (NumPresentModes > 0)
                {
                    PresentModes.resize(NumPresentModes);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(mVulkanPhysicalDevice, Surface, &NumPresentModes, PresentModes.data());
                    for (VkPresentModeKHR& PresentMode : PresentModes)
                    {
                        if (PresentMode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR)
                        {
                            bSupportFIFOPresent = true;
                            break;
                        }
                    }
                }

                if (bSupportBGRA8_SRGB && bSupportFIFOPresent)
                {
                    VkSurfaceCapabilitiesKHR SurfaceCaps;
                    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVulkanPhysicalDevice, Surface, &SurfaceCaps);

                    auto clamp = [](int v, int min, int max) { return (v > max) ? max : (v < min ? min : v); };
                    windowWidth = clamp(windowWidth, SurfaceCaps.minImageExtent.width, SurfaceCaps.maxImageExtent.width);
                    windowHeight = clamp(windowHeight, SurfaceCaps.minImageExtent.height, SurfaceCaps.maxImageExtent.height);
                    uint32_t NumSwapchainImages = 3;
                    if (SurfaceCaps.maxImageCount != 0)
                    {
                        NumSwapchainImages = clamp(NumSwapchainImages, SurfaceCaps.minImageCount, SurfaceCaps.maxImageCount);
                    }

                    VkSurfaceTransformFlagBitsKHR SurfacePreTransform =
                        (SurfaceCaps.supportedTransforms & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
                        ? VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
                        : SurfaceCaps.currentTransform;

                    VkCompositeAlphaFlagBitsKHR SurfaceCompositeAlpha =
                        (SurfaceCaps.supportedCompositeAlpha & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
                        ? VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
                        : VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

                    VkSwapchainCreateInfoKHR SwapChainCreateInfo;
                    VulkanZeroMemory(SwapChainCreateInfo);
                    SwapChainCreateInfo.surface = Surface;
                    SwapChainCreateInfo.minImageCount = NumSwapchainImages;
                    SwapChainCreateInfo.imageFormat = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
                    SwapChainCreateInfo.imageColorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                    SwapChainCreateInfo.imageExtent.width = windowWidth;
                    SwapChainCreateInfo.imageExtent.height = windowHeight;
                    SwapChainCreateInfo.imageArrayLayers = 1;
                    //TODO:
                    // It is possible that you'll render images to a separate image first to perform post-processing.
                    // In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead
                    // and use a memory operation to transfer the rendered image to a swap chain image.
                    SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                    // An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family.
                    // This option offers the best performance.
                    SwapChainCreateInfo.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
                    SwapChainCreateInfo.queueFamilyIndexCount = 0;
                    SwapChainCreateInfo.pQueueFamilyIndices = nullptr;
                    SwapChainCreateInfo.preTransform = SurfacePreTransform;
                    SwapChainCreateInfo.compositeAlpha = SurfaceCompositeAlpha;
                    SwapChainCreateInfo.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
                    SwapChainCreateInfo.clipped = VK_TRUE;
                    //TODO:
                    // Set to last if re-creating swapchain.
                    SwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

                    VkSwapchainKHR VulkanSwapchain;
                    VkResult RetCreateSwapChain = vkCreateSwapchainKHR(mVulkanDevice, &SwapChainCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanSwapchain);

                    if (RetCreateSwapChain == VK_SUCCESS)
                    {
                        //TODO:
                        SwapChainVulkan* SwapChain = new SwapChainVulkan(this, VulkanSwapchain);
                        return SwapChain;
                        
                    }
                }
            }
            else
            {
                vkDestroySurfaceKHR(mBelongsTo->GetInstance(), Surface, GFX_VK_ALLOCATION_CALLBACK);
            }
        }

        return nullptr;
    }

    SamplerState* GraphicDeviceVulkan::CreateSamplerState(const SamplerState::CreateInfo&)
    {
        return nullptr;
    }
}
