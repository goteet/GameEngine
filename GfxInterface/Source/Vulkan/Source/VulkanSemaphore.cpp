#include "VulkanSemaphore.h"

namespace GFXI
{
    SemaphoreVulkan::SemaphoreVulkan(GraphicDeviceVulkan* belongsTo, VkSemaphore semaphore, bool bUseRelase)
        : BaseDeviceResourceVulkan(belongsTo)
        , mVulkanSemaphore(semaphore)
        , mUseRelease(bUseRelase)
    {

    }
    SemaphoreVulkan::~SemaphoreVulkan()
    {
        vkDestroySemaphore(GetVulkanDevice(), mVulkanSemaphore, GFX_VK_ALLOCATION_CALLBACK);
    }
    void SemaphoreVulkan::Release()
    {
        if (mUseRelease)
            delete this;
    }
}
