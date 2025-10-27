#include <string>
#include "D3D11Buffer.h"
#include "D3D11CommandQueue.h"
#include "D3D11GraphicDevice.h"
#include "D3D11GraphicModule.h"
#include "D3D11PipelineState.h"
#include "D3D11SamplerState.h"
#include "D3D11Shader.h"
#include "D3D11SwapChain.h"
#include "D3D11Texture.h"


namespace GFXI
{
    GraphicDeviceD3D11::GraphicDeviceD3D11(GraphicModuleD3D11* belongsTo, ID3D11Device* device, ID3D11DeviceContext* immediateContext, ID3D11DeviceContext* deferredContext)
        : mBelongsTo(belongsTo)
        , mD3D11Device(device)
        , mImmediateContext(immediateContext)
        , mDeferredContext(deferredContext)
    {

    }

    GraphicDeviceD3D11::~GraphicDeviceD3D11()
    {
        CommandQueueD3D11::FreeAllUnusedCommandQueues();

        mDeferredContext.Release();
        mImmediateContext.Release();
        mD3D11Device.Reset();
    }

    SwapChain* GraphicDeviceD3D11::CreateSwapChain(void* windowHandle, int32_t windowWidth, int32_t windowHeight, bool isFullscreen)
    {
        const bool kIsUsedForShaders = true;
        const uint32_t  kSampleCount = 1;
        const uint32_t  kSampleQuality = 0;
        const DXGI_FORMAT kBackbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        swapChainDesc.Width = windowWidth;
        swapChainDesc.Height = windowHeight;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferUsage = kIsUsedForShaders
            ? (DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT)
            : DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = kSampleCount;
        swapChainDesc.SampleDesc.Quality = kSampleQuality;
        swapChainDesc.Flags = 0;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Format = kBackbufferFormat;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
        fullScreenDesc.RefreshRate.Numerator = 60;
        fullScreenDesc.RefreshRate.Denominator = 1;
        fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
        fullScreenDesc.Windowed = !isFullscreen;

        ComPtr<IDXGIOutput> output = nullptr;
        //UINT outputIndex = 0;
        //while (DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(OutputIndex, &Output))
        //{
        //    DXGI_OUTPUT_DESC OutputDesc;
        //    output->GetDesc(&OutputDesc);
        //    outputIndex++;
        //}


        ComPtr<IDXGISwapChain1> swapChain;
        IDXGIFactory2* DXGIFactory = (IDXGIFactory2*)mBelongsTo->GetDXGIFactory();
        HRESULT retCreateSwapChain = DXGIFactory->CreateSwapChainForHwnd(mD3D11Device.Get(), (HWND)windowHandle, &swapChainDesc, &fullScreenDesc, output.Get(), &swapChain);

        if (SUCCEEDED(retCreateSwapChain))
        {
            ///* CreateVertexBuffer SwapChain */
            bool kUsedByShader = true;
            //mBackbufferRenderTarget = std::make_unique<GfxRenderTarget>(ERenderTargetFormat::UNormRGBA8, usedForshader);
            //mBackbufferRenderTarget->mTexture2DDesc.Width = mClientWidth;
            //mBackbufferRenderTarget->mTexture2DDesc.Height = mClientHeight;

            ComPtr<ID3D11Texture2D> outBackbufferTexture = nullptr;
            HRESULT retGetDefaultBackbufferTexture = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &outBackbufferTexture);
            if (SUCCEEDED(retGetDefaultBackbufferTexture))
            {
                ComPtr<ID3D11RenderTargetView>      defaultBackbufferRTV = nullptr;
                ComPtr<ID3D11ShaderResourceView>    defaultBackbufferSRV = nullptr;

                D3D11_RENDER_TARGET_VIEW_DESC       renderTargetViewDesc;
                renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                renderTargetViewDesc.Texture2D.MipSlice = 0;

                D3D11_SHADER_RESOURCE_VIEW_DESC     shaderResourceViewDesc;
                shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
                shaderResourceViewDesc.Texture2D.MipLevels = -1;

                HRESULT retCreateDefaultBackbufferRTV = mD3D11Device->CreateRenderTargetView(outBackbufferTexture.Get(), &renderTargetViewDesc, &defaultBackbufferRTV);
                HRESULT retCreateDefaultBackbufferSRV = mD3D11Device->CreateShaderResourceView(outBackbufferTexture.Get(), &shaderResourceViewDesc, &defaultBackbufferSRV);
                if (SUCCEEDED(retCreateDefaultBackbufferSRV) && SUCCEEDED(retCreateDefaultBackbufferRTV))
                {
                    RenderTargetD3D11* backbufferRenderTarget = new RenderTargetD3D11(RenderTargetView::EFormat::R8G8B8A8_UNormInt, swapChainDesc.Width, swapChainDesc.Height, kUsedByShader, outBackbufferTexture.Detach(), defaultBackbufferRTV.Detach(), defaultBackbufferSRV.Detach());
                    return new SwapChainD3D11(swapChain.Detach(), backbufferRenderTarget);
                }
            }
        }

        return nullptr;
    }


    const D3D11_CULL_MODE D3D11CullModeMap[] =
    {
        D3D11_CULL_NONE,
        D3D11_CULL_FRONT,
        D3D11_CULL_BACK
    };
    const D3D11_FILL_MODE D3D11FillModeMap[] =
    {
        D3D11_FILL_SOLID, D3D11_FILL_WIREFRAME
    };
    const D3D11_BLEND_OP D3D11BlendOpMap[] =
    {
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_SUBTRACT,
        D3D11_BLEND_OP_REV_SUBTRACT,
        D3D11_BLEND_OP_MIN,
        D3D11_BLEND_OP_MAX
    };
    const D3D11_BLEND D3D11BlendFactorMap[] =
    {
        D3D11_BLEND_ZERO,
        D3D11_BLEND_ONE,
        D3D11_BLEND_SRC_COLOR,
        D3D11_BLEND_DEST_COLOR,
        D3D11_BLEND_BLEND_FACTOR,
        D3D11_BLEND_INV_SRC_COLOR,
        D3D11_BLEND_INV_DEST_COLOR,
        D3D11_BLEND_INV_BLEND_FACTOR,
        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_DEST_ALPHA,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_INV_DEST_ALPHA
    };
    const D3D11_COMPARISON_FUNC D3D11CompareFunctionMap[] =
    {
        D3D11_COMPARISON_NEVER,
        D3D11_COMPARISON_ALWAYS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_GREATER_EQUAL
    };
    const D3D11_STENCIL_OP D3D11StencilOpMap[] =
    {
        D3D11_STENCIL_OP_ZERO,
        D3D11_STENCIL_OP_KEEP,
        D3D11_STENCIL_OP_REPLACE,
        D3D11_STENCIL_OP_INCR_SAT,
        D3D11_STENCIL_OP_INCR,
        D3D11_STENCIL_OP_DECR_SAT,
        D3D11_STENCIL_OP_DECR,
        D3D11_STENCIL_OP_INVERT
    };

    const D3D11_PRIMITIVE_TOPOLOGY D3D11PrimitiveTopologyMap[] =
    {
        D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
        D3D_PRIMITIVE_TOPOLOGY_LINELIST,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
        D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ
    };

    const D3D11_INPUT_CLASSIFICATION D3D11InstanceRateMap[] = { D3D11_INPUT_PER_VERTEX_DATA, D3D11_INPUT_PER_INSTANCE_DATA };

    const DXGI_FORMAT D3DVertexFormatMap[] =
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_UINT,
        DXGI_FORMAT_R32G32B32A32_SINT,

        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32_UINT,
        DXGI_FORMAT_R32G32B32_SINT,

        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32_UINT,
        DXGI_FORMAT_R32G32_SINT
    };

    void StencilDescMap(const GFXI::GraphicPipelineState::DepthStencilState::StencilDesc& stencilFaceDesc, D3D11_DEPTH_STENCILOP_DESC& outD3D11StencilDesc)
    {
        outD3D11StencilDesc.StencilFailOp       = D3D11StencilOpMap[static_cast<uint32_t>(stencilFaceDesc.StenciFailOp)];
        outD3D11StencilDesc.StencilDepthFailOp  = D3D11StencilOpMap[static_cast<uint32_t>(stencilFaceDesc.DepthFailOp)];
        outD3D11StencilDesc.StencilPassOp       = D3D11StencilOpMap[static_cast<uint32_t>(stencilFaceDesc.StencilPassOp)];
        outD3D11StencilDesc.StencilFunc         = D3D11CompareFunctionMap[static_cast<uint32_t>(stencilFaceDesc.StencilTestFunction)];
    }

    GraphicPipelineState* GraphicDeviceD3D11::CreateGraphicPipelineState(const GraphicPipelineState::CreateInfo& createInfo)
    {
        using DepthStencilState = GraphicPipelineState::DepthStencilState;
        using RasterizationState = GraphicPipelineState::RasterizationState;
        using ColorBlendState = GraphicPipelineState::ColorBlendState;

        const DepthStencilState& depthStencilStateDesc = createInfo.DepthStencilState;
        const RasterizationState& rasterizationStateDesc = createInfo.RasterizationState;
        const ColorBlendState& colorBlendStateDesc = createInfo.ColorBlendState;
        D3D11_RASTERIZER_DESC rasterizerDesc;
        rasterizerDesc.FillMode = D3D11FillModeMap[static_cast<uint32_t>(rasterizationStateDesc.FillMode)];
        rasterizerDesc.CullMode = D3D11CullModeMap[static_cast<uint32_t>(rasterizationStateDesc.CullMode)];
        rasterizerDesc.FrontCounterClockwise = rasterizationStateDesc.Frontface == RasterizationState::EFrontFace::CounterClockwise;
        rasterizerDesc.DepthBias = rasterizationStateDesc.DepthBias;
        rasterizerDesc.DepthBiasClamp = rasterizationStateDesc.DepthBiasClamp;
        rasterizerDesc.SlopeScaledDepthBias = rasterizationStateDesc.SlopeScaledDepthBias;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.ScissorEnable = rasterizationStateDesc.UseScissorClip;
        rasterizerDesc.MultisampleEnable = false;
        rasterizerDesc.AntialiasedLineEnable = rasterizationStateDesc.UseAntialiasedline;

        D3D11_BLEND_DESC colorBlendDesc;
        colorBlendDesc.AlphaToCoverageEnable = false;
        colorBlendDesc.IndependentBlendEnable = false;
        for (uint32_t blendIndex = 0; blendIndex < colorBlendStateDesc.NumAttachments; blendIndex++)
        {
            const ColorBlendState::AttachmentBlendState& attachmentBlendState = colorBlendStateDesc.AttachmentBlendStates[blendIndex];
            D3D11_RENDER_TARGET_BLEND_DESC& attachmentBlendDesc = colorBlendDesc.RenderTarget[blendIndex];
            attachmentBlendDesc.BlendEnable = attachmentBlendState.UsingBlend;
            attachmentBlendDesc.BlendOp         = D3D11BlendOpMap[static_cast<uint32_t>(attachmentBlendState.ColorBlendOp)];
            attachmentBlendDesc.BlendOpAlpha    = D3D11BlendOpMap[static_cast<uint32_t>(attachmentBlendState.AlphaBlendOp)];
            attachmentBlendDesc.SrcBlend        = D3D11BlendFactorMap[static_cast<uint32_t>(attachmentBlendState.ColorSrcFactor)];
            attachmentBlendDesc.DestBlend       = D3D11BlendFactorMap[static_cast<uint32_t>(attachmentBlendState.ColorDstFactor)];
            attachmentBlendDesc.SrcBlendAlpha   = D3D11BlendFactorMap[static_cast<uint32_t>(attachmentBlendState.AlphaSrcFactor)];
            attachmentBlendDesc.DestBlendAlpha  = D3D11BlendFactorMap[static_cast<uint32_t>(attachmentBlendState.AlphaDstFactor)];

            if (!attachmentBlendState.ColorWriteR || !attachmentBlendState.ColorWriteG || !attachmentBlendState.ColorWriteB || !attachmentBlendState.ColorWriteR)
            {
                attachmentBlendDesc.RenderTargetWriteMask = 0;
                if (attachmentBlendState.ColorWriteR) attachmentBlendDesc.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;
                if (attachmentBlendState.ColorWriteG) attachmentBlendDesc.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
                if (attachmentBlendState.ColorWriteB) attachmentBlendDesc.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
                if (attachmentBlendState.ColorWriteA) attachmentBlendDesc.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
            }
            else
            {
                attachmentBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            }
        }

        for (int BlendIndex = colorBlendStateDesc.NumAttachments; BlendIndex < 8; BlendIndex++)
        {
            colorBlendDesc.RenderTarget[BlendIndex].BlendEnable = false;
            colorBlendDesc.RenderTarget[BlendIndex].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        }

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        depthStencilDesc.DepthEnable = depthStencilStateDesc.UsingDepthTest;
        depthStencilDesc.DepthWriteMask = depthStencilStateDesc.EnableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc = D3D11CompareFunctionMap[static_cast<uint32_t>(depthStencilStateDesc.DepthTestFunction)];
        depthStencilDesc.StencilEnable = depthStencilStateDesc.UsingStencilTest;
        depthStencilDesc.StencilReadMask = depthStencilStateDesc.StencilReadMask;
        depthStencilDesc.StencilWriteMask = depthStencilStateDesc.StencilWriteMask;
        StencilDescMap(depthStencilStateDesc.StencilFrontFace, depthStencilDesc.FrontFace);
        StencilDescMap(depthStencilStateDesc.StencilBackFace, depthStencilDesc.BackFace);

        Shader* vertexShader    = createInfo.ShaderModuleDesc.StageShaders[static_cast<uint32_t>(GraphicPipelineState::EShaderStage::Vertex)];
        Shader* pixelShader     = createInfo.ShaderModuleDesc.StageShaders[static_cast<uint32_t>(GraphicPipelineState::EShaderStage::Pixel)];
        Shader* geometryShader  = createInfo.ShaderModuleDesc.StageShaders[static_cast<uint32_t>(GraphicPipelineState::EShaderStage::Geometry)];
        Shader* domainShader    = createInfo.ShaderModuleDesc.StageShaders[static_cast<uint32_t>(GraphicPipelineState::EShaderStage::Domain)];
        Shader* hullShader      = createInfo.ShaderModuleDesc.StageShaders[static_cast<uint32_t>(GraphicPipelineState::EShaderStage::Hull)];
        ComPtr<ID3D11InputLayout> D3D11InputLayout = nullptr;
        HRESULT retCreateInputLayout = S_OK;
        if (vertexShader != nullptr && createInfo.VertexInputLayout.AttributeArray> 0)
        {
            auto FindVertexBinding = [&LayoutDesc = createInfo.VertexInputLayout](uint32_t bindingIndex)
                -> GraphicPipelineState::VertexInputLayout::Binding*
            {
                for (uint32_t index = 0; index < LayoutDesc.NumBindingArray; index++)
                {
                    if (LayoutDesc.BindingArray[index].Index == bindingIndex)
                        return LayoutDesc.BindingArray + index;
                }
                return nullptr;
            };

            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements(createInfo.VertexInputLayout.NumAttributeArray);
            for (uint32_t index = 0; index < createInfo.VertexInputLayout.NumAttributeArray; index++)
            {
                const GraphicPipelineState::VertexInputLayout::Attribute& attribute = createInfo.VertexInputLayout.AttributeArray[index];
                D3D11_INPUT_ELEMENT_DESC& d3d11Element = inputElements[index];

                GraphicPipelineState::VertexInputLayout::Binding* binding = FindVertexBinding(attribute.BingdingIndex);
                if (binding == nullptr)
                {
                    retCreateInputLayout = E_INVALIDARG;
                    break;
                }

                d3d11Element.InputSlotClass = D3D11InstanceRateMap[static_cast<uint32_t>(binding->InputRate)];
                d3d11Element.Format = D3DVertexFormatMap[static_cast<uint32_t>(attribute.VertexFormat)];
                d3d11Element.SemanticName = attribute.SemanticName;
                d3d11Element.SemanticIndex = attribute.SemanticIndex;
                d3d11Element.InputSlot = attribute.BingdingIndex;
                d3d11Element.AlignedByteOffset = attribute.VertexOffset;
                d3d11Element.InstanceDataStepRate = (binding->InputRate == GraphicPipelineState::VertexInputLayout::EInputRate::Instance ? 1 : 0);
            }

            if (retCreateInputLayout == S_OK)
            {
                void* vsBytecode = vertexShader->GetBytecode();
                uint32_t vsBytecodeLength = vertexShader->GetBytecodeLength();
                retCreateInputLayout = mD3D11Device->CreateInputLayout(inputElements.data(), static_cast<UINT>(inputElements.size())
                    , vsBytecode, vsBytecodeLength, &D3D11InputLayout);
            }
        }

        ComPtr<ID3D11DepthStencilState> D3D11DepthStencilState = nullptr;
        ComPtr<ID3D11BlendState> D3D11ColorBlendState = nullptr;
        ComPtr<ID3D11RasterizerState> D3D11RasterizerState = nullptr;

        HRESULT retCreateDepthStencilState  = mD3D11Device->CreateDepthStencilState(&depthStencilDesc, &D3D11DepthStencilState);
        HRESULT retCreateColorBlendState    = mD3D11Device->CreateBlendState(&colorBlendDesc, &D3D11ColorBlendState);
        HRESULT retCreateRasterizerState    = mD3D11Device->CreateRasterizerState(&rasterizerDesc, &D3D11RasterizerState);

        if (SUCCEEDED(retCreateDepthStencilState)
            && SUCCEEDED(retCreateColorBlendState)
            && SUCCEEDED(retCreateRasterizerState)
            && SUCCEEDED(retCreateInputLayout))
        {
            ComPtr<ID3D11VertexShader>      D3D11VertexShader   = GetShaderPtr<ID3D11VertexShader>(vertexShader);
            ComPtr<ID3D11PixelShader>       D3D11PixelShader    = GetShaderPtr<ID3D11PixelShader>(pixelShader);
            ComPtr<ID3D11GeometryShader>    D3D11GeometryShader = GetShaderPtr<ID3D11GeometryShader>(geometryShader);
            ComPtr<ID3D11DomainShader>      D3D11DomainShader   = GetShaderPtr<ID3D11DomainShader>(domainShader);
            ComPtr<ID3D11HullShader>        D3D11HullShader     = GetShaderPtr<ID3D11HullShader>(hullShader);

            D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11PrimitiveTopologyMap[static_cast<uint32_t>(createInfo.PrimitiveTopology)];
            return new GraphicPipelineStateD3D11(
                primitiveTopology, D3D11InputLayout.Detach(),
                depthStencilDesc, D3D11DepthStencilState.Detach(),
                colorBlendDesc, D3D11ColorBlendState.Detach(), createInfo.ColorBlendState.BlendConstant,
                rasterizerDesc, D3D11RasterizerState.Detach(),
                D3D11VertexShader,
                D3D11PixelShader,
                D3D11GeometryShader,
                D3D11DomainShader,
                D3D11HullShader);
        }
        return nullptr;
    }

    D3D11_TEXTURE_ADDRESS_MODE D3DTextureAddressModeMap[] =
    {
        D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_CLAMP,
        D3D11_TEXTURE_ADDRESS_MIRROR,
        D3D11_TEXTURE_ADDRESS_BORDER,
        D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
    };
    D3D11_FILTER D3D11SamplingFilterMap(const SamplerState::ESamplingFilter samplingFilter)
    {
        switch (samplingFilter)
        {
        case SamplerState::ESamplingFilter::MinPoint_MagPoint_MipPoint:    return D3D11_FILTER_MIN_MAG_MIP_POINT;
        case SamplerState::ESamplingFilter::MinPoint_MagPoint_MipLinear:   return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case SamplerState::ESamplingFilter::MinPoint_MagLinear_MipPoint:   return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case SamplerState::ESamplingFilter::MinPoint_MagLinear_MipLinear:  return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        case SamplerState::ESamplingFilter::MinLinear_MagPoint_MipPoint:   return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case SamplerState::ESamplingFilter::MinLinear_MagPoint_MipLinear:  return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case SamplerState::ESamplingFilter::MinLinear_MagLinear_MipPoint:  return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        default:
        case SamplerState::ESamplingFilter::MinLinear_MagLinear_MipLinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        case SamplerState::ESamplingFilter::Anisotropic:                   return D3D11_FILTER_ANISOTROPIC;
        }
    }
    SamplerState* GraphicDeviceD3D11::CreateSamplerState(const SamplerState::CreateInfo& createInfo)
    {
        D3D11_SAMPLER_DESC samplerDesc;
        samplerDesc.Filter = D3D11SamplingFilterMap(createInfo.SamplingFilter);
        samplerDesc.AddressU = D3DTextureAddressModeMap[static_cast<uint32_t>(createInfo.SamplingAddressU)];
        samplerDesc.AddressV = D3DTextureAddressModeMap[static_cast<uint32_t>(createInfo.SamplingAddressV)];
        samplerDesc.AddressW = D3DTextureAddressModeMap[static_cast<uint32_t>(createInfo.SamplingAddressW)];
        samplerDesc.MipLODBias = createInfo.MipLODBias;
        samplerDesc.MaxAnisotropy = createInfo.MaxAnisotropy;
        samplerDesc.ComparisonFunc = D3D11CompareFunctionMap[static_cast<uint32_t>(createInfo.ComparisonFunction)];
        samplerDesc.MinLOD = createInfo.MinLOD;
        samplerDesc.MaxLOD = createInfo.MaxLOD;
        samplerDesc.BorderColor[0] = createInfo.BorderColor[0];
        samplerDesc.BorderColor[1] = createInfo.BorderColor[1];
        samplerDesc.BorderColor[2] = createInfo.BorderColor[2];
        samplerDesc.BorderColor[3] = createInfo.BorderColor[3];

        ComPtr<ID3D11SamplerState> samplerState = nullptr;
        if (SUCCEEDED(mD3D11Device->CreateSamplerState(&samplerDesc, &samplerState)))
        {
            return new SamplerStateD3D11(samplerDesc, samplerState.Detach());
        }
        return nullptr;
    }

    ShaderBinary* GraphicDeviceD3D11::CompileShader(const ShaderBinary::CreateInfo& createInfo)
    {
        const char* profileTarget = nullptr;
        switch (createInfo.ShaderType)
        {
        default:
        case EShaderType::VertexShader:
            profileTarget = "vs_5_0";
            break;
        case EShaderType::PixelShader:
            profileTarget = "ps_5_0";
            break;
        case EShaderType::GeometryShader:
            profileTarget = "gs_5_0";
            break;
        case EShaderType::DomainShader:
            profileTarget = "ds_5_0";
            break;
        case EShaderType::HullShader:
            profileTarget = "hs_5_0";
            break;
        case EShaderType::ComputeShader:
            profileTarget = "cs_5_0";
            break;
        }

        ComPtr<ID3DBlob> shaderBlobPtr = nullptr;
        ComPtr<ID3DBlob> errorBlobPtr = nullptr;

        HRESULT retCompileShader = ::D3DCompile(
            createInfo.ShaderSourceCodeData,
            createInfo.ShaderSourceCodeLength,
            createInfo.ShaderNameString,
            NULL,    //Defines
            NULL,    //Inlcudes
            createInfo.EntryNameString,
            profileTarget,
            0,        //Flag1
            0,        //Flag2
            &shaderBlobPtr,
            &errorBlobPtr);

        if (SUCCEEDED(retCompileShader))
        {
            
            return new ShaderBinaryD3D11(createInfo.ShaderType, shaderBlobPtr.Detach(), createInfo.ShaderNameString, createInfo.EntryNameString);
        }
        else
        {
            const char* reasonPtr = (const char*)errorBlobPtr->GetBufferPointer();
            std::string reasonString = reasonPtr;
            return nullptr;
        }
    }

    Shader* GraphicDeviceD3D11::CreateShader(const Shader::CreateInfo& createInfo)
    {
        EShaderType shaderType = createInfo.ShaderBinary->GetShaderType();
        ComPtr<ID3DBlob> shaderBinaryBlob = dynamic_cast<ShaderBinaryD3D11*>(createInfo.ShaderBinary)->GetShaderBinaryBlob();
        ID3D11ClassLinkage* classLinkage = nullptr;
        switch (shaderType)
        {
        case EShaderType::VertexShader:
        {
            ComPtr<ID3D11VertexShader> shader;
            HRESULT retCreateShader = mD3D11Device->CreateVertexShader(createInfo.ShaderBinary->GetBytecode(), createInfo.ShaderBinary->GetBytecodeLength(), classLinkage, &shader);
            if (SUCCEEDED(retCreateShader))
            {
                return NewTShaderInstance(shaderType, shaderBinaryBlob, shader.Detach(), createInfo.ShaderBinary->GetShaderName(), createInfo.ShaderBinary->GetEntryPointName());
            }
            break;
        }
        case EShaderType::PixelShader:
        {
            ComPtr<ID3D11PixelShader> shader;
            HRESULT retCreateShader = mD3D11Device->CreatePixelShader(createInfo.ShaderBinary->GetBytecode(), createInfo.ShaderBinary->GetBytecodeLength(), classLinkage, &shader);
            if (SUCCEEDED(retCreateShader))
            {
                return NewTShaderInstance(shaderType, shaderBinaryBlob, shader.Detach(), createInfo.ShaderBinary->GetShaderName(), createInfo.ShaderBinary->GetEntryPointName());
            }
            break;
        }
        case EShaderType::DomainShader:
        {
            ComPtr<ID3D11DomainShader> shader;
            HRESULT retCreateShader = mD3D11Device->CreateDomainShader(createInfo.ShaderBinary->GetBytecode(), createInfo.ShaderBinary->GetBytecodeLength(), classLinkage, &shader);
            if (SUCCEEDED(retCreateShader))
            {
                return NewTShaderInstance(shaderType, shaderBinaryBlob, shader.Detach(), createInfo.ShaderBinary->GetShaderName(), createInfo.ShaderBinary->GetEntryPointName());
            }
            break;
        }
        case EShaderType::HullShader:
        {
            ComPtr<ID3D11HullShader> shader;
            HRESULT retCreateShader = mD3D11Device->CreateHullShader(createInfo.ShaderBinary->GetBytecode(), createInfo.ShaderBinary->GetBytecodeLength(), classLinkage, &shader);
            if (SUCCEEDED(retCreateShader))
            {
                return NewTShaderInstance(shaderType, shaderBinaryBlob, shader.Detach(), createInfo.ShaderBinary->GetShaderName(), createInfo.ShaderBinary->GetEntryPointName());
            }
            break;
        }
        case EShaderType::ComputeShader:
        {
            ComPtr<ID3D11ComputeShader> shader;
            HRESULT retCreateShader = mD3D11Device->CreateComputeShader(createInfo.ShaderBinary->GetBytecode(), createInfo.ShaderBinary->GetBytecodeLength(), classLinkage, &shader);
            if (SUCCEEDED(retCreateShader))
            {
                return NewTShaderInstance(shaderType, shaderBinaryBlob, shader.Detach(), createInfo.ShaderBinary->GetShaderName(), createInfo.ShaderBinary->GetEntryPointName());
            }
            break;
        }
        }
        return nullptr;
    }

    const D3D11_USAGE D3D11UsageMap[] = {
        D3D11_USAGE_DEFAULT,
        D3D11_USAGE_IMMUTABLE,
        D3D11_USAGE_DYNAMIC,
        D3D11_USAGE_STAGING
    };

    template<D3D11_BIND_FLAG BindFlag, typename CreateInfoType>
    ComPtr<ID3D11Buffer> CreateD3D11Buffer(ID3D11Device* D3D11Device, const CreateInfoType& createInfo)
    {
        if (createInfo.DataUsage == EDataUsage::Immutable && createInfo.InitializedBufferDataPtr == nullptr)
        {
            return nullptr;
        }
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = createInfo.BufferSize;
        bufferDesc.Usage = D3D11UsageMap[static_cast<uint32_t>(createInfo.DataUsage)];
        bufferDesc.BindFlags = BindFlag;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0; //https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag
        bufferDesc.StructureByteStride = 0;

        if (createInfo.DataUsage == EDataUsage::Dynamic)
        {
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }
        else if (createInfo.DataUsage == EDataUsage::Staging)
        {
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        }

        ComPtr<ID3D11Buffer> bufferPtr = nullptr;

        if (createInfo.InitializedBufferDataPtr == nullptr)
        {
            HRESULT retCreateBuffer = D3D11Device->CreateBuffer(&bufferDesc, nullptr, &bufferPtr);
            return bufferPtr;
        }
        else
        {
            D3D11_SUBRESOURCE_DATA subResourceData;
            subResourceData.pSysMem = createInfo.InitializedBufferDataPtr;
            subResourceData.SysMemPitch = 0;      // System-memory pitch is used only for 2D and 3D texture data as it is has no meaning for the other resource types. 
            subResourceData.SysMemSlicePitch = 0; // System-memory-slice pitch is only used for 3D texture data as it has no meaning for the other resource types.

            HRESULT retCreateBuffer = D3D11Device->CreateBuffer(&bufferDesc, &subResourceData, &bufferPtr);
            return bufferPtr;
        }
        return bufferPtr;
    }

    VertexBuffer* GraphicDeviceD3D11::CreateVertexBuffer(const VertexBuffer::CreateInfo& createInfo)
    {
        ComPtr<ID3D11Buffer> vertexBuffer = CreateD3D11Buffer<D3D11_BIND_VERTEX_BUFFER>(mD3D11Device.Get(), createInfo);
        if (vertexBuffer)
        {
            return new VertexBufferD3D11(createInfo.DataUsage, createInfo.BufferSize, vertexBuffer.Detach());
        }
        return nullptr;
    }

    IndexBuffer* GraphicDeviceD3D11::CreateIndexBuffer(const IndexBuffer::CreateInfo& createInfo)
    {
        ComPtr<ID3D11Buffer> indexBuffer = CreateD3D11Buffer<D3D11_BIND_INDEX_BUFFER>(mD3D11Device.Get(), createInfo);
        if (indexBuffer)
        {
            return new IndexBufferD3D11(createInfo.DataFormat, createInfo.DataUsage, createInfo.BufferSize, indexBuffer.Detach());
        }
        return nullptr;
    }

    UniformBuffer* GraphicDeviceD3D11::CreateUniformbuffer(const UniformBuffer::CreateInfo& createInfo)
    {
        //If the bind flag is D3D11_BIND_CONSTANT_BUFFER, you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT
        UniformBuffer::CreateInfo adjustedInfo = createInfo;
        if (adjustedInfo.BufferSize > D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT)
        {
            adjustedInfo.BufferSize = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;
        }
        else if (adjustedInfo.BufferSize % 16 != 0)
        {
            adjustedInfo.BufferSize = ((adjustedInfo.BufferSize + 15) / 16) * 16;
        }
        ComPtr<ID3D11Buffer> uniformBuffer = CreateD3D11Buffer<D3D11_BIND_CONSTANT_BUFFER>(mD3D11Device.Get(), adjustedInfo);
        if (uniformBuffer)
        {
            return new UniformBufferD3D11(createInfo.DataUsage, createInfo.BufferSize, uniformBuffer.Detach());
        }
        return nullptr;
    }

    struct TextureFormatDesc
    {
        DXGI_FORMAT Texture2D;
        DXGI_FORMAT View;
        DXGI_FORMAT ShaderResource;
    };

    TextureFormatDesc D3D11RenderTargetFormatMapFunc(RenderTargetView::EFormat format, bool usedByShader)
    {
        switch (format)
        {
        default:
        case RenderTargetView::EFormat::R10G10B10A2_UNormInt:
            return { DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM };
            break;
        case RenderTargetView::EFormat::R8G8B8A8_UNormInt:
            return { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };
            break;
        }
    };

    RenderTargetView* GraphicDeviceD3D11::CreateRenderTargetView(const RenderTargetView::CreateInfo& createInfo)
    {
        TextureFormatDesc D3D11RTFormat = D3D11RenderTargetFormatMapFunc(createInfo.Format, createInfo.UsedByShader);

        D3D11_TEXTURE2D_DESC texture2DDesc;
        texture2DDesc.Width = createInfo.Width;
        texture2DDesc.Height = createInfo.Height;
        texture2DDesc.MipLevels = 1;
        texture2DDesc.ArraySize = 1;
        texture2DDesc.Format = D3D11RTFormat.Texture2D;
        texture2DDesc.SampleDesc.Count = 1;
        texture2DDesc.SampleDesc.Quality = 0;
        texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
        texture2DDesc.BindFlags = createInfo.UsedByShader ? (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE) : D3D11_BIND_RENDER_TARGET;
        texture2DDesc.CPUAccessFlags = 0;
        texture2DDesc.MiscFlags = 0;

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        renderTargetViewDesc.Format = D3D11RTFormat.View;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;

        //TODO: more general way.
        const int kMipLevels = -1; //Set to -1 to indicate all the mipmap levels from MostDetailedMip on down to least detailed.
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        shaderResourceViewDesc.Format = D3D11RTFormat.ShaderResource;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = kMipLevels;

        ComPtr<ID3D11Texture2D> rtTexture2D;
        ComPtr<ID3D11RenderTargetView> rtView;
        ComPtr<ID3D11ShaderResourceView> rtShaderResourceView;
        HRESULT retCreateTexture = mD3D11Device->CreateTexture2D(&texture2DDesc, nullptr, &rtTexture2D);
        if (FAILED(retCreateTexture))
        {
            return nullptr;
        }

        HRESULT retCreateDSView = mD3D11Device->CreateRenderTargetView(rtTexture2D.Get(), &renderTargetViewDesc, &rtView);
        if (FAILED(retCreateDSView))
        {
            return nullptr;
        }

        if (createInfo.UsedByShader)
        {
            HRESULT retCreateSRView = mD3D11Device->CreateShaderResourceView(rtTexture2D.Get(), &shaderResourceViewDesc, &rtShaderResourceView);
            if (FAILED(retCreateSRView))
            {
                return nullptr;
            }
        }
        return new RenderTargetD3D11(createInfo.Format, createInfo.Width, createInfo.Height, createInfo.UsedByShader, rtTexture2D.Detach(), rtView.Detach(), rtShaderResourceView.Detach());

        return nullptr;
    }

    TextureFormatDesc D3D11DepthStencilFormatMapFunc(DepthStencilView::EFormat format, bool usedByShader)
    {
        switch (format)
        {
        default:
        case DepthStencilView::EFormat::D24_UNormInt_S8_UInt:
            if (!usedByShader)
            {
                return { DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS };
            }
            else
            {
                return { DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS };
            }
            break;
        case DepthStencilView::EFormat::D32_SFloat:
            return { DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT };
            break;
        }
    };

    DepthStencilView* GraphicDeviceD3D11::CreateDepthStencilView(const DepthStencilView::CreateInfo& createInfo)
    {
        //std::vector<D3D11_SUBRESOURCE_DATA> Data;
        //if (InSubResourceData != nullptr)
        //{
        //    uint32_t DataSize = InDesc->ArraySize * std::max(1u, InDesc->MipLevels);
        //    Data.resize(DataSize);
        //    for (uint32_t Slice = 0; Slice < DataSize; ++Slice)
        //        Data[Slice] = InSubResourceData[Slice];
        //}
        //https://docs.microsoft.com/en-us/windows/uwp/gaming/create-depth-buffer-resource--view--and-sampler-state

        TextureFormatDesc D3D11DSFormat = D3D11DepthStencilFormatMapFunc(createInfo.Format, createInfo.UsedByShader);

        D3D11_TEXTURE2D_DESC texture2DDesc;
        texture2DDesc.Width     = createInfo.Width;
        texture2DDesc.Height    = createInfo.Height;
        texture2DDesc.MipLevels = 1;
        texture2DDesc.ArraySize = 1;
        texture2DDesc.Format    = D3D11DSFormat.Texture2D;
        texture2DDesc.SampleDesc.Count = 1;
        texture2DDesc.SampleDesc.Quality = 0;
        texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
        texture2DDesc.BindFlags = createInfo.UsedByShader ? (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE) : D3D11_BIND_DEPTH_STENCIL;
        texture2DDesc.CPUAccessFlags = 0;
        texture2DDesc.MiscFlags      = 0;

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        depthStencilViewDesc.Format = D3D11DSFormat.View;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Flags = 0;
        depthStencilViewDesc.Texture2D.MipSlice = 0;

        //TODO: more general way.
        const int kMipLevels = 1; //Set to -1 to indicate all the mipmap levels from MostDetailedMip on down to least detailed.
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        shaderResourceViewDesc.Format = D3D11DSFormat.ShaderResource;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = kMipLevels;



        ComPtr<ID3D11Texture2D> dsTexture2D;
        ComPtr<ID3D11DepthStencilView> depthStencilView;
        ComPtr<ID3D11ShaderResourceView> dsShaderResourceView;
        HRESULT retCreateTexture = mD3D11Device->CreateTexture2D(&texture2DDesc, nullptr, &dsTexture2D);
        if (FAILED(retCreateTexture))
        {
            return nullptr;
        }

        HRESULT retCreateDSView = mD3D11Device->CreateDepthStencilView(dsTexture2D.Get(), &depthStencilViewDesc, &depthStencilView);
        if (FAILED(retCreateDSView))
        {
            return nullptr;
        }

        if (createInfo.UsedByShader)
        {
            HRESULT retCreateSRView = mD3D11Device->CreateShaderResourceView(dsTexture2D.Get(), &shaderResourceViewDesc, &dsShaderResourceView);
            if (FAILED(retCreateSRView))
            {
                return nullptr;
            }
        }
        return new DepthStencilD3D11(createInfo.Format, createInfo.Width, createInfo.Height, createInfo.UsedByShader, dsTexture2D.Detach(), depthStencilView.Detach(), dsShaderResourceView.Detach());
        return nullptr;
    }

    void GraphicDeviceD3D11::Release()
    {
        delete this;
    }
}
