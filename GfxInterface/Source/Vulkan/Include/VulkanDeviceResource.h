#pragma once

namespace GFXI
{
    struct GraphicDeviceVulkan;
    struct BaseDeviceResourceVulkan
    {
        BaseDeviceResourceVulkan(GraphicDeviceVulkan* parent);
        GraphicDeviceVulkan* GetParent();
        VkDevice GetVulkanDevice();;
    private:
        GraphicDeviceVulkan* mParent = nullptr;
    };
}
