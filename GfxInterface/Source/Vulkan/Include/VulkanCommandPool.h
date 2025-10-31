#pragma once
#include "GfxInterface.h"
#include "VulkanInclude.h"
#include "VulkanCommandBuffer.h"

namespace GFXI
{
    struct GraphicDeviceVulkan;

    template<typename BufferType, typename BufferImplType>
    struct CommandPoolVulkan : public TCommandPool<BufferType>, public BaseDeviceResourceVulkan
    {
        CommandPoolVulkan(GraphicDeviceVulkan* belongsTo, uint32_t queueFamily)
            : BaseDeviceResourceVulkan(belongsTo)
            , mQueueFamily(queueFamily)
        {
            mVulkanCommandPool = CreateCommandPool(GetVulkanDevice(), mQueueFamily);
        }

        virtual ~CommandPoolVulkan()
        {
            if (mVulkanCommandPool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(GetVulkanDevice(), mVulkanCommandPool, GFX_VK_ALLOCATION_CALLBACK);
                mVulkanCommandPool = VK_NULL_HANDLE;
            }
        }

        virtual void Release() override
        {
            delete this;
        }

        virtual BufferType* CreateCommandBuffer() override
        {
            VkCommandBufferAllocateInfo allocateInfo;
            VulkanZeroMemory(allocateInfo);
            allocateInfo.commandPool = mVulkanCommandPool;
            allocateInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            VkResult retAllocateBuffer = vkAllocateCommandBuffers(GetVulkanDevice(), &allocateInfo, &commandBuffer);
            if (retAllocateBuffer == VkResult::VK_SUCCESS)
            {
                return new BufferImplType(GetParent(), this, commandBuffer);
            }
            else
            {
                return nullptr;
            }
        }
    private:
        static VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueFamily)
        {
            VkCommandPoolCreateInfo commandPoolCreateInfo;
            VulkanZeroMemory(commandPoolCreateInfo);
            commandPoolCreateInfo.queueFamilyIndex = queueFamily;
            commandPoolCreateInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            VkCommandPool pool;
            VkResult retCreateCommandPool = vkCreateCommandPool(device, &commandPoolCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &pool);
            if (retCreateCommandPool == VkResult::VK_SUCCESS)
            {
                return pool;
            }
            else
            {
                return VK_NULL_HANDLE;
            }
        }

        uint32_t        mQueueFamily;
        VkCommandPool   mVulkanCommandPool = VK_NULL_HANDLE;
    };

    using TransferCommandPoolVulkan = CommandPoolVulkan<TransferCommandBuffer, TransferCommandBufferVulkan>;
    using ComputeCommandPoolVulkan  = CommandPoolVulkan<ComputeCommandBuffer,  ComputeCommandBufferVulkan>;
    using GraphicCommandPoolVulkan  = CommandPoolVulkan<GraphicCommandBuffer,  GraphicCommandBufferVulkan>;
    
}
