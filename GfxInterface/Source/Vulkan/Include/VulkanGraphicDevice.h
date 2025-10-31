#pragma once
#include "VulkanInclude.h"
#include "GfxInterface.h"
#include "VulkanDeviceContext.h"
#include "VulkanCommandQueue.h"



namespace GFXI
{
    struct GraphicModuleVulkan;

    struct GraphicDeviceVulkan : public GraphicDevice
    {
        GraphicDeviceVulkan(GraphicModuleVulkan* belongsTo, VkPhysicalDevice vulkanPhysicalDevice, VkDevice vulkanDevice,
            CommandQueueVulkan graphicQueue, CommandQueueVulkan computeQueue, CommandQueueVulkan transferQueue);
        virtual ~GraphicDeviceVulkan();
        virtual void Release() override;

        virtual SwapChain*              CreateSwapChain(void* windowHandle, int windowWidth, int windowHeight, bool isFullscreen) override;
        virtual GraphicPipelineState*   CreateGraphicPipelineState(const GraphicPipelineState::CreateInfo&) override;
        virtual ComputePipelineState*   CreateComputePipelineState(const ComputePipelineState::CreateInfo&) override { return nullptr; }
        virtual DescriptorSetLayout*    CreateDescriptorSetLayout(const DescriptorSetLayout::CreateInfo&) override;
        virtual SamplerState*           CreateSamplerState(const SamplerState::CreateInfo&) override;
        virtual ShaderBinary*           CompileShader(const ShaderBinary::CreateInfo&) override;
        virtual Shader*                 CreateShader(const Shader::CreateInfo&) override;
        virtual VertexBuffer*           CreateVertexBuffer(const VertexBuffer::CreateInfo&) override { return nullptr; }
        virtual IndexBuffer*            CreateIndexBuffer(const IndexBuffer::CreateInfo&) override { return nullptr; }
        virtual UniformBuffer*          CreateUniformbuffer(const UniformBuffer::CreateInfo&) override { return nullptr; }
        virtual RenderTargetView*       CreateRenderTargetView(const RenderTargetView::CreateInfo&) override { return nullptr; }
        virtual DepthStencilView*       CreateDepthStencilView(const DepthStencilView::CreateInfo&) override { return nullptr; }
        virtual ImmediateContext*       GetImmediateContext() override { return nullptr; }
        virtual DeferredContext*        GetDeferredContext()  override { return nullptr; }

        VkDevice GetVulkanDevice() { return mVulkanDevice; }
    private:
        GraphicModuleVulkan* mBelongsTo;
        VkPhysicalDevice mVulkanPhysicalDevice;
        VkDevice mVulkanDevice;

        CommandQueueVulkan mVulkanGraphicQueue;
        CommandQueueVulkan mVulkanComputeQueue;
        CommandQueueVulkan mVulkanTransferQueue;
        CommandQueueVulkan mVulkanPresentQueue;

        //DeviceContextVulkan mDeferredContext;
    };
}
