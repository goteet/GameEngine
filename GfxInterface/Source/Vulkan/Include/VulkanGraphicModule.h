#pragma once
#include <vector>
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    const int32_t kQueueFamilyNotFound = -1;

    struct GraphicModuleVulkan : public GraphicModule
    {
        GraphicModuleVulkan();
        virtual ~GraphicModuleVulkan();
        virtual void Release() override;

        virtual bool IsHardwareSupported() override;
        virtual GraphicDevice* CreateDevice() override;

        int32_t GetQueueFamilyIndex() const { return mQueueFamilyIndex; }

    private:
        VkInstance CreateVkInstance(bool bEnableDebugLayer);
        bool SelectBestPhysicalDevice(const VkInstance& VulkanInstance, VkPhysicalDevice& PhysicalDevice, int32_t& QueueFamilyIndex, VkPhysicalDeviceFeatures& OutDeviceFeatures);

        std::vector<const char*>    mDebugLayer;
        VkPhysicalDeviceFeatures    mDeviceFeatures;

        VkInstance mVulkanInstance = nullptr;
        VkPhysicalDevice mPhyiscalDevice = nullptr;
        int32_t mQueueFamilyIndex = kQueueFamilyNotFound;
    };
}
