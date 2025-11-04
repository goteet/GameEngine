#include "VulkanDeviceContext.h"

namespace GFXI
{
    DeferredContextVulkan::DeferredContextVulkan(GraphicDeviceVulkan* belongsTo, CommandQueueVulkan cmdQueue)
        : BaseDeviceResourceVulkan(belongsTo)
        , mCommandQueue(cmdQueue)
    {
        CreateCommandPool();
    }

    DeferredContextVulkan::~DeferredContextVulkan()
    {
        if (mVulkanCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(GetVulkanDevice(), mVulkanCommandPool, GFX_VK_ALLOCATION_CALLBACK);
            mVulkanCommandPool = VK_NULL_HANDLE;
        }
    }

    void DeferredContextVulkan::Release()
    {
        //delete this;
    }

    struct RenderTargetViewVulkan : public RenderTargetView
    {
        VkImageView GetVulkanImageView() { return VkImageView(); }
    };

    struct DepthStencilViewVulkan : public DepthStencilView
    {
        VkImageView GetVulkanImageView() { return VkImageView(); }
    };
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
    bool DeferredContextVulkan::BeginRecordCommands(const RenderingInfo& renderingInfo)
    {
        using RenderTargetDesc = GFXI::DeferredContext::RenderingInfo::RenderTargetDesc;
        using DepthStencilDesc = GFXI::DeferredContext::RenderingInfo::DepthStencilDesc;

        if (mVulkanCommandPool == VK_NULL_HANDLE || mInsideRenderPass)
        {
            return false;
        }

        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        for (uint32_t index = 0; index < renderingInfo.NumRenderTargets; index++)
        {
            const RenderTargetDesc& renderTargetDesc = renderingInfo.RenderTargets[index];
            VkRenderingAttachmentInfo colorAttachment;
            VulkanZeroMemory(colorAttachment);

            colorAttachment.imageView = reinterpret_cast<RenderTargetViewVulkan*>(renderTargetDesc.View)->GetVulkanImageView();
            //TODO:Fixed this with right parameter.
            colorAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            if (renderTargetDesc.EnableResolve)
            {
                colorAttachment.resolveMode = VkResolveModeFlagBits::VK_RESOLVE_MODE_AVERAGE_BIT;
                colorAttachment.resolveImageView = reinterpret_cast<RenderTargetViewVulkan*>(renderTargetDesc.ResolvedView)->GetVulkanImageView();
                //TODO:Fixed this with right parameter.
                colorAttachment.resolveImageLayout= VkImageLayout::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            }
            else
            {
                colorAttachment.resolveMode = VkResolveModeFlagBits::VK_RESOLVE_MODE_NONE;
            }
            colorAttachment.loadOp  = VulkanAttachmentLoadOpMapping[static_cast<uint8_t>(renderTargetDesc.LoadOp)];
            colorAttachment.storeOp = VulkanAttachmentStoreOpMapping[static_cast<uint8_t>(renderTargetDesc.StoreOp)];
            //TODO: Fixed this with different value types.
            colorAttachment.clearValue.color.float32[0] = renderTargetDesc.ClearValue[0];
            colorAttachment.clearValue.color.float32[1] = renderTargetDesc.ClearValue[1];
            colorAttachment.clearValue.color.float32[2] = renderTargetDesc.ClearValue[2];
            colorAttachment.clearValue.color.float32[3] = renderTargetDesc.ClearValue[3];

            colorAttachments.emplace_back(colorAttachment);
        }

        const DepthStencilDesc& dsDesc = renderingInfo.DepthStencil;
        VkRenderingAttachmentInfo depthStencilAttachment;
        VulkanZeroMemory(depthStencilAttachment);
        depthStencilAttachment.imageView = reinterpret_cast<DepthStencilViewVulkan*>(dsDesc.View)->GetVulkanImageView();
        depthStencilAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachment.resolveMode = VkResolveModeFlagBits::VK_RESOLVE_MODE_NONE;
        depthStencilAttachment.loadOp  = VulkanAttachmentLoadOpMapping[static_cast<uint8_t>(dsDesc.LoadOp)];
        depthStencilAttachment.storeOp = VulkanAttachmentStoreOpMapping[static_cast<uint8_t>(dsDesc.StoreOp)];
        depthStencilAttachment.clearValue.depthStencil.depth = dsDesc.DepthClearValue;
        depthStencilAttachment.clearValue.depthStencil.stencil = dsDesc.StencilClearValue;


        VkRenderingInfo vulkanRenderingInfo;
        VulkanZeroMemory(vulkanRenderingInfo);
        //vulkanRenderingInfo.flags = VkRenderingFlagBits::
        vulkanRenderingInfo.renderArea.offset.x = 0;
        vulkanRenderingInfo.renderArea.offset.y = 0;
        vulkanRenderingInfo.renderArea.extent.width     = renderingInfo.SwapchainWidth;
        vulkanRenderingInfo.renderArea.extent.height    = renderingInfo.SwapchainHeight;
        vulkanRenderingInfo.layerCount = 1;
        vulkanRenderingInfo.viewMask   = renderingInfo.ActiveViewMask;

        if (renderingInfo.NumRenderTargets > 0)
        {
            vulkanRenderingInfo.colorAttachmentCount    = static_cast<uint32_t>(colorAttachments.size());
            vulkanRenderingInfo.pColorAttachments       = colorAttachments.data();
        }
        else
        {
            vulkanRenderingInfo.colorAttachmentCount    = 0;
            vulkanRenderingInfo.pColorAttachments       = nullptr;
        }
        vulkanRenderingInfo.pDepthAttachment      = &depthStencilAttachment;
        vulkanRenderingInfo.pStencilAttachment    = &depthStencilAttachment;


        VkCommandBufferAllocateInfo commandBufferAllocationInfo;
        VulkanZeroMemory(commandBufferAllocationInfo);
        commandBufferAllocationInfo.commandPool = mVulkanCommandPool;
        commandBufferAllocationInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocationInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(GetVulkanDevice(), &commandBufferAllocationInfo, &mActiveCommandBuffer);
        vkCmdBeginRendering(mActiveCommandBuffer, &vulkanRenderingInfo);

        mInsideRenderPass = true;
        return true;
    }

    CommandQueue* DeferredContextVulkan::EndRecordCommands(bool bRestoreToDefaultState)
    {
        if (mInsideRenderPass)
        {
            vkCmdEndRendering(mActiveCommandBuffer);
            mInsideRenderPass = false;

            return nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    void DeferredContextVulkan::CreateCommandPool()
    {
        if (mVulkanCommandPool == VK_NULL_HANDLE)
        {
            VkCommandPoolCreateInfo commandPoolCreateInfo;
            VulkanZeroMemory(commandPoolCreateInfo);
            commandPoolCreateInfo.queueFamilyIndex = mCommandQueue.GetFamilyIndex();
            commandPoolCreateInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            VkResult retCreateCommandPool = vkCreateCommandPool(GetVulkanDevice(), &commandPoolCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &mVulkanCommandPool);
            if (retCreateCommandPool != VkResult::VK_SUCCESS)
            {
                //assert(0);
            }
        }
    }
}
