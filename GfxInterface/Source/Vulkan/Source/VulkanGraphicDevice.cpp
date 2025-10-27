#include "VulkanGraphicDevice.h"
#include "VulkanGraphicModule.h"
#include "VulkanShader.h"
#include "VulkanSwapChain.h"



namespace GFXI
{
    GraphicDeviceVulkan::GraphicDeviceVulkan(GraphicModuleVulkan* belongsTo,
        VkPhysicalDevice physicalDevice,    VkDevice device,
        uint32_t graphicQueueFamilyIndex,   uint32_t graphicQueueIndex,
        uint32_t computeQueueFamilyIndex,   uint32_t computeQueueIndex,
        uint32_t transferQueueFamilyIndex,  uint32_t transferQueueIndex
    )
        : mBelongsTo(belongsTo)
        , mVulkanPhysicalDevice(physicalDevice)
        , mVulkanDevice(device)
        , mGraphicQueueFamilyIndex(graphicQueueFamilyIndex),   mGraphicQueueIndex(graphicQueueIndex)
        , mComputeQueueFamilyIndex(computeQueueFamilyIndex),   mComputeQueueIndex(computeQueueIndex)
        , mTransferQueueFamilyIndex(transferQueueFamilyIndex), mTransferQueueIndex(transferQueueIndex)
    {
        vkGetDeviceQueue(device, graphicQueueFamilyIndex,  graphicQueueIndex,  &mVulkanGraphicQueue);
        vkGetDeviceQueue(device, computeQueueFamilyIndex,  computeQueueIndex,  &mVulkanComputeQueue);
        vkGetDeviceQueue(device, transferQueueFamilyIndex, transferQueueIndex, &mVulkanTransferQueue);
    }

    GraphicDeviceVulkan::~GraphicDeviceVulkan()
    {
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
            uint32_t PresentQueueFamilyIndex = mComputeQueueFamilyIndex;
            VkBool32 bSupportPresent = false;
            VkResult RetCheckSurfaceSupport = vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice, mComputeQueueFamilyIndex, Surface, &bSupportPresent);
            if (RetCheckSurfaceSupport != VkResult::VK_SUCCESS || !bSupportPresent)
            {
                PresentQueueFamilyIndex = mGraphicQueueFamilyIndex;
                RetCheckSurfaceSupport = vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice, mGraphicQueueFamilyIndex, Surface, &bSupportPresent);
            }

            if (bSupportPresent)
            {
                mVulkanPresentQueue = (PresentQueueFamilyIndex == mComputeQueueFamilyIndex) ? mVulkanComputeQueue : mVulkanGraphicQueue;

                bool bSupportBGRA8_SRGB = false;
                bool bSupportFIFOPresent = false;
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
                        if (PresentMode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR)
                        {
                            bSupportFIFOPresent = true;
                            break;
                        }
                    }
                }

                if (bSupportBGRA8_SRGB && bSupportFIFOPresent)
                {
                    VkSurfaceCapabilitiesKHR SurfaceCaps;
                    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVulkanPhysicalDevice, Surface, &SurfaceCaps);

                    auto clamp = [](int v, int min, int max) { return (v > max) ? max : (v < min ? min : v); };
                    windowWidth = clamp(windowWidth, SurfaceCaps.minImageExtent.width, SurfaceCaps.maxImageExtent.width);
                    windowHeight = clamp(windowHeight, SurfaceCaps.minImageExtent.height, SurfaceCaps.maxImageExtent.height);

                    //TODO: check and determin swapchain image count.
                    uint32_t NumSwapchainImages = 3;
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
                    SwapChainCreateInfo.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
                    SwapChainCreateInfo.clipped = VK_TRUE;
                    //TODO:
                    // Set to last if re-creating swapchain.
                    SwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

                    VkSwapchainKHR VulkanSwapchain;
                    VkResult RetCreateSwapChain = vkCreateSwapchainKHR(mVulkanDevice, &SwapChainCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanSwapchain);

                    if (RetCreateSwapChain == VK_SUCCESS)
                    {
                        //TODO: create image before creating swapchain.
                        SwapChainVulkan* SwapChain = new SwapChainVulkan(this, VulkanSwapchain);
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
        for (int32_t stageIndex = 0; stageIndex < GraphicPipelineState::kNumShaderStage; stageIndex++)
        {
            if (info.StageShaders[stageIndex] != nullptr)
            {
                ShaderVulkan* StageShader = reinterpret_cast<ShaderVulkan*>(info.StageShaders[stageIndex]);
                VkPipelineShaderStageCreateInfo StageCreateInfo;
                VulkanZeroMemory(StageCreateInfo);
                StageCreateInfo.stage = VulkanPipelineShaderStageMapping[static_cast<unsigned char>(StageShader->GetShaderType())];
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
        VkDynamicState DynamicStates[] = { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
        uint32_t NumDynamicState = sizeof(DynamicStates) / sizeof(VkDynamicState);
        DynamicStateInfo.dynamicStateCount = NumDynamicState;
        DynamicStateInfo.pDynamicStates = DynamicStates;

        //Layout
        std::vector<VkDescriptorSetLayoutBinding> LayoutBinding;
        VkDescriptorSetLayoutBinding binding;
        binding.binding = 0;
        binding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
        LayoutBinding.emplace_back(binding);

        binding.binding = 1;
        LayoutBinding.emplace_back(binding);

        //TODO:??
        //binding.binding = 2;
        //LayoutBinding.emplace_back(binding);

        VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
        VulkanZeroMemory(DescriptorSetLayoutCreateInfo);
        DescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(LayoutBinding.size());
        DescriptorSetLayoutCreateInfo.pBindings = LayoutBinding.data();

        VkDescriptorSetLayout DescriptorSetLayouts;
        VkResult RetCreateDescriptorSetLayout = vkCreateDescriptorSetLayout(mVulkanDevice, &DescriptorSetLayoutCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &DescriptorSetLayouts);
        if (RetCreateDescriptorSetLayout != VkResult::VK_SUCCESS)
        {
            return nullptr;
        }


        VkPipelineLayoutCreateInfo LayoutCreateInfo;
        VulkanZeroMemory(LayoutCreateInfo);
        LayoutCreateInfo.setLayoutCount = 1;
        LayoutCreateInfo.pSetLayouts = &DescriptorSetLayouts;
        LayoutCreateInfo.pushConstantRangeCount = 0;
        LayoutCreateInfo.pPushConstantRanges = nullptr;

        VkPipelineLayout VulkanPipelineLayout;
        VkResult RetCreatePipelineLayout = vkCreatePipelineLayout(mVulkanDevice, &LayoutCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanPipelineLayout);
        if (RetCreatePipelineLayout != VkResult::VK_SUCCESS)
        {
            vkDestroyDescriptorSetLayout(mVulkanDevice, DescriptorSetLayouts, GFX_VK_ALLOCATION_CALLBACK);
            return nullptr;
        }

        //RenderPass
        VkRenderPassCreateInfo RenderPassCreateInfo;
        VulkanZeroMemory(RenderPassCreateInfo);

        VkAttachmentDescription RenderPassAttachment;
        RenderPassAttachment.flags = 0;
        RenderPassAttachment.format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
        RenderPassAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
        RenderPassAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        RenderPassAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
        RenderPassAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        RenderPassAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
        RenderPassAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
        RenderPassAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef;
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription RenderPassSubpass;
        RenderPassSubpass.flags = 0;
        RenderPassSubpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
        RenderPassSubpass.inputAttachmentCount = 0;
        RenderPassSubpass.pInputAttachments = nullptr; 
        RenderPassSubpass.colorAttachmentCount = 1;
        RenderPassSubpass.pColorAttachments = &colorAttachmentRef;
        RenderPassSubpass.pResolveAttachments = nullptr;
        RenderPassSubpass.pDepthStencilAttachment = nullptr;
        RenderPassSubpass.preserveAttachmentCount = 0;
        RenderPassSubpass.pPreserveAttachments = nullptr;

        RenderPassCreateInfo.attachmentCount = 1;
        RenderPassCreateInfo.pAttachments = &RenderPassAttachment;
        RenderPassCreateInfo.subpassCount = 1;
        RenderPassCreateInfo.pSubpasses = &RenderPassSubpass;

        VkRenderPass VulkanRenderPass;
        VkResult RetCreateRenderPass = vkCreateRenderPass(mVulkanDevice, &RenderPassCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanRenderPass);
        if (RetCreateRenderPass != VkResult::VK_SUCCESS)
        {
            vkDestroyDescriptorSetLayout(mVulkanDevice, DescriptorSetLayouts, GFX_VK_ALLOCATION_CALLBACK);
            vkDestroyPipelineLayout(mVulkanDevice, VulkanPipelineLayout, GFX_VK_ALLOCATION_CALLBACK);
            return nullptr;
        }

        VkGraphicsPipelineCreateInfo GfxPipelineCreateInfo;
        VulkanZeroMemory(GfxPipelineCreateInfo);
        GfxPipelineCreateInfo.stageCount = static_cast<uint32_t>(Stages.size());
        GfxPipelineCreateInfo.pStages = Stages.data();
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
        GfxPipelineCreateInfo.renderPass            = VulkanRenderPass;
        GfxPipelineCreateInfo.subpass               = 0;
        //VkPipeline       //GfxPipelineCreateInfo.basePipelineHandle;
        //int32_t          //GfxPipelineCreateInfo.basePipelineIndex;

        VkPipeline VulkanPipeline;
        VkResult RetCreateGfxPipeline = vkCreateGraphicsPipelines(mVulkanDevice, VK_NULL_HANDLE, 1, &GfxPipelineCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanPipeline);
        if (RetCreateGfxPipeline == VkResult::VK_SUCCESS)
        {
            //TOOD:
            return nullptr;
        }

        vkDestroyDescriptorSetLayout(mVulkanDevice, DescriptorSetLayouts, GFX_VK_ALLOCATION_CALLBACK);
        vkDestroyPipelineLayout(mVulkanDevice, VulkanPipelineLayout, GFX_VK_ALLOCATION_CALLBACK);
        vkDestroyRenderPass(mVulkanDevice, VulkanRenderPass, GFX_VK_ALLOCATION_CALLBACK);
        return nullptr;
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

    SamplerState* GraphicDeviceVulkan::CreateSamplerState(const SamplerState::CreateInfo&)
    {
        return nullptr;
    }
}
