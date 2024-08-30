#include "VulkanGraphicDevice.h"
#include "VulkanGraphicModule.h"


namespace GFXI
{
    GraphicDeviceVulkan::GraphicDeviceVulkan(GraphicModuleVulkan* belongsTo, VkDevice vulkanDevice)
        : mBelongsTo(belongsTo)
        , mVulkanDevice(vulkanDevice)
    {
        uint32_t QueueFamilyIndex = mBelongsTo->GetQueueFamilyIndex();
        uint32_t QueueIndex = 0;
        vkGetDeviceQueue(vulkanDevice, QueueFamilyIndex, QueueIndex, &mVulkanQueue);
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
        return nullptr;
    }
}
