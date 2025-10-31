#include "VulkanGraphicDevice.h"
#include "VulkanGraphicModule.h"
#include "VulkanPipelineState.h"
#include "VulkanShader.h"
#include "VulkanSwapChain.h"



namespace GFXI
{
    GraphicDeviceVulkan::GraphicDeviceVulkan(GraphicModuleVulkan* belongsTo,
        VkPhysicalDevice physicalDevice,    VkDevice device,
        CommandQueueVulkan graphicQueue,
        CommandQueueVulkan computeQueue,
        CommandQueueVulkan transferQueue
    )
        : mBelongsTo(belongsTo)
        , mVulkanPhysicalDevice(physicalDevice)
        , mVulkanDevice(device)
        , mVulkanGraphicQueue(graphicQueue)
        , mVulkanComputeQueue(computeQueue)
        , mVulkanTransferQueue(transferQueue)
    {
    }

    GraphicDeviceVulkan::~GraphicDeviceVulkan()
    {
        vkDeviceWaitIdle(mVulkanDevice);
        vkDestroyDevice(mVulkanDevice, GFX_VK_ALLOCATION_CALLBACK);
    }
    void GraphicDeviceVulkan::Release()
    {
        delete this;
    }

    SwapChain* GraphicDeviceVulkan::CreateSwapChain(void* windowHandle, int windowWidth, int windowHeight, bool isFullscreen)
    {
        //TODO: support High-DPI.
        uint32_t pixelWidth = windowWidth;
        uint32_t pixelHeight = windowHeight;

        //TODO:
        // Support Multiple Platforms.
        //
        // We create a new surface to check
        // if device & related queue support presenting surfaces.
        VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo;
        VulkanZeroMemory(SurfaceCreateInfo);
        SurfaceCreateInfo.hwnd = static_cast<HWND>(windowHandle);
        SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

        VkSurfaceKHR Surface = nullptr;
        VkResult RetCreateSurface = vkCreateWin32SurfaceKHR(mBelongsTo->GetInstance(), &SurfaceCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &Surface);

        if (RetCreateSurface == VkResult::VK_SUCCESS)
        {

            //TODO: There will be some DeviceQueueFamily that only support Presentation.
            // I think the dedicated presentation queue is aimed for peformance, but I need more study in it.
            // and if dedicated presentation queue need to be used, we should create it in CreateDevice ahead.
            // so we follow the strategy in UE4 use compute/gfx queue for presentation.
            //TODO:
            // vkGetPhysicalDeviceWin32PresentationSupportKHR
            // This platform-specific function can be called prior to creating a surface.
            bool bUseComputeQueueToPresent = true;
            VkBool32 bSupportPresent = false;
            VkResult RetCheckSurfaceSupport = vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice, mVulkanComputeQueue.GetFamilyIndex(), Surface, &bSupportPresent);
            if (RetCheckSurfaceSupport != VkResult::VK_SUCCESS || !bSupportPresent)
            {
                bUseComputeQueueToPresent = false;
                RetCheckSurfaceSupport = vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice, mVulkanGraphicQueue.GetFamilyIndex(), Surface, &bSupportPresent);
            }

            if (bSupportPresent)
            {
                mVulkanPresentQueue = bUseComputeQueueToPresent
                    ? mVulkanComputeQueue
                    : mVulkanGraphicQueue;

                bool bSupportBGRA8_SRGB = false;
                bool bSupportPresentFIFORelaxed = false;
                bool bSupportPresentMailbox = false;
                uint32_t NumSurfaceFormats = 0;
                uint32_t NumPresentModes = 0;
                std::vector<VkSurfaceFormatKHR> SurfaceFormats;
                std::vector<VkPresentModeKHR> PresentModes;
                vkGetPhysicalDeviceSurfaceFormatsKHR(mVulkanPhysicalDevice, Surface, &NumSurfaceFormats, nullptr);
                vkGetPhysicalDeviceSurfacePresentModesKHR(mVulkanPhysicalDevice, Surface, &NumPresentModes, nullptr);
                if (NumSurfaceFormats > 0)
                {
                    SurfaceFormats.resize(NumSurfaceFormats);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(mVulkanPhysicalDevice, Surface, &NumSurfaceFormats, SurfaceFormats.data());
                    for (VkSurfaceFormatKHR& Format : SurfaceFormats)
                    {
                        if (Format.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                            && Format.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM)
                        {
                            bSupportBGRA8_SRGB = true;
                            break;
                        }
                    }
                }
                if (NumPresentModes > 0)
                {
                    PresentModes.resize(NumPresentModes);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(mVulkanPhysicalDevice, Surface, &NumPresentModes, PresentModes.data());
                    for (VkPresentModeKHR& PresentMode : PresentModes)
                    {
                        if (PresentMode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                        {
                            bSupportPresentFIFORelaxed = true;
                            if (bSupportPresentMailbox)
                            {
                                break;
                            }
                        }
                        else if (PresentMode == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
                        {
                            bSupportPresentMailbox = true;

                            if (bSupportPresentFIFORelaxed)
                            {
                                break;
                            }
                        }
                    }
                }

                if (bSupportBGRA8_SRGB && (bSupportPresentFIFORelaxed || bSupportPresentMailbox))
                {
                    VkSurfaceCapabilitiesKHR SurfaceCaps;
                    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVulkanPhysicalDevice, Surface, &SurfaceCaps);

                    auto clamp = [](int v, int min, int max) { return (v > max) ? max : (v < min ? min : v); };
                    windowWidth = clamp(windowWidth, SurfaceCaps.minImageExtent.width, SurfaceCaps.maxImageExtent.width);
                    windowHeight = clamp(windowHeight, SurfaceCaps.minImageExtent.height, SurfaceCaps.maxImageExtent.height);

                    //TODO: check and determin swapchain image count.
                    uint32_t NumSwapchainImages = bSupportPresentMailbox ? 3 : 2;
                    if (SurfaceCaps.maxImageCount != 0)
                    {
                        NumSwapchainImages = clamp(NumSwapchainImages, SurfaceCaps.minImageCount, SurfaceCaps.maxImageCount);
                    }

                    VkSurfaceTransformFlagBitsKHR SurfacePreTransform =
                        (SurfaceCaps.supportedTransforms & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
                        ? VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
                        : SurfaceCaps.currentTransform;

                    VkCompositeAlphaFlagBitsKHR SurfaceCompositeAlpha =
                        (SurfaceCaps.supportedCompositeAlpha & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
                        ? VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
                        : VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

                    VkSwapchainCreateInfoKHR SwapChainCreateInfo;
                    VulkanZeroMemory(SwapChainCreateInfo);
                    SwapChainCreateInfo.surface = Surface;
                    SwapChainCreateInfo.minImageCount = NumSwapchainImages;
                    SwapChainCreateInfo.imageFormat = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
                    SwapChainCreateInfo.imageColorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                    SwapChainCreateInfo.imageExtent.width = pixelWidth;
                    SwapChainCreateInfo.imageExtent.height = pixelHeight;
                    SwapChainCreateInfo.imageArrayLayers = 1;
                    //TODO:
                    // It is possible that you'll render images to a separate image first to perform post-processing.
                    // In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead
                    // and use a memory operation to transfer the rendered image to a swap chain image.
                    SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                    // An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family.
                    // This option offers the best performance.
                    SwapChainCreateInfo.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
                    SwapChainCreateInfo.queueFamilyIndexCount = 0;
                    SwapChainCreateInfo.pQueueFamilyIndices = nullptr;
                    SwapChainCreateInfo.preTransform = SurfacePreTransform;
                    SwapChainCreateInfo.compositeAlpha = SurfaceCompositeAlpha;
                    SwapChainCreateInfo.presentMode = bSupportPresentMailbox
                        ? VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR
                        : VkPresentModeKHR::VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                    SwapChainCreateInfo.clipped = VK_TRUE;
                    //TODO:
                    // Set to last if re-creating swapchain.
                    SwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

                    VkSwapchainKHR VulkanSwapchain;
                    VkResult RetCreateSwapChain = vkCreateSwapchainKHR(mVulkanDevice, &SwapChainCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanSwapchain);

                    if (RetCreateSwapChain == VK_SUCCESS)
                    {
                        //TODO: create image before creating swapchain.
                        SwapChainVulkan* SwapChain = new SwapChainVulkan(this, VulkanSwapchain, mVulkanPresentQueue);
                        return SwapChain;
                    }
                }
            }
            else
            {
                vkDestroySurfaceKHR(mBelongsTo->GetInstance(), Surface, GFX_VK_ALLOCATION_CALLBACK);
            }
        }

        return nullptr;
    }

    //TODO: Create PipelineStateObject.
    static const VkShaderStageFlagBits VulkanPipelineShaderStageMapping[] = {
        VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
        VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
        VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT,
        VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT
    };
    static const VkFormat VulkanRenderTargetFormatMapping[] = {
        VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32,   // R10G10B10A2_UNormInt
        VkFormat::VK_FORMAT_A8B8G8R8_UNORM_PACK32,      // R8G8B8A8_UNormInt
    };

    static const VkFormat VulkanDepthFormatMapping[] = {
        VkFormat::VK_FORMAT_D24_UNORM_S8_UINT,          //D24_UNormInt_S8_UInt
        VkFormat::VK_FORMAT_D32_SFLOAT,                 //D32_SFloat        
    };

    static const VkFormat VulkanStencilFormatMapping[] = {
        VkFormat::VK_FORMAT_D24_UNORM_S8_UINT,          //D24_UNormInt_S8_UInt
        VkFormat::VK_FORMAT_UNDEFINED,                  //D32_SFloat        
    };

    static const VkFormat VulkanVertexFormatMapping[] = {
        VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,    //RGBA32_SFloat
        VkFormat::VK_FORMAT_R8G8B8A8_UINT,          //RGBA32_UInt
        VkFormat::VK_FORMAT_R8G8B8A8_SINT,          //RGBA32_SInt
        VkFormat::VK_FORMAT_R32G32B32_SFLOAT,       //RGB32_SFloat
        VkFormat::VK_FORMAT_R32G32B32_UINT,         //RGB32_UInt
        VkFormat::VK_FORMAT_R32G32B32_SINT,         //RGB32_SInt
        VkFormat::VK_FORMAT_R32G32_SFLOAT,          //RG32_SFloat
        VkFormat::VK_FORMAT_R32G32_UINT,            //RG32_UInt
        VkFormat::VK_FORMAT_R32G32_SINT             //RG32_SInt
    };
    static const VkVertexInputRate VulkanVertexInputRateMapping[] = {
        VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX,
        VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE
    };
    static const VkPrimitiveTopology VulkanPrimitiveTopologyMapping[] = {
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST,                   //PointList = 0,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,                    //LineList = 1,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                //TriangleList = 2,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,               //TriangleStrip = 3,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,     //AdjacentLineListList = 4,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, //AdjacentTriangleList = 5,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY //AdjacentTriangleStrip = 6
    };
    static const VkBlendFactor VulkanBlendFactorMapping[] = {
        VK_BLEND_FACTOR_ZERO,                   //Zero = 0,
        VK_BLEND_FACTOR_ONE,                    //One = 1,
        VK_BLEND_FACTOR_SRC_COLOR,              //SrcColor = 2,
        VK_BLEND_FACTOR_DST_COLOR,              //DstColor = 3,
        VK_BLEND_FACTOR_CONSTANT_COLOR,         //Constant = 4,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,    //OneMinusSrcColor = 5,
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,    //OneMinusDstColor = 6,
        VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,//OneMinusConstant = 7,
        VK_BLEND_FACTOR_SRC_ALPHA,              //SrcAlpha = 8,
        VK_BLEND_FACTOR_DST_ALPHA,              //DstAlpha = 9,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,    //OneMinusSrcAlpha = 10,
        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,    //OneMinusDstAlpha = 11,

        //VK_BLEND_FACTOR_CONSTANT_ALPHA = 12,
        //VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13,
        //VK_BLEND_FACTOR_SRC_ALPHA_SATURATE = 14,
        //VK_BLEND_FACTOR_SRC1_COLOR = 15,
        //VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR = 16,
        //VK_BLEND_FACTOR_SRC1_ALPHA = 17,
        //VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA = 18,
    };
    static const VkBlendOp VulkanBlendOpMapping[] = {
        VK_BLEND_OP_ADD,                //Add = 0,
        VK_BLEND_OP_SUBTRACT,           //Sub = 1,
        VK_BLEND_OP_REVERSE_SUBTRACT,   //InvSub = 2,
        VK_BLEND_OP_MIN,                //Min = 3,
        VK_BLEND_OP_MAX,                //Max = 4
    };
    static const VkPolygonMode VulkanPolygonModeMapping[] = {
        VkPolygonMode::VK_POLYGON_MODE_FILL,    //Solid = 0,
        VkPolygonMode::VK_POLYGON_MODE_LINE     //Wireframe = 1
    };
    static const VkCullModeFlagBits VulkanCullModeMapping[] = {
        VkCullModeFlagBits::VK_CULL_MODE_NONE,  //None = 0,
        VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT, //Frontface = 1,
        VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT //Backface = 2
    };
    static const VkFrontFace VulkanFrontFaceMapping[] = {
        VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE,  // CounterClockwise = 0,
        VkFrontFace::VK_FRONT_FACE_CLOCKWISE           //Clockwise = 1,
    };


    static const VkCompareOp VulkanCompareOpMapping[] = {
        VkCompareOp::VK_COMPARE_OP_NEVER,           //Never = 0,
        VkCompareOp::VK_COMPARE_OP_ALWAYS,          //Always = 1,
        VkCompareOp::VK_COMPARE_OP_EQUAL,           //Equal = 2,
        VkCompareOp::VK_COMPARE_OP_NOT_EQUAL,       //NotEqual = 3,
        VkCompareOp::VK_COMPARE_OP_LESS,            //Less = 4,
        VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL,   //LessEqual = 5,
        VkCompareOp::VK_COMPARE_OP_GREATER,         //Greater = 6,
        VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL //GreaterEqual = 7,
    };

    static const VkStencilOp VulkanStencilOpMapping[] = {
        VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        VkStencilOp::VK_STENCIL_OP_ZERO,                //Zero = 0,
        VkStencilOp::VK_STENCIL_OP_KEEP,                //Keep = 1,
        VkStencilOp::VK_STENCIL_OP_REPLACE,             //Replace = 2,
        VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP, //IncreaseClamp = 3,
        VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP,  //IncreaseWrap = 4,
        VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP, //DecraseClamp = 5,
        VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP,  //DecraseWrap = 6,
        VkStencilOp::VK_STENCIL_OP_INVERT               //Invert = 7
    };

    static const VkDescriptorType VulkanDescriptorTypeMapping[] = {
        VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
        VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,

    };

    void VulkanMapStencilFaceSetting(VkStencilOpState& faceSetting,
        const GraphicPipelineState::DepthStencilState::StencilDesc& faceDesc,
        const GraphicPipelineState::DepthStencilState& dsDesc)
    {
        faceSetting.failOp = VulkanStencilOpMapping[static_cast<uint32_t>(faceDesc.StenciFailOp)];
        faceSetting.passOp = VulkanStencilOpMapping[static_cast<uint32_t>(faceDesc.StencilPassOp)];
        faceSetting.depthFailOp = VulkanStencilOpMapping[static_cast<uint32_t>(faceDesc.DepthFailOp)];
        faceSetting.compareOp = VulkanCompareOpMapping[static_cast<uint32_t>(faceDesc.StencilTestFunction)];
        faceSetting.reference = dsDesc.StencilReference;
        faceSetting.compareMask = dsDesc.StencilReadMask;
        faceSetting.writeMask = dsDesc.StencilWriteMask;
    };

    GraphicPipelineState* GraphicDeviceVulkan::CreateGraphicPipelineState(const GraphicPipelineState::CreateInfo& info)
    {
        if (info.VertexInputLayout.NumBindingArray == 0 || info.VertexInputLayout.NumAttributeArray == 0)
        {
            return nullptr;
        }

        //Stages
        std::vector<VkPipelineShaderStageCreateInfo> Stages;
        for (int32_t stageIndex = 0; stageIndex < static_cast<uint32_t>(GraphicPipelineState::EShaderStage::Num); stageIndex++)
        {
            if (info.ShaderModuleDesc.StageShaders[stageIndex] != nullptr)
            {
                ShaderVulkan* StageShader = reinterpret_cast<ShaderVulkan*>(info.ShaderModuleDesc.StageShaders[stageIndex]);
                VkPipelineShaderStageCreateInfo StageCreateInfo;
                VulkanZeroMemory(StageCreateInfo);
                StageCreateInfo.stage = VulkanPipelineShaderStageMapping[stageIndex];
                StageCreateInfo.module = StageShader->GetVulkanShaderModule();
                StageCreateInfo.pName = StageShader->GetEntryPointName();
                //TODO: no need to do in current state.
                // Change constant variable in runtime.
                //StageCreateInfo.pSpecializationInfo;
                Stages.emplace_back(StageCreateInfo);
            }
        }

        if (Stages.size() == 0)
        {
            return nullptr;
        }

        //VertexInputState
        VkPipelineVertexInputStateCreateInfo VertexInputStateInfo;
        VulkanZeroMemory(VertexInputStateInfo);
        std::vector<VkVertexInputBindingDescription> VertexBindings(info.VertexInputLayout.NumBindingArray);
        std::vector<VkVertexInputAttributeDescription> VertexAttributes(info.VertexInputLayout.NumAttributeArray);

        for (uint32_t BindingIndex = 0; BindingIndex < info.VertexInputLayout.NumBindingArray; BindingIndex++)
        {
            VkVertexInputBindingDescription& VulkanBinding = VertexBindings[BindingIndex];
            GraphicPipelineState::VertexInputLayout::Binding& Binding = info.VertexInputLayout.BindingArray[BindingIndex];
            VulkanBinding.binding = Binding.Index;
            VulkanBinding.inputRate = VulkanVertexInputRateMapping[static_cast<uint32_t>(Binding.InputRate)];
            VulkanBinding.stride = Binding.Stride;
        }

        for (uint32_t AttributeIndex = 0; AttributeIndex < info.VertexInputLayout.NumAttributeArray; AttributeIndex++)
        {
            VkVertexInputAttributeDescription& VulkanAttribute = VertexAttributes[AttributeIndex];
            GraphicPipelineState::VertexInputLayout::Attribute& Attribute = info.VertexInputLayout.AttributeArray[AttributeIndex];
            VulkanAttribute.location = Attribute.LocationIndex;
            VulkanAttribute.binding = Attribute.BingdingIndex;
            VulkanAttribute.format = VulkanVertexFormatMapping[static_cast<uint32_t>(Attribute.VertexFormat)];
            VulkanAttribute.offset = Attribute.VertexOffset;
        }

        VertexInputStateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(VertexBindings.size());
        VertexInputStateInfo.pVertexBindingDescriptions = VertexBindings.data();
        VertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexAttributes.size());
        VertexInputStateInfo.pVertexAttributeDescriptions = VertexAttributes.data();

        //Input Assemble
        VkPipelineInputAssemblyStateCreateInfo InputAssembleInfo;
        VulkanZeroMemory(InputAssembleInfo);
        InputAssembleInfo.topology = VulkanPrimitiveTopologyMapping[static_cast<uint32_t>(info.PrimitiveTopology)];

        //Viewport
        VkPipelineViewportStateCreateInfo ViewportInfo;
        VulkanZeroMemory(ViewportInfo);

        VkViewport  ViewportSetting;
        VkRect2D    ScissorSetting;

        ViewportSetting.x           = info.RasterizationState.ViewportInfo.X;
        ViewportSetting.y           = info.RasterizationState.ViewportInfo.Y;
        ViewportSetting.width       = info.RasterizationState.ViewportInfo.Width;
        ViewportSetting.height      = info.RasterizationState.ViewportInfo.Height;
        ViewportSetting.minDepth    = info.RasterizationState.ViewportInfo.MinDepth;
        ViewportSetting.maxDepth    = info.RasterizationState.ViewportInfo.MaxDepth;

        ScissorSetting.offset.x     = static_cast<int32_t>(info.RasterizationState.ViewportInfo.X);
        ScissorSetting.offset.y     = static_cast<int32_t>(info.RasterizationState.ViewportInfo.Y);
        ScissorSetting.extent.width = static_cast<uint32_t>(info.RasterizationState.ViewportInfo.Width);
        ScissorSetting.extent.height= static_cast<uint32_t>(info.RasterizationState.ViewportInfo.Height);

        ViewportInfo.viewportCount = 1;
        ViewportInfo.pViewports = &ViewportSetting;
        ViewportInfo.scissorCount = 1;
        ViewportInfo.pScissors = &ScissorSetting;
        

        VkPipelineRasterizationStateCreateInfo RasterizationStateInfo;
        VulkanZeroMemory(RasterizationStateInfo);
        RasterizationStateInfo.depthClampEnable = VK_TRUE;
        RasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
        RasterizationStateInfo.polygonMode = VulkanPolygonModeMapping[static_cast<uint32_t>(info.RasterizationState.FillMode)];
        RasterizationStateInfo.cullMode    = VulkanCullModeMapping[static_cast<uint32_t>(info.RasterizationState.CullMode)];
        RasterizationStateInfo.frontFace   = VulkanFrontFaceMapping[static_cast<uint32_t>(info.RasterizationState.Frontface)];
        RasterizationStateInfo.depthBiasEnable = (info.RasterizationState.DepthBias > 0) ? VK_TRUE : VK_FALSE;
        RasterizationStateInfo.depthBiasConstantFactor = 1.0f;
        RasterizationStateInfo.depthBiasClamp = info.RasterizationState.DepthBiasClamp;
        RasterizationStateInfo.depthBiasSlopeFactor = info.RasterizationState.SlopeScaledDepthBias;
        RasterizationStateInfo.lineWidth = 1.0f;

        //Multisample State
        VkPipelineMultisampleStateCreateInfo MultisampleStateInfo;
        VulkanZeroMemory(MultisampleStateInfo);
        MultisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        MultisampleStateInfo.sampleShadingEnable = VK_FALSE;
        MultisampleStateInfo.minSampleShading = 1.0;
        MultisampleStateInfo.pSampleMask = nullptr;
        MultisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
        MultisampleStateInfo.alphaToOneEnable = VK_FALSE;


        //Depth Stencil
        VkPipelineDepthStencilStateCreateInfo DepthStencilStateInfo;
        VulkanZeroMemory(DepthStencilStateInfo);
        DepthStencilStateInfo.depthTestEnable   = info.DepthStencilState.UsingDepthTest ? VK_TRUE : VK_FALSE;
        DepthStencilStateInfo.depthWriteEnable  = info.DepthStencilState.EnableDepthWrite ? VK_TRUE : VK_FALSE;
        DepthStencilStateInfo.depthCompareOp    = VulkanCompareOpMapping[static_cast<uint32_t>(info.DepthStencilState.DepthTestFunction)];
        DepthStencilStateInfo.stencilTestEnable = info.DepthStencilState.UsingStencilTest ? VK_TRUE : VK_FALSE;
        VulkanMapStencilFaceSetting(DepthStencilStateInfo.front, info.DepthStencilState.StencilFrontFace, info.DepthStencilState);
        VulkanMapStencilFaceSetting(DepthStencilStateInfo.back, info.DepthStencilState.StencilBackFace, info.DepthStencilState);

        //BlendState
        bool bUseColorBlend = info.ColorBlendState.NumAttachments > 0;
        VkPipelineColorBlendStateCreateInfo ColorBlendStateInfo;
        VulkanZeroMemory(ColorBlendStateInfo);

        std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachmentStates;
        if (bUseColorBlend)
        {
            ColorBlendAttachmentStates.resize(info.ColorBlendState.NumAttachments);
            ColorBlendStateInfo.attachmentCount = info.ColorBlendState.NumAttachments;
            for (uint32_t index = 0; index < info.ColorBlendState.NumAttachments; index++)
            {
                using AttachmentBlendState = GFXI::GraphicPipelineState::ColorBlendState::AttachmentBlendState;
                VkPipelineColorBlendAttachmentState& VulkanAttachmentState = ColorBlendAttachmentStates[index];
                const AttachmentBlendState& AttachmentState = info.ColorBlendState.AttachmentBlendStates[index];

                VulkanAttachmentState.blendEnable = AttachmentState.UsingBlend ? VK_TRUE : VK_FALSE;

                VulkanAttachmentState.srcColorBlendFactor   = VulkanBlendFactorMapping[static_cast<uint32_t>(AttachmentState.ColorSrcFactor)];
                VulkanAttachmentState.dstColorBlendFactor   = VulkanBlendFactorMapping[static_cast<uint32_t>(AttachmentState.ColorDstFactor)];
                VulkanAttachmentState.colorBlendOp          = VulkanBlendOpMapping[static_cast<uint32_t>(AttachmentState.ColorBlendOp)];
                VulkanAttachmentState.srcAlphaBlendFactor   = VulkanBlendFactorMapping[static_cast<uint32_t>(AttachmentState.AlphaSrcFactor)];;
                VulkanAttachmentState.dstAlphaBlendFactor   = VulkanBlendFactorMapping[static_cast<uint32_t>(AttachmentState.AlphaDstFactor)];;
                VulkanAttachmentState.alphaBlendOp          = VulkanBlendOpMapping[static_cast<uint32_t>(AttachmentState.AlphaBlendOp)];
                if (VulkanAttachmentState.srcAlphaBlendFactor == VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR)
                {
                    VulkanAttachmentState.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_ALPHA;
                }
                if (VulkanAttachmentState.dstAlphaBlendFactor == VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR)
                {
                    VulkanAttachmentState.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_ALPHA;
                }

                VkColorComponentFlags VulkanColorWriteMask = 0;
                if (AttachmentState.ColorWriteR) VulkanColorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
                if (AttachmentState.ColorWriteG) VulkanColorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
                if (AttachmentState.ColorWriteB) VulkanColorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
                if (AttachmentState.ColorWriteA) VulkanColorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
                VulkanAttachmentState.colorWriteMask = VulkanColorWriteMask;
            }     
            ColorBlendStateInfo.pAttachments = ColorBlendAttachmentStates.data();
            for (int index = 0; index < 4; index++)
            {
                ColorBlendStateInfo.blendConstants[index] = info.ColorBlendState.BlendConstant[index];
            }
        }

        VkPipelineDynamicStateCreateInfo DynamicStateInfo;
        VulkanZeroMemory(DynamicStateInfo);
        VkDynamicState DynamicStates[] = {
            VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
            VkDynamicState::VK_DYNAMIC_STATE_SCISSOR
        };
        uint32_t NumDynamicState = sizeof(DynamicStates) / sizeof(VkDynamicState);
        DynamicStateInfo.dynamicStateCount = NumDynamicState;
        DynamicStateInfo.pDynamicStates = DynamicStates;

        //Layout
        std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
        for (uint32_t index = 0; index < info.ShaderModuleDesc.NumDescriptorSetLayouts; index++)
        {
            DescriptorSetLayoutVulkan* VulkanLayout = reinterpret_cast<DescriptorSetLayoutVulkan*>(info.ShaderModuleDesc.DescriptorSetLayouts[index]);
            DescriptorSetLayouts.emplace_back(VulkanLayout->GetVulkanDescriptorSetLayout());
        }

        VkPipelineLayoutCreateInfo LayoutCreateInfo;
        VulkanZeroMemory(LayoutCreateInfo);
        LayoutCreateInfo.setLayoutCount = info.ShaderModuleDesc.NumDescriptorSetLayouts;
        LayoutCreateInfo.pSetLayouts = DescriptorSetLayouts.data();
        LayoutCreateInfo.pushConstantRangeCount = 0;
        LayoutCreateInfo.pPushConstantRanges = nullptr;

        VkPipelineLayout VulkanPipelineLayout;
        VkResult RetCreatePipelineLayout = vkCreatePipelineLayout(mVulkanDevice, &LayoutCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanPipelineLayout);
        if (RetCreatePipelineLayout != VkResult::VK_SUCCESS)
        {
            return nullptr;
        }

        VkPipelineRenderingCreateInfo RenderingCreateInfo;
        VulkanZeroMemory(RenderingCreateInfo);
        std::vector<VkFormat> ColorAttachmentFormats(info.RenderAttachemntDesc.NumColorAttachments);
        for (uint32_t index = 0; index < info.RenderAttachemntDesc.NumColorAttachments; index++)
        {
            ColorAttachmentFormats[index] = VulkanRenderTargetFormatMapping[static_cast<uint32_t>(info.RenderAttachemntDesc.ColorAttachmentFormats[index])];
        }
        RenderingCreateInfo.viewMask                = info.RenderAttachemntDesc.AticveViewMask;
        RenderingCreateInfo.colorAttachmentCount    = info.RenderAttachemntDesc.NumColorAttachments;
        RenderingCreateInfo.pColorAttachmentFormats = ColorAttachmentFormats.data();
        RenderingCreateInfo.depthAttachmentFormat   = VulkanDepthFormatMapping[static_cast<uint32_t>(info.RenderAttachemntDesc.DepthStencilFormat)];
        RenderingCreateInfo.stencilAttachmentFormat = VulkanStencilFormatMapping[static_cast<uint32_t>(info.RenderAttachemntDesc.DepthStencilFormat)];

        VkGraphicsPipelineCreateInfo GfxPipelineCreateInfo;
        VulkanZeroMemory(GfxPipelineCreateInfo);
        GfxPipelineCreateInfo.pNext                 = &RenderingCreateInfo;
        GfxPipelineCreateInfo.stageCount            = static_cast<uint32_t>(Stages.size());
        GfxPipelineCreateInfo.pStages               = Stages.data();
        GfxPipelineCreateInfo.pVertexInputState     = &VertexInputStateInfo;
        GfxPipelineCreateInfo.pInputAssemblyState   = &InputAssembleInfo;
        //VkPipelineTessellationStateCreateInfo* GfxPipelineCreateInfo.pTessellationState;
        GfxPipelineCreateInfo.pViewportState        = &ViewportInfo;
        GfxPipelineCreateInfo.pRasterizationState   = &RasterizationStateInfo;
        GfxPipelineCreateInfo.pMultisampleState     = &MultisampleStateInfo;
        GfxPipelineCreateInfo.pDepthStencilState    = &DepthStencilStateInfo;
        GfxPipelineCreateInfo.pColorBlendState      = &ColorBlendStateInfo;
        GfxPipelineCreateInfo.pDynamicState         = &DynamicStateInfo;
        GfxPipelineCreateInfo.layout                = VulkanPipelineLayout;
        GfxPipelineCreateInfo.renderPass            = VK_NULL_HANDLE;
        GfxPipelineCreateInfo.subpass               = info.RenderAttachemntDesc.SubPassIndex;
        //VkPipeline       //GfxPipelineCreateInfo.basePipelineHandle;
        //int32_t          //GfxPipelineCreateInfo.basePipelineIndex;

        VkPipeline VulkanPipeline;
        VkResult RetCreateGfxPipeline = vkCreateGraphicsPipelines(mVulkanDevice, VK_NULL_HANDLE, 1, &GfxPipelineCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanPipeline);
        if (RetCreateGfxPipeline == VkResult::VK_SUCCESS)
        {
            vkDestroyPipelineLayout(mVulkanDevice, VulkanPipelineLayout, GFX_VK_ALLOCATION_CALLBACK);
            GraphicPipelineStateVulkan* vulkanPipelineState = new GraphicPipelineStateVulkan(this, VulkanPipeline);
            return vulkanPipelineState;
        }
        else
        {
            vkDestroyPipelineLayout(mVulkanDevice, VulkanPipelineLayout, GFX_VK_ALLOCATION_CALLBACK);
            return nullptr;
        }
    }

    //TODO:
    static const shaderc_shader_kind VulkanShaderTypeMapping[] = {
            shaderc_shader_kind::shaderc_vertex_shader,         //VertexShader = 0,
            shaderc_shader_kind::shaderc_fragment_shader,       //PixelShader = 1,
            shaderc_shader_kind::shaderc_geometry_shader,       //GeometryShader = 2,
            shaderc_shader_kind::shaderc_tess_control_shader,   //DomainShader = 3,
            shaderc_shader_kind::shaderc_tess_evaluation_shader,//HullShader = 4,
            shaderc_shader_kind::shaderc_compute_shader         //ComputeShader = 5,
    };
    ShaderBinary* GraphicDeviceVulkan::CompileShader(const ShaderBinary::CreateInfo& info)
    {
        shaderc_shader_kind ShaderType = VulkanShaderTypeMapping[static_cast<uint32_t>(info.ShaderType)];
        const char* InputFileName = info.ShaderNameString;
        shaderc::Compiler ShaderCompiler;
        shaderc::CompileOptions CompileOptions;

        //CompileOptions.SetSourceLanguage(shaderc_source_language::shaderc_source_language_hlsl);
        
        //TODO: replace EntryPoint to main() in GLSL SourceCodeData.
        // For GLSL compilation, the entry point name is assumed to be "main".
        shaderc::SpvCompilationResult SpirVCompilationResult = ShaderCompiler.CompileGlslToSpv(
            static_cast<const char*>(info.ShaderSourceCodeData),
            static_cast<size_t>(info.ShaderSourceCodeLength),
            ShaderType,
            InputFileName,
            info.EntryNameString,
            CompileOptions);

        shaderc_compilation_status RetCompilationStatus = SpirVCompilationResult.GetCompilationStatus();
        if (RetCompilationStatus == shaderc_compilation_status::shaderc_compilation_status_success)
        {
            const unsigned char* IterBegin = reinterpret_cast<const unsigned char*>(SpirVCompilationResult.begin());
            const unsigned char* IterEnd = reinterpret_cast<const unsigned char*>(SpirVCompilationResult.end());
            std::vector<unsigned char> SpirVBinaries(IterBegin, IterEnd);
            return new ShaderBinaryVulkan(info.ShaderType, std::move(SpirVBinaries), info.ShaderNameString, info.EntryNameString);
        }

        std::string error = SpirVCompilationResult.GetErrorMessage();
        size_t NumCompileErrors = SpirVCompilationResult.GetNumErrors();
        return nullptr;
    }

    Shader* GraphicDeviceVulkan::CreateShader(const Shader::CreateInfo& info)
    {
        ShaderBinaryVulkan* SpirVBinary = reinterpret_cast<ShaderBinaryVulkan*>(info.ShaderBinary);
        VkShaderModuleCreateInfo ShaderCreateInfo;
        VkShaderModule ShaderModule;

        VulkanZeroMemory(ShaderCreateInfo);
        ShaderCreateInfo.pCode      = reinterpret_cast<uint32_t*>(SpirVBinary->GetBytecode());
        ShaderCreateInfo.codeSize   = SpirVBinary->GetBytecodeLength();

        VkResult RetCreateShaderModule = vkCreateShaderModule(mVulkanDevice, &ShaderCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &ShaderModule);
        if (RetCreateShaderModule == VkResult::VK_SUCCESS)
        {
            unsigned char* IterBegin = reinterpret_cast<unsigned char*>(SpirVBinary->GetBytecode());
            unsigned char* IterEnd = IterBegin + SpirVBinary->GetBytecodeLength();
            std::vector<unsigned char> Binary(IterBegin, IterEnd);
            return new ShaderVulkan(this, SpirVBinary->GetShaderType(), ShaderModule, std::move(Binary), SpirVBinary->GetShaderName(), SpirVBinary->GetEntryPointName());
        }
        return nullptr;
    }

    VkShaderStageFlags VulkanShaderStageFlagsMapping(uint32_t bindingShaderStageFlags)
    {

        if (bindingShaderStageFlags == GFXI::DescriptorSetLayout::EShaderStageFlags::AllStageBits)
        {
            return VK_SHADER_STAGE_ALL;
        }
        else
        {
            auto CheckStageFlagBits = [&val = bindingShaderStageFlags](uint32_t mask, uint32_t mapping) -> uint32_t
            { return (val & mask) > 0 ? mapping : 0; };

            typedef GFXI::DescriptorSetLayout::EShaderStageFlags Flags;
            const uint32_t& v = bindingShaderStageFlags;
            //TODO:
            // Implement Domain & Hull Shader.
            uint32_t VSBits = CheckStageFlagBits(Flags::VertexShaderStageBits,   VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
            uint32_t PSBits = CheckStageFlagBits(Flags::PixelShaderStageBits,    VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
            uint32_t GSBits = CheckStageFlagBits(Flags::GeometryShaderStageBits, VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT);
            uint32_t DSBits = CheckStageFlagBits(Flags::DomainShaderStageBits,   0);//VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
            uint32_t HSBits = CheckStageFlagBits(Flags::HullShaderStageBits,     0);//VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
            uint32_t CSBits = CheckStageFlagBits(Flags::ComputeShaderStageBits,  VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);

            return VSBits | PSBits | GSBits | DSBits | HSBits | CSBits;
            //TODO: Ray-Tracing.
            // Provided by VK_KHR_ray_tracing_pipeline
            //VK_SHADER_STAGE_RAYGEN_BIT_KHR = 0x00000100,
            //// Provided by VK_KHR_ray_tracing_pipeline
            //VK_SHADER_STAGE_ANY_HIT_BIT_KHR = 0x00000200,
            //// Provided by VK_KHR_ray_tracing_pipeline
            //VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR = 0x00000400,
            //// Provided by VK_KHR_ray_tracing_pipeline
            //VK_SHADER_STAGE_MISS_BIT_KHR = 0x00000800,
            //// Provided by VK_KHR_ray_tracing_pipeline
            //VK_SHADER_STAGE_INTERSECTION_BIT_KHR = 0x00001000,
            //// Provided by VK_KHR_ray_tracing_pipeline
            //VK_SHADER_STAGE_CALLABLE_BIT_KHR = 0x00002000,
            //// Provided by VK_EXT_mesh_shader
            //VK_SHADER_STAGE_TASK_BIT_EXT = 0x00000040,
            //// Provided by VK_EXT_mesh_shader
            //VK_SHADER_STAGE_MESH_BIT_EXT = 0x00000080,
            //// Provided by VK_HUAWEI_subpass_shading
            //VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI = 0x00004000,
            //// Provided by VK_HUAWEI_cluster_culling_shader
            //VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI = 0x00080000,
            //// Provided by VK_NV_ray_tracing
            //VK_SHADER_STAGE_RAYGEN_BIT_NV = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            //// Provided by VK_NV_ray_tracing
            //VK_SHADER_STAGE_ANY_HIT_BIT_NV = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
            //// Provided by VK_NV_ray_tracing
            //VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
            //// Provided by VK_NV_ray_tracing
            //VK_SHADER_STAGE_MISS_BIT_NV = VK_SHADER_STAGE_MISS_BIT_KHR,
            //// Provided by VK_NV_ray_tracing
            //VK_SHADER_STAGE_INTERSECTION_BIT_NV = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
            //// Provided by VK_NV_ray_tracing
            //VK_SHADER_STAGE_CALLABLE_BIT_NV = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
            //// Provided by VK_NV_mesh_shader
            //VK_SHADER_STAGE_TASK_BIT_NV = VK_SHADER_STAGE_TASK_BIT_EXT,
            //// Provided by VK_NV_mesh_shader
            //VK_SHADER_STAGE_MESH_BIT_NV =  VK_SHADER_STAGE_MESH_BIT_EXT,
        }
    }

    DescriptorSetLayout* GraphicDeviceVulkan::CreateDescriptorSetLayout(const DescriptorSetLayout::CreateInfo& createInfo)
    {
        std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;

        for (uint32_t bindingIndex = 0; bindingIndex < createInfo.NumDescriptorBindings; bindingIndex++)
        {
            GFXI::DescriptorSetLayout::CreateInfo::DescriptorDesc& desc = createInfo.DescriptorBindings[bindingIndex];

            VkDescriptorSetLayoutBinding binding;

            //TODO:
            // fix binding index after update Graphic API.
            binding.binding         = bindingIndex;
            binding.descriptorType  = VulkanDescriptorTypeMapping[static_cast<uint32_t>(desc.DescriptorType)];
            binding.descriptorCount = desc.NumDescriptors;
            binding.stageFlags      = VulkanShaderStageFlagsMapping(desc.ShaderStageFlags);
            binding.pImmutableSamplers = VK_NULL_HANDLE;
            DescriptorSetLayoutBindings.emplace_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
        VulkanZeroMemory(DescriptorSetLayoutCreateInfo);
        DescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(DescriptorSetLayoutBindings.size());
        DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBindings.data();

        VkDescriptorSetLayout DescriptorSetLayouts;
        VkResult RetCreateDescriptorSetLayout = vkCreateDescriptorSetLayout(mVulkanDevice, &DescriptorSetLayoutCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &DescriptorSetLayouts);

        if (RetCreateDescriptorSetLayout == VkResult::VK_SUCCESS)
        {
            return new DescriptorSetLayoutVulkan(this, DescriptorSetLayouts);
        }
        return nullptr;
    }

    SamplerState* GraphicDeviceVulkan::CreateSamplerState(const SamplerState::CreateInfo&)
    {
        return nullptr;
    }
}
