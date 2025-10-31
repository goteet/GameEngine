#pragma once
#include "VulkanInclude.h"
#include "GfxInterface.h"


namespace GFXI
{
    struct GraphicDeviceVulkan;

    struct SemaphoreVulkan : public Semaphore, public BaseDeviceResourceVulkan
    {
        SemaphoreVulkan(GraphicDeviceVulkan* belongsTo, VkSemaphore semaphore, bool bUseRelase);
        virtual ~SemaphoreVulkan();
        virtual void Release() override;

        VkSemaphore GetVulkanSemaphore() { return mVulkanSemaphore; }

    private:
        VkSemaphore mVulkanSemaphore;
        bool mUseRelease;
    };
}
