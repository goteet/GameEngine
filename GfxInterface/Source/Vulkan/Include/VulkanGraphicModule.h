#pragma once
#include <vector>
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    struct GraphicModuleVulkan : public GraphicModule
    {
        GraphicModuleVulkan();
        virtual ~GraphicModuleVulkan();
        virtual void Release() override;

        virtual bool IsHardwareSupported() override;
        virtual GraphicDevice* CreateDevice() override;

        VkInstance GetInstance() const { return mVulkanInstance; }

    private:
        VkInstance CreateVkInstance(bool bEnableDebugLayer);
        VkInstance mVulkanInstance = nullptr;
        bool mEnableDebugLayer = false;
    };
}
