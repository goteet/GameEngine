#pragma once
#include <vulkan/vulkan.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#include <memory>

#define GFX_VK_ALLOCATION_CALLBACK nullptr

template <typename VulkanStruct> VkStructureType VulkanStructType();

#define MapVulkanStruct(S, E) template<> inline VkStructureType VulkanStructType<S>(){ return VkStructureType::E; }
MapVulkanStruct(VkApplicationInfo,              VK_STRUCTURE_TYPE_APPLICATION_INFO)
MapVulkanStruct(VkInstanceCreateInfo,           VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO)
MapVulkanStruct(VkWin32SurfaceCreateInfoKHR,    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR)
MapVulkanStruct(VkSwapchainCreateInfoKHR,       VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR)
MapVulkanStruct(VkDeviceQueueCreateInfo,        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO)
MapVulkanStruct(VkDeviceCreateInfo,             VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO)
MapVulkanStruct(VkImageViewCreateInfo,          VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO)
#undef MapVulkanStruct

template <typename VulkanStruct>
void VulkanZeroMemory(VulkanStruct& Struct, VkStructureType StructType = VulkanStructType<VulkanStruct>())
{
    memset(&Struct, 0, sizeof(VulkanStruct));
    Struct.sType = StructType;
}

