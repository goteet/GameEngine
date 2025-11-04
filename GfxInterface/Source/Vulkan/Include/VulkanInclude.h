#pragma once
#include <vulkan/vulkan.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <shaderc/shaderc.hpp>

#include <memory>

#define GFX_VK_ALLOCATION_CALLBACK nullptr

template <typename VulkanStruct> VkStructureType VulkanStructType();

#define MapVulkanStruct(S, E) template<> inline VkStructureType VulkanStructType<S>(){ return VkStructureType::E; }
MapVulkanStruct(VkApplicationInfo,                          VK_STRUCTURE_TYPE_APPLICATION_INFO)
MapVulkanStruct(VkInstanceCreateInfo,                       VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO)
MapVulkanStruct(VkPhysicalDeviceDynamicRenderingFeatures,   VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES)
MapVulkanStruct(VkWin32SurfaceCreateInfoKHR,                VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR)
MapVulkanStruct(VkSwapchainCreateInfoKHR,                   VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR)
MapVulkanStruct(VkDeviceQueueCreateInfo,                    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO)
MapVulkanStruct(VkDeviceCreateInfo,                         VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO)
MapVulkanStruct(VkImageViewCreateInfo,                      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO)
MapVulkanStruct(VkShaderModuleCreateInfo,                   VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO)
MapVulkanStruct(VkGraphicsPipelineCreateInfo,               VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
MapVulkanStruct(VkPipelineRenderingCreateInfo,              VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO)
MapVulkanStruct(VkPipelineShaderStageCreateInfo,            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO)
MapVulkanStruct(VkPipelineVertexInputStateCreateInfo,       VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineInputAssemblyStateCreateInfo,     VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineTessellationStateCreateInfo,      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineViewportStateCreateInfo,          VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineRasterizationStateCreateInfo,     VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineMultisampleStateCreateInfo,       VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineDepthStencilStateCreateInfo,      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineColorBlendStateCreateInfo,        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineDynamicStateCreateInfo,           VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO)
MapVulkanStruct(VkPipelineLayoutCreateInfo,                 VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO)
MapVulkanStruct(VkRenderPassCreateInfo,                     VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO)
MapVulkanStruct(VkDescriptorSetLayoutCreateInfo,            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO)
MapVulkanStruct(VkFramebufferCreateInfo,                    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO)
MapVulkanStruct(VkSemaphoreCreateInfo,                      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO)
MapVulkanStruct(VkFenceCreateInfo,                          VK_STRUCTURE_TYPE_FENCE_CREATE_INFO)
MapVulkanStruct(VkPresentInfoKHR,                           VK_STRUCTURE_TYPE_PRESENT_INFO_KHR)
MapVulkanStruct(VkRenderingInfo,                            VK_STRUCTURE_TYPE_RENDERING_INFO)
MapVulkanStruct(VkRenderingAttachmentInfo,                  VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO)
MapVulkanStruct(VkCommandPoolCreateInfo,                    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO)
MapVulkanStruct(VkCommandBufferAllocateInfo,                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO)

#undef MapVulkanStruct

template <typename VulkanStruct>
void VulkanZeroMemory(VulkanStruct& Struct, VkStructureType StructType = VulkanStructType<VulkanStruct>())
{
    memset(&Struct, 0, sizeof(VulkanStruct));
    Struct.sType = StructType;
}

#include "VulkanDeviceResource.h"
