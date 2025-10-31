#pragma once
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanSemaphore.h"

namespace GFXI
{
    GraphicCommandBufferVulkan::GraphicCommandBufferVulkan(GraphicDeviceVulkan* belongsTo, GraphicCommandPoolVulkan* pool, VkCommandBuffer buffer)
        : BaseDeviceResourceVulkan(belongsTo)
        , mPool(pool)
        , mVulkanCommandBuffer(belongsTo, buffer)
    {
    }

    GraphicCommandBufferVulkan::~GraphicCommandBufferVulkan()
    {
        mVulkanCommandBuffer.ReleaseVulkanResources();
    }

    void GraphicCommandBufferVulkan::Release()
    {
        delete this;
    }

    ComputeCommandBufferVulkan::ComputeCommandBufferVulkan(GraphicDeviceVulkan* belongsTo, ComputeCommandPoolVulkan* pool, VkCommandBuffer buffer)
        : BaseDeviceResourceVulkan(belongsTo)
        , mPool(pool)
        , mVulkanCommandBuffer(belongsTo, buffer)
    {
    }

    ComputeCommandBufferVulkan::~ComputeCommandBufferVulkan()
    {
        mVulkanCommandBuffer.WaitUntilFinish();
        mVulkanCommandBuffer.ReleaseVulkanResources();
    }

    void ComputeCommandBufferVulkan::Release()
    {
        delete this;
    }

    TransferCommandBufferVulkan::TransferCommandBufferVulkan(GraphicDeviceVulkan* belongsTo, TransferCommandPoolVulkan* pool, VkCommandBuffer buffer)
        : BaseDeviceResourceVulkan(belongsTo)
        , mPool(pool)
        , mVulkanCommandBuffer(belongsTo, buffer)
    {
    }

    TransferCommandBufferVulkan::~TransferCommandBufferVulkan()
    {
        mVulkanCommandBuffer.WaitUntilFinish();
        mVulkanCommandBuffer.ReleaseVulkanResources();
    }

    void TransferCommandBufferVulkan::Release()
    {
        delete this;
    }

    VulkanCommandBufferWrap::VulkanCommandBufferWrap(GraphicDeviceVulkan* belongsTo, VkCommandBuffer buffer)
        : BaseDeviceResourceVulkan(belongsTo)
        , Buffer(buffer)
    {
        CreateFinishSync();
    }

    bool VulkanCommandBufferWrap::BeginRecord()
    {
        if (!IsRecording)
        {
            VkCommandBufferBeginInfo beginCommandInfo;
            VulkanZeroMemory(beginCommandInfo);
            beginCommandInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VkResult retBeginRecord = vkBeginCommandBuffer(Buffer, &beginCommandInfo);
            if (retBeginRecord == VkResult::VK_SUCCESS)
            {
                IsRecording = true;
                return true;
            }
        }
            return false;
    }

    void VulkanCommandBufferWrap::EndRecord()
    {
        if (IsRecording)
        {
            VkResult retEndRecord = vkEndCommandBuffer(Buffer);
            if (retEndRecord == VkResult::VK_SUCCESS)
            {
                SignalSemaphores.emplace_back(FinishSemaphore);
                IsRecording = false;
            }
        }
    }

    VkPipelineStageFlags VulkanPipelineStageWaitMapping(ECommandWaitStage stage)
    {
        auto CheckStageFlagBits = [value = static_cast<uint32_t>(stage)](uint32_t mask, uint32_t mapping) -> uint32_t
        { return (value & mask) > 0 ? mapping : 0; };

        uint32_t colorAttachmentOuputStage = CheckStageFlagBits(ECommandWaitStage::ColorAttachmentOutput, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        return colorAttachmentOuputStage;
    }

    void VulkanCommandBufferWrap::AddWaitSemaphore(Semaphore* semaphore, ECommandWaitStage stage)
    {
        SemaphoreVulkan* semaphoreImpl = reinterpret_cast<SemaphoreVulkan*>(semaphore);
        WaitSemaphores.emplace_back(semaphoreImpl->GetVulkanSemaphore());
        WaitStages.emplace_back(VulkanPipelineStageWaitMapping(stage));
    }

    void VulkanCommandBufferWrap::AddSignalSemaphore(Semaphore* semaphore)
    {
        SemaphoreVulkan* semaphoreImpl = reinterpret_cast<SemaphoreVulkan*>(semaphore);
        SignalSemaphores.emplace_back(semaphoreImpl->GetVulkanSemaphore());
    }



    VkAttachmentLoadOp VulkanAttachmentLoadOpMapping[] = {
        VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE_EXT,
        VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD,
        VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR
    };
    VkAttachmentStoreOp VulkanAttachmentStoreOpMapping[] = {
        VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE,
        VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE
    };

    struct DepthStencilViewVulkan : public DepthStencilView
    {
        VkImageView GetVulkanImageView() { return VkImageView(); }
    };

    void VulkanCommandBufferWrap::BeginRendering(const GFXI::RenderingInfo& renderingInfo)
    {
        if (IsInsideRenderPass)
        {
            return;
        }

        VkRenderingInfo vulkanRenderingInfo;
        VulkanZeroMemory(vulkanRenderingInfo);
        vulkanRenderingInfo.renderArea.offset.x = 0;
        vulkanRenderingInfo.renderArea.offset.y = 0;
        vulkanRenderingInfo.renderArea.extent.width = renderingInfo.SwapchainWidth;
        vulkanRenderingInfo.renderArea.extent.height = renderingInfo.SwapchainHeight;
        vulkanRenderingInfo.layerCount = 1;
        vulkanRenderingInfo.viewMask = renderingInfo.ActiveViewMask;
        //vulkanRenderingInfo.flags = VkRenderingFlagBits::

        using RenderTargetDesc = GFXI::RenderingInfo::RenderTargetDesc;
        using DepthStencilDesc = GFXI::RenderingInfo::DepthStencilDesc;

        //TODO:
        //vkCmdPipelineBarrier(Buffer)

        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        if (renderingInfo.NumRenderTargets > 0)
        {
            for (uint32_t index = 0; index < renderingInfo.NumRenderTargets; index++)
            {
                const RenderTargetDesc& renderTargetDesc = renderingInfo.RenderTargets[index];
                VkRenderingAttachmentInfo colorAttachment;
                VulkanZeroMemory(colorAttachment);

                if (renderTargetDesc.View != nullptr)
                {
                    colorAttachment.imageView = reinterpret_cast<RenderTargetViewVulkan*>(renderTargetDesc.View)->GetVulkanImageView();
                    //TODO:Fixed this with right parameter.
                    colorAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    colorAttachment.resolveImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    if (renderTargetDesc.EnableResolve && renderTargetDesc.ResolvedView != nullptr)
                    {
                        colorAttachment.resolveMode = VkResolveModeFlagBits::VK_RESOLVE_MODE_AVERAGE_BIT;
                        colorAttachment.resolveImageView = reinterpret_cast<RenderTargetViewVulkan*>(renderTargetDesc.ResolvedView)->GetVulkanImageView();
                        //TODO:Fixed this with right parameter.
                        colorAttachment.resolveImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
                    }
                    else
                    {
                        colorAttachment.resolveMode = VkResolveModeFlagBits::VK_RESOLVE_MODE_NONE;
                    }
                    colorAttachment.loadOp = VulkanAttachmentLoadOpMapping[static_cast<uint8_t>(renderTargetDesc.LoadOp)];
                    colorAttachment.storeOp = VulkanAttachmentStoreOpMapping[static_cast<uint8_t>(renderTargetDesc.StoreOp)];
                    //TODO: Fixed this with different value types.
                    colorAttachment.clearValue.color.float32[0] = renderTargetDesc.ClearValue[0];
                    colorAttachment.clearValue.color.float32[1] = renderTargetDesc.ClearValue[1];
                    colorAttachment.clearValue.color.float32[2] = renderTargetDesc.ClearValue[2];
                    colorAttachment.clearValue.color.float32[3] = renderTargetDesc.ClearValue[3];

                    //colorAttachment.clearValue.color.int32[0] = 128;
                    //colorAttachment.clearValue.color.int32[1] = 0;
                    //colorAttachment.clearValue.color.int32[2] = 0;
                    //colorAttachment.clearValue.color.int32[3] = 128;
                    //
                    //colorAttachment.clearValue.color.uint32[0] = 255;// renderTargetDesc.ClearValue[0];
                    //colorAttachment.clearValue.color.uint32[1] = 0;//renderTargetDesc.ClearValue[1];
                    //colorAttachment.clearValue.color.uint32[2] = 0;//renderTargetDesc.ClearValue[2];
                    //colorAttachment.clearValue.color.uint32[3] = 255;//renderTargetDesc.ClearValue[3];
                }
                else
                {
                    colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE_EXT;
                    colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE;
                    colorAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
                }
                colorAttachments.emplace_back(colorAttachment);
            }
            vulkanRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
            vulkanRenderingInfo.pColorAttachments = colorAttachments.data();
        }

        const DepthStencilDesc& dsDesc = renderingInfo.DepthStencil;
        VkRenderingAttachmentInfo depthStencilAttachment;
        VulkanZeroMemory(depthStencilAttachment);
        if (dsDesc.View != nullptr)
        {
            depthStencilAttachment.imageView = reinterpret_cast<DepthStencilViewVulkan*>(dsDesc.View)->GetVulkanImageView();
            depthStencilAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthStencilAttachment.resolveMode = VkResolveModeFlagBits::VK_RESOLVE_MODE_NONE;
            depthStencilAttachment.loadOp = VulkanAttachmentLoadOpMapping[static_cast<uint8_t>(dsDesc.LoadOp)];
            depthStencilAttachment.storeOp = VulkanAttachmentStoreOpMapping[static_cast<uint8_t>(dsDesc.StoreOp)];
            depthStencilAttachment.clearValue.depthStencil.depth = dsDesc.DepthClearValue;
            depthStencilAttachment.clearValue.depthStencil.stencil = dsDesc.StencilClearValue;

            vulkanRenderingInfo.pDepthAttachment = &depthStencilAttachment;
            vulkanRenderingInfo.pStencilAttachment = &depthStencilAttachment;
        }

        vkCmdBeginRendering(Buffer, &vulkanRenderingInfo);
        IsInsideRenderPass = true;


        //VkViewport viewport{};
        //viewport.height   = (float)renderingInfo.SwapchainHeight;
        //viewport.width    = (float)renderingInfo.SwapchainWidth;
        //viewport.minDepth = (float)0.0f;
        //viewport.maxDepth = (float)1.0f;
        //vkCmdSetViewport(Buffer, 0, 1, &viewport);
        
    }

    void VulkanCommandBufferWrap::EndRendering()
    {
        if (IsInsideRenderPass)
        {
            vkCmdEndRendering(Buffer);
            IsInsideRenderPass = false;
        }
    }

    void VulkanCommandBufferWrap::ClearSemaphores()
    {
        WaitSemaphores.clear();
        SignalSemaphores.clear();
        WaitStages.clear();
        vkResetCommandBuffer(Buffer, 0);
    }

    void VulkanCommandBufferWrap::WaitUntilFinish()
    {
        vkWaitForFences(GetVulkanDevice(), 1, &FinishFence, VK_TRUE, UINT64_MAX);
        vkResetFences(GetVulkanDevice(), 1, &FinishFence);
    }

    void VulkanCommandBufferWrap::ReleaseVulkanResources()
    {
        vkDestroyFence(GetVulkanDevice(), FinishFence, GFX_VK_ALLOCATION_CALLBACK);
        vkDestroySemaphore(GetVulkanDevice(), FinishSemaphore, GFX_VK_ALLOCATION_CALLBACK);
    }
    void VulkanCommandBufferWrap::CreateFinishSync()
    {
        VkFenceCreateInfo fenceCreateInfo;
        VulkanZeroMemory(fenceCreateInfo);
        vkCreateFence(GetVulkanDevice(), &fenceCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &FinishFence);

        VkSemaphoreCreateInfo semaphoreCreateInfo;
        VulkanZeroMemory(semaphoreCreateInfo);
        vkCreateSemaphore(GetVulkanDevice(), &semaphoreCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &FinishSemaphore);
    }
}
