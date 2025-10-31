#pragma once
#include "VulkanInclude.h"
#include "GfxInterface.h"
#include "VulkanCommandBuffer.h"

namespace GFXI
{
    struct VulkanCommandQueue
    {
        VulkanCommandQueue() = default;
        VulkanCommandQueue(VkDevice device, uint32_t familyIndex, uint32_t index);
        VulkanCommandQueue(VkQueue  queue,  uint32_t familyIndex, uint32_t index);
        VkQueue     GetVulkanQueue() const { return mVulkanQueue; }
        uint32_t    GetFamilyIndex() const { return mFamilyIndex; }
        uint32_t    GetQueueIndex()  const { return mQueueIndex;  }

    private:
        VkQueue   mVulkanQueue = VK_NULL_HANDLE;
        uint32_t  mFamilyIndex = 0;
        uint32_t  mQueueIndex  = 0;
    };

    template<typename BufferType, typename VulkanBufferType>
    struct TCommandQueueVulkan : public TCommandQueue<BufferType>, public BaseDeviceResourceVulkan
    {
        TCommandQueueVulkan(GraphicDeviceVulkan* belongsTo, VulkanCommandQueue queue)
            : BaseDeviceResourceVulkan(belongsTo), mQueue(queue) { }

        virtual void Release() override { }

        virtual void ExecuteCommandBuffer(BufferType* commandBuffer) override
        {
            VulkanBufferType* commadnBufferImpl = reinterpret_cast<VulkanBufferType*> (commandBuffer);
            const VulkanCommandBufferWrap& commandBufferWrap = commadnBufferImpl->GetCommandBufferWrapper();
            VkCommandBuffer vulkanCommandBuffer = commadnBufferImpl->GetVulkanCommandBuffer();

            VkSubmitInfo submitInfo;
            VulkanZeroMemory(submitInfo);
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &vulkanCommandBuffer;
            submitInfo.waitSemaphoreCount = static_cast<uint32_t>(commandBufferWrap.WaitSemaphores.size());
            submitInfo.pWaitSemaphores = commandBufferWrap.WaitSemaphores.data();
            submitInfo.pWaitDstStageMask = commandBufferWrap.WaitStages.data();
            submitInfo.signalSemaphoreCount = static_cast<uint32_t>(commandBufferWrap.SignalSemaphores.size());
            submitInfo.pSignalSemaphores = commandBufferWrap.SignalSemaphores.data();
            VkResult retSubmit = vkQueueSubmit(mQueue.GetVulkanQueue(), 1, &submitInfo, commandBufferWrap.FinishFence);
            if (retSubmit != VkResult::VK_SUCCESS)
            {
                DebugBreak();
            }
            PendingCommands.emplace_back(commadnBufferImpl);
        }

        const VulkanCommandQueue& GetQueue() { return mQueue; }

    private:
        VulkanCommandQueue mQueue;

    public:
        std::vector<VulkanBufferType*> PendingCommands;
    };

    struct TransferCommandBufferVulkan;
    struct ComputeCommandBufferVulkan;
    struct GraphicCommandBufferVulkan;
    using TransferCommandQueueVulkan = TCommandQueueVulkan<TransferCommandBuffer, TransferCommandBufferVulkan>;
    using ComputeCommandQueueVulkan  = TCommandQueueVulkan<ComputeCommandBuffer , ComputeCommandBufferVulkan >;
    using GraphicCommandQueueVulkan  = TCommandQueueVulkan<GraphicCommandBuffer , GraphicCommandBufferVulkan >;
}
