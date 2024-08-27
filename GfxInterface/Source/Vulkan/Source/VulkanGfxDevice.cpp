#include "VulkanGfxDevice.h"
#include "VulkanGfxModule.h"


namespace GFXI
{
    GfxDeviceVulkan::GfxDeviceVulkan(GfxModuleVulkan* belongsTo, VkDevice vulkanDevice)
        : mBelongsTo(belongsTo)
        , mVulkanDevice(vulkanDevice)
    {
        uint32_t QueueFamilyIndex = mBelongsTo->GetQueueFamilyIndex();
        uint32_t QueueIndex = 0;
        vkGetDeviceQueue(vulkanDevice, QueueFamilyIndex, QueueIndex, &mVulkanQueue);
    }

    GfxDeviceVulkan::~GfxDeviceVulkan()
    {
        vkDestroyDevice(mVulkanDevice, GFX_VK_ALLOCATION_CALLBACK);
    }
    void GfxDeviceVulkan::Release()
    {
        delete this;
    }

    SwapChain* GfxDeviceVulkan::CreateSwapChain(void* windowHandle, int windowWidth, int windowHeight, bool isFullscreen)
    {
        return nullptr;
    }
}
