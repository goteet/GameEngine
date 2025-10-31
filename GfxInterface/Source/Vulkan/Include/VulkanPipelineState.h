#pragma once
#include "VulkanInclude.h"

namespace GFXI
{
    struct GraphicPipelineStateVulkan : public GraphicPipelineState, public BaseDeviceResourceVulkan
    {
        GraphicPipelineStateVulkan(GraphicDeviceVulkan* belongsTo, VkPipeline pipline);
        virtual ~GraphicPipelineStateVulkan();
        virtual void Release() override;
    private:
        VkPipeline mVulkanPipeline;
    };

    struct DescriptorSetLayoutVulkan : public DescriptorSetLayout, public BaseDeviceResourceVulkan
    {
        DescriptorSetLayoutVulkan(GraphicDeviceVulkan* belongsTo, VkDescriptorSetLayout layout);
        virtual ~DescriptorSetLayoutVulkan();
        virtual void Release() override;

        VkDescriptorSetLayout GetVulkanDescriptorSetLayout() { return mVulkanDescriptorSetLayout; }
    private:
        VkDescriptorSetLayout mVulkanDescriptorSetLayout;
    };
}
