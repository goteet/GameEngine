
#include "VulkanPipelineState.h"
#include "VulkanGraphicDevice.h"



namespace GFXI
{
    GraphicPipelineStateVulkan::GraphicPipelineStateVulkan(GraphicDeviceVulkan* belongsTo, VkPipeline pipline)
        : BaseDeviceResourceVulkan(belongsTo)
        , mVulkanPipeline(pipline)
    {
;
    }

    GraphicPipelineStateVulkan::~GraphicPipelineStateVulkan()
    {
        vkDestroyPipeline(GetVulkanDevice(), mVulkanPipeline, GFX_VK_ALLOCATION_CALLBACK);
    }

    void GraphicPipelineStateVulkan::Release()
    {
        delete this;
    }

    DescriptorSetLayoutVulkan::DescriptorSetLayoutVulkan(GraphicDeviceVulkan* belongsTo, VkDescriptorSetLayout layout)
        : BaseDeviceResourceVulkan(belongsTo)
        , mVulkanDescriptorSetLayout(layout)
    {
    }

    DescriptorSetLayoutVulkan::~DescriptorSetLayoutVulkan()
    {
        vkDestroyDescriptorSetLayout(GetVulkanDevice(), mVulkanDescriptorSetLayout, GFX_VK_ALLOCATION_CALLBACK);
    }

    void DescriptorSetLayoutVulkan::Release()
    {
        delete this;
    }
}
