#pragma once
#include "VulkanInclude.h"

namespace GFXI
{
    struct CommandQueueVulkan
    {
        CommandQueueVulkan() = default;
        CommandQueueVulkan(VkDevice device, uint32_t familyIndex, uint32_t index);
        CommandQueueVulkan(VkQueue  queue,  uint32_t familyIndex, uint32_t index);
        VkQueue     GetVulkanQueue() const { return mVulkanQueue; }
        uint32_t    GetFamilyIndex() const { return mFamilyIndex; }
        uint32_t    GetQueueIndex()  const { return mQueueIndex;  }

    private:
        VkQueue   mVulkanQueue = VK_NULL_HANDLE;
        uint32_t  mFamilyIndex = 0;
        uint32_t  mQueueIndex  = 0;
    };
}
