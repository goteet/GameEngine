#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#define GFX_VK_ALLOCATION_CALLBACK nullptr

template <typename VulkanStruct>
void VulkanZeroMemory(VulkanStruct& Struct, VkStructureType StructType)
{
    memset(&Struct, 0, sizeof(VulkanStruct));
    Struct.sType = StructType;
}

