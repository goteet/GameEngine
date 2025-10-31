#include "VulkanCommandQueue.h"
#include "VulkanGraphicDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSemaphore.h"

namespace GFXI
{
    VulkanCommandQueue::VulkanCommandQueue(VkDevice device, uint32_t familyIndex, uint32_t index)
        : mFamilyIndex(familyIndex)
        , mQueueIndex(index)
    {
        vkGetDeviceQueue(device, familyIndex, index, &mVulkanQueue);
    }

    VulkanCommandQueue::VulkanCommandQueue(VkQueue queue,   uint32_t familyIndex, uint32_t index)
        : mVulkanQueue(queue)
        , mFamilyIndex(familyIndex)
        , mQueueIndex(index)
    {
    }

}
