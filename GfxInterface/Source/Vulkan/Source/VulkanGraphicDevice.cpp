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
                //TODO:
            }
            else
            {
                vkDestroySurfaceKHR(mBelongsTo->GetInstance(), Surface, GFX_VK_ALLOCATION_CALLBACK);
            }
        }

        return nullptr;
    }
}
