#pragma once
#include "VulkanInclude.h"
#include "VulkanGraphicDevice.h"

namespace GFXI
{
    struct BaseDeviceResourceVulkan
    {
        BaseDeviceResourceVulkan(GraphicDeviceVulkan* parent) : mParent(parent) { }
        GraphicDeviceVulkan* GetParent() { return mParent; }
        VkDevice GetVulkanDevice() { return mParent->GetVulkanDevice(); };
    private:
        GraphicDeviceVulkan* mParent = nullptr;
    };
}
