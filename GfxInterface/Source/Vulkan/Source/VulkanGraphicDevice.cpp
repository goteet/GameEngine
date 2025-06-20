#include "VulkanGraphicDevice.h"
#include "VulkanGraphicModule.h"


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
        VulkanZeroMemory(SurfaceCreateInfo, VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR);
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

                VkSurfaceCapabilitiesKHR SurfaceCaps;
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVulkanPhysicalDevice, Surface, &SurfaceCaps);

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
                            && Format.format == VkFormat::VK_FORMAT_R8G8B8A8_UNORM)
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
                    //TODO:
                    //VulkanSwapchain* SwapChain = new VulkanSwapchain();
                    //return SwapChain;
                }


            }
            else
            {
                vkDestroySurfaceKHR(mBelongsTo->GetInstance(), Surface, GFX_VK_ALLOCATION_CALLBACK);
            }
        }

        return nullptr;
    }
}
