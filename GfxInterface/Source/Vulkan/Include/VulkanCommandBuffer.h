#pragma once
#include "GfxInterface.h"
#include "VulkanInclude.h"

namespace GFXI
{
    struct GraphicDeviceVulkan;
    struct SemaphoreVulkan;
    struct TransferCommandBufferVulkan;
    struct ComputeCommandBufferVulkan;
    struct GraphicCommandBufferVulkan;

    template<typename BufferType, typename BufferImplType> struct CommandPoolVulkan;
    using TransferCommandPoolVulkan = CommandPoolVulkan<TransferCommandBuffer, TransferCommandBufferVulkan>;
    using ComputeCommandPoolVulkan  = CommandPoolVulkan<ComputeCommandBuffer,  ComputeCommandBufferVulkan>;
    using GraphicCommandPoolVulkan  = CommandPoolVulkan<GraphicCommandBuffer,  GraphicCommandBufferVulkan>;

    struct VulkanCommandBufferWrap : public BaseDeviceResourceVulkan
    {
        VulkanCommandBufferWrap(GraphicDeviceVulkan* belongsTo, VkCommandBuffer buffer);

        // Common Commands
        bool BeginRecord();
        void EndRecord();
        void AddWaitSemaphore(Semaphore*, ECommandWaitStage);
        void AddSignalSemaphore(Semaphore*);

        //Graphic Commands
        void BeginRendering(const RenderingInfo & renderingInfo);
        void EndRendering();

        void ClearSemaphores();
        
        void WaitUntilFinish();
        void ReleaseVulkanResources();

        VkCommandBuffer Buffer;
        VkSemaphore     FinishSemaphore = VK_NULL_HANDLE;
        VkFence         FinishFence     = VK_NULL_HANDLE;
        bool IsRecording = false;
        bool IsInsideRenderPass = false;

        std::vector<VkSemaphore>    WaitSemaphores;
        std::vector<VkSemaphore>    SignalSemaphores;
        std::vector<VkPipelineStageFlags> WaitStages;
    private:
        void CreateFinishSync();
    };

    struct TransferCommandBufferVulkan : public TransferCommandBuffer, public BaseDeviceResourceVulkan
    {
        TransferCommandBufferVulkan(GraphicDeviceVulkan* belongsTo, TransferCommandPoolVulkan* pool, VkCommandBuffer buffer);

        virtual ~TransferCommandBufferVulkan();

        virtual void Release() override;

        virtual bool BeginRecord()  override { return mVulkanCommandBuffer.BeginRecord(); }
        virtual void EndRecord()    override { return mVulkanCommandBuffer.EndRecord(); }
        virtual bool IsRecording()  override { return mVulkanCommandBuffer.IsRecording; }
        virtual void AddWaitSemaphore(Semaphore* semaphore, ECommandWaitStage stage) override { mVulkanCommandBuffer.AddWaitSemaphore(semaphore, stage); }
        virtual void AddSignalSemaphore(Semaphore* semaphore) override { mVulkanCommandBuffer.AddSignalSemaphore(semaphore); }

        VkCommandBuffer GetVulkanCommandBuffer() { return mVulkanCommandBuffer.Buffer; }
        const VulkanCommandBufferWrap& GetCommandBufferWrapper() { return mVulkanCommandBuffer; }
        void ClearSemaphores() { mVulkanCommandBuffer.ClearSemaphores(); }
        void WaitUntilFinish() { mVulkanCommandBuffer.WaitUntilFinish(); }
    private:
        TransferCommandPoolVulkan* mPool;

        VulkanCommandBufferWrap mVulkanCommandBuffer;
    };

    struct ComputeCommandBufferVulkan : public ComputeCommandBuffer, public BaseDeviceResourceVulkan
    {
        ComputeCommandBufferVulkan(GraphicDeviceVulkan* belongsTo, ComputeCommandPoolVulkan* pool, VkCommandBuffer buffer);

        virtual ~ComputeCommandBufferVulkan();

        virtual void Release() override;

        virtual bool BeginRecord() override { return mVulkanCommandBuffer.BeginRecord(); }
        virtual void EndRecord() override { return mVulkanCommandBuffer.EndRecord(); }
        virtual bool IsRecording() override { return mVulkanCommandBuffer.IsRecording; }
        virtual void AddWaitSemaphore(Semaphore* semaphore, ECommandWaitStage stage) override { mVulkanCommandBuffer.AddWaitSemaphore(semaphore, stage); }
        virtual void AddSignalSemaphore(Semaphore* semaphore) override { mVulkanCommandBuffer.AddSignalSemaphore(semaphore); }

        VkCommandBuffer GetVulkanCommandBuffer() { return mVulkanCommandBuffer.Buffer; }
        const VulkanCommandBufferWrap& GetCommandBufferWrapper() { return mVulkanCommandBuffer; }
        void ClearSemaphores() { mVulkanCommandBuffer.ClearSemaphores(); }
        void WaitUntilFinish() { mVulkanCommandBuffer.WaitUntilFinish(); }

    private:
        ComputeCommandPoolVulkan* mPool;
        VulkanCommandBufferWrap mVulkanCommandBuffer;
    };

    struct GraphicCommandBufferVulkan : public GraphicCommandBuffer, public BaseDeviceResourceVulkan
    {
        GraphicCommandBufferVulkan(GraphicDeviceVulkan* belongsTo, GraphicCommandPoolVulkan* pool, VkCommandBuffer buffer);

        virtual ~GraphicCommandBufferVulkan();

        virtual void Release() override;

        virtual bool BeginRecord() override { return mVulkanCommandBuffer.BeginRecord(); }
        virtual void EndRecord() override { return mVulkanCommandBuffer.EndRecord(); }
        virtual bool IsRecording() override { return mVulkanCommandBuffer.IsRecording; }
        virtual void AddWaitSemaphore(Semaphore* semaphore, ECommandWaitStage stage) override { mVulkanCommandBuffer.AddWaitSemaphore(semaphore, stage); }
        virtual void AddSignalSemaphore(Semaphore* semaphore) override { mVulkanCommandBuffer.AddSignalSemaphore(semaphore); }

        virtual void BeginRendering(const RenderingInfo& renderingInfo) override { mVulkanCommandBuffer.BeginRendering(renderingInfo); }
        virtual void EndRendering() override { return mVulkanCommandBuffer.EndRendering(); }
        virtual bool IsInsideRenderPass() override { return mVulkanCommandBuffer.IsInsideRenderPass; }

        VkCommandBuffer GetVulkanCommandBuffer() { return mVulkanCommandBuffer.Buffer; }
        const VulkanCommandBufferWrap& GetCommandBufferWrapper() { return mVulkanCommandBuffer; }
        void ClearSemaphores() { mVulkanCommandBuffer.ClearSemaphores(); }
        void WaitUntilFinish() { mVulkanCommandBuffer.WaitUntilFinish(); }
    private:
        GraphicCommandPoolVulkan* mPool;
        VulkanCommandBufferWrap mVulkanCommandBuffer;
    };

}
