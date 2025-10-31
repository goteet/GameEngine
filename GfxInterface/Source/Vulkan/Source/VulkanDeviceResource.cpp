#pragma once
#include "VulkanInclude.h"
#include "VulkanGraphicDevice.h"

namespace GFXI
{
    BaseDeviceResourceVulkan::BaseDeviceResourceVulkan(GraphicDeviceVulkan* parent)
        : mParent(parent)
    {
    }

    GraphicDeviceVulkan* BaseDeviceResourceVulkan::GetParent()
    {
        return mParent;
    }
    VkDevice BaseDeviceResourceVulkan::GetVulkanDevice()
    {
        return mParent->GetVulkanDevice();
    }
}
