#include "VulkanCommandQueue.h"

namespace GFXI
{
    CommandQueueVulkan::CommandQueueVulkan(VkDevice device, uint32_t familyIndex, uint32_t index)
        : mFamilyIndex(familyIndex)
        , mQueueIndex(index)
    {
        vkGetDeviceQueue(device, familyIndex, index, &mVulkanQueue);
    }

    CommandQueueVulkan::CommandQueueVulkan(VkQueue queue,   uint32_t familyIndex, uint32_t index)
        : mVulkanQueue(queue)
        , mFamilyIndex(familyIndex)
        , mQueueIndex(index)
    {
    }
}
