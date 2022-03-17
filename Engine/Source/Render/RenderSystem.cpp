#include "RenderSystem.h"
#include <Foundation/Base/ScopeHelper.h>
#include <Foundation/Base/MemoryHelper.h>
#include <vector>
#include <string>
#include <d3dcompiler.h>
#include "Core/GameEngine.h"
#include "Scene/Components.h"
#include "Scene/Scene.h"
#include "Render/FrameGraph.h"

using Microsoft::WRL::ComPtr;

Microsoft::WRL::ComPtr<IDXGIFactory2> GetDXGIAdapterFromDevice(Microsoft::WRL::ComPtr<ID3D11Device> pDevice)
{
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIDevice2> DXGIDevice2;
    HRESULT resultGetDXGIDevice2 = pDevice.As(&DXGIDevice2);
    if (SUCCEEDED(resultGetDXGIDevice2))
    {
        ComPtr<IDXGIAdapter1> DXGIDevice1;
        HRESULT resultGetDXGIDevice1 = DXGIDevice2->GetParent(IID_PPV_ARGS(&DXGIDevice1));
        if (SUCCEEDED(resultGetDXGIDevice1))
        {
            ComPtr<IDXGIFactory2> mDXGIFactory;
            if (SUCCEEDED(DXGIDevice1->GetParent(IID_PPV_ARGS(&mDXGIFactory))))
            {
                return mDXGIFactory;
            }
        }
    }
    return nullptr;
}


const unsigned int InputLayoutCount = 3;
D3D11_INPUT_ELEMENT_DESC InputLayout[InputLayoutCount]
{
    D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

namespace context_private_impl
{
    ComPtr<ID3DBlob> CompileShaderSource(
        const std::string& sourceCodes,
        const std::string& entryPoint,
        const std::string profileTarget)
    {
        ComPtr<ID3DBlob> shaderBlobPtr = nullptr;
        ComPtr<ID3DBlob> errorBlobPtr = nullptr;

        HRESULT resultCompile = ::D3DCompile(sourceCodes.c_str(), sourceCodes.size(),
            NULL,    //Name
            NULL,    //Defines
            NULL,    //Inlcudes
            entryPoint.c_str(),
            profileTarget.c_str(),
            0,        //Flag1
            0,        //Flag2
            &shaderBlobPtr,
            &errorBlobPtr);

        if (SUCCEEDED(resultCompile))
        {
            return shaderBlobPtr;
        }
        else
        {
            const char* reasonPtr = (const char*)errorBlobPtr->GetBufferPointer();
            std::string reasonString = reasonPtr;
            return nullptr;
        }
    }
}

struct VertexShader
{
    ComPtr<ID3D11VertexShader> mVertexShader = nullptr;
    ComPtr<ID3D11InputLayout> mInputLayout = nullptr;
    std::vector<D3D11_INPUT_ELEMENT_DESC> mInputLayoutDesc;
};

struct PixelShader
{
    ComPtr<ID3D11PixelShader> mPixelShader = nullptr;
};

struct ShaderProgram
{
    ~ShaderProgram()
    {
        safe_delete(VertexShader);
        safe_delete(PixelShader);
    }
    VertexShader* VertexShader = nullptr;
    PixelShader* PixelShader = nullptr;
};


VertexShader* CreateVertexShader(ID3D11Device* GfxDevice,
    const std::string& source, const std::string& entry,
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout)
{
    using context_private_impl::CompileShaderSource;
    const std::string profile = "vs_5_0";
    ComPtr<ID3DBlob> vsBlob = CompileShaderSource(source, entry, profile);

    if (vsBlob == nullptr)
    {
        ASSERT(vsBlob != nullptr);
        return nullptr;
    }

    ComPtr<ID3D11VertexShader> vertexShader = nullptr;
    ComPtr<ID3D11InputLayout> inputLayout = nullptr;

    HRESULT rstCreateVertexShader = GfxDevice->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        NULL,
        &vertexShader);

    if (FAILED(rstCreateVertexShader))
    {
        ASSERT_SUCCEEDED(rstCreateVertexShader);
        return nullptr;
    }

    HRESULT rstCreateInputLayout = GfxDevice->CreateInputLayout(
        &(layout[0]), (UINT)layout.size(),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &inputLayout);

    if (FAILED(rstCreateInputLayout))
    {
        ASSERT_SUCCEEDED(rstCreateInputLayout);
        return nullptr;
    }

    VertexShader* shader = new VertexShader();
    shader->mVertexShader = vertexShader;
    shader->mInputLayout = inputLayout;
    shader->mInputLayoutDesc = layout;
    return shader;
}

PixelShader* CreatePixelShader(ID3D11Device* GfxDevice,
    const std::string& source, const std::string& entry)
{
    using context_private_impl::CompileShaderSource;
    const std::string profile = "ps_5_0";
    ComPtr<ID3DBlob> psBlob = CompileShaderSource(source, entry, profile);

    if (psBlob == nullptr)
    {
        ASSERT(psBlob != nullptr);
        return nullptr;
    }

    ComPtr<ID3D11PixelShader> pixelShader = nullptr;
    HRESULT rstCreatePixelShader = GfxDevice->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        NULL,
        &pixelShader);

    if (FAILED(rstCreatePixelShader))
    {
        ASSERT_SUCCEEDED(rstCreatePixelShader);
        return nullptr;
    }

    PixelShader* shader = new PixelShader();
    shader->mPixelShader = pixelShader;
    return shader;
}

ShaderProgram* LinkShader(VertexShader* vertexShader, PixelShader* pixelShader)
{
    auto vertexShaderImpl = reinterpret_cast<::VertexShader*>(vertexShader);
    auto pixelShaderImpl = reinterpret_cast<::PixelShader*>(pixelShader);
    if (vertexShaderImpl == nullptr || pixelShaderImpl == nullptr)
    {
        ASSERT(vertexShaderImpl != nullptr && pixelShaderImpl != nullptr);
        return nullptr;
    }

    ShaderProgram* program = new ShaderProgram();
    program->VertexShader = vertexShaderImpl;
    program->PixelShader = pixelShaderImpl;
    return program;
}

struct ObjectConstantDesc
{
    math::float4x4 MatrixObjectToWorld;
    math::float4x4 MatrixWorldToObject;
};

struct LightViewConstantDesc
{
    math::float4x4 MatrixView;
    math::float4x4 MatrixProj;
};

const std::string DefaultVertexShaderSourceCode = R"(
    cbuffer object
    {
        float4x4 MatrixObjectToWorld;
        float4x4 MatrixWorldToObject;
    }
    cbuffer scene
    {
        float4x4 MatrixView;
        float4x4 InvMatrixView;
        float4x4 MatrixProj;
        float3   LightColor;
        float    Padding0;
        float3   LightDirection;
        float    Padding1;
    }
    cbuffer lightView
    {
        float4x4 LightMatrixView;
        float4x4 LightMatrixProj;
    }
    struct VertexLayout
    {
        float4 Position  : POSITION;
        float3 Normal    : NORMAL;
        float2 Texcoord0 : TEXCOORD0;
    };
    struct VertexOutput
    {
        float4 Position  : SV_POSITION;
        float3 Normal    : TEXCOORD0;
        float3 ViewDirWS : TEXCOORD1;
        float4 LightPos  : TEXCOORD2;
    };
    VertexOutput VSMain(VertexLayout input)
    {
        VertexOutput output;
        float4 ModelPosition = mul(input.Position, MatrixObjectToWorld);
        float4 ViewPosition  = mul(ModelPosition, MatrixView);
        output.Position      = mul(ViewPosition, MatrixProj);
        float4 ModelNormal   = float4(input.Normal, 0.0f);
        output.Normal        = normalize(mul(ModelNormal, transpose(MatrixWorldToObject)).xyz);
        float3 CameraPositionWS = InvMatrixView._m03_m13_m23;
        output.ViewDirWS     = CameraPositionWS - ModelPosition.xyz;
        output.LightPos      = mul(mul(ModelPosition, LightMatrixView), LightMatrixProj);
        return output;
    }
)";

const std::string SimpleColorPixelShaderSourceCode = R"(
    Texture2D Texture : register(t0);
    sampler Sampler : register(s0);
    cbuffer scene
    {
        float4x4 MatrixView;
        float4x4 InvMatrixView;
        float4x4 MatrixProj;
        float3   LightColor;
        float    Padding0;
        float3   LightDirection;
        float    Padding1;
    }
    struct VertexOutput
    {
        float4 Position  : SV_POSITION;
        float3 Normal    : TEXCOORD0;
        float3 ViewDirWS : TEXCOORD1;
        float4 LightPos  : TEXCOORD2;
    };
    float4 PSMain(VertexOutput input) : SV_TARGET
    {
        float3 projection = input.LightPos.xyz / input.LightPos.w;
        projection.y = -projection.y;
        float2 shadowDepthTexcoord = 0.5 * projection.xy + 0.5;
        float depth = projection.z;
        float shadowDepth = Texture.Sample(Sampler, shadowDepthTexcoord).r;
        float shadow = shadowDepth + 0.0000025 >= depth ;
        //return float4(float3(1,1,1) * shadow, 1); 
        float3 N = normalize(input.Normal);
        float3 L = -LightDirection;
        float3 V = normalize(input.ViewDirWS);
        float3 H = normalize(L + V);
        float NoL = max(0.0, dot(L, N));
        float NoH = max(0.0, dot(N, H));
        float3 Ambient = float3(0.15, 0.1, 0.1);
        float3 Diffuse = float3(1.0, 1.0, 1.0) * NoL;
        float3 Specular = pow(NoH, 128);
        float4 Color = float4((Diffuse + Specular) * LightColor * shadow + Ambient, 1.0);
        return Color;
    }
)";

const std::string BlitVertexShaderSource = R"(
    struct VertexLayout
    {
        float4 Position : POSITION;
        float3 Normal   : NORMAL;
        float2 Texcoord : TEXCOORD0;
    };
    struct VertexOutput
    {
        float4 Position : SV_POSITION;
        float2 Texcoord : TEXCOORD0;
    };
    VertexOutput VSMain(VertexLayout input)
    {
        VertexOutput output;
        output.Position = input.Position;
        output.Texcoord = input.Texcoord;
        return output;
    }
)";

const std::string BlitPixelShaderSource = R"(
    Texture2D Texture : register(t0);
    sampler Sampler : register(s0);
    struct VertexOutput
    {
        float4 Position : SV_POSITION;
        float2 Texcoord : TEXCOORD0;
    };
    float4 PSMain(VertexOutput input) : SV_TARGET
    {
        float4 color = Texture.Sample(Sampler, input.Texcoord);
        return float4(color.rgb, 1.0);
    }
)";


const std::string ShadowVertexShaderSource = R"(
    cbuffer object
    {
        float4x4 MatrixObjectToWorld;
        float4x4 MatrixWorldToObject;
    }
    cbuffer lightView
    {
        float4x4 LightMatrixView;
        float4x4 LightMatrixProj;
    }
    struct VertexLayout
    {
        float4 Position  : POSITION;
        float3 Normal    : NORMAL;
        float2 Texcoord0 : TEXCOORD0;
    };
    struct VertexOutput
    {
        float4 Position  : SV_POSITION;
        float2 DepthPos  : TEXCOORD0;
    };
    VertexOutput VSMain(VertexLayout input)
    {
        VertexOutput output;
        float4 ModelPosition = mul(input.Position, MatrixObjectToWorld);
        float4 ViewPosition  = mul(ModelPosition, LightMatrixView);
        float4 HomoPosition  = mul(ViewPosition, LightMatrixProj);
        output.Position      = HomoPosition;
        output.DepthPos      = HomoPosition.zw;
        return output;
    }
)";

const std::string ShadowPixelShaderSource = R"(
    struct VertexOutput
    {
        float4 Position : SV_POSITION;
        float2 DepthPos  : TEXCOORD0;
    };
    float4 PSMain(VertexOutput input) : SV_TARGET
    {
        return float4(0.0, 0.0, 0.0, 1.0);
    }
)";


namespace fullscreen_quad
{
    const unsigned int FSQuadVertexCount = 3;
    engine::VertexLayout FSQuadVertices[FSQuadVertexCount] =
    {
        { math::float4(-1.0f, -1.0f, 0.0f, 1), math::normalized_float3::unit_z_neg(),  math::float2(0, 1) },
        { math::float4(+3.0f, -1.0f, 0.0f, 1), math::normalized_float3::unit_z_neg(),  math::float2(2, 1) },
        { math::float4(-1.0f, +3.0f, 0.0f, 1), math::normalized_float3::unit_z_neg(),  math::float2(0, -1) }
    };
}

namespace engine
{
    GfxDefaultVertexBuffer* FullscreenQuadVertexBuffer = nullptr;
    GfxDynamicConstantBuffer* ObjectConstantBufferPtr = nullptr;
    GfxDynamicConstantBuffer* SceneConstantBufferPtr = nullptr;
    GfxDynamicConstantBuffer* LightViewConstantBufferPtr = nullptr;
    ShaderProgram* ShadowShaderProgramPtr = nullptr;
    ShaderProgram* SimpleShaderProgramPtr = nullptr;
    ShaderProgram* BlitShaderProgramPtr = nullptr;
    ID3D11CommandList* CubeRenderingCommandList = nullptr;
    ID3D11CommandList* BlitRenderingCommandList = nullptr;
    ID3D11RasterizerState* DefaultRasterState = nullptr;
    ID3D11SamplerState* DefaultSamplerState = nullptr;

    RenderSystem::RenderSystem(void* hWindow, bool fullscreen, int width, int height)
        : mMainWindowHandle((HWND)hWindow)
        , mIsFullScreen(fullscreen)
        , mWindowWidth(width)
        , mWindowHeight(height)
    {
        RECT clientRect;
        ::GetClientRect(mMainWindowHandle, &clientRect);
        mClientWidth = clientRect.right - clientRect.left;
        mClientHeight = clientRect.bottom - clientRect.top;
    }


    template<typename T>
    void safe_release(std::unique_ptr<T>& ptr) { ptr.release(); }
    RenderSystem::~RenderSystem()
    {
        safe_delete(FullscreenQuadVertexBuffer);
        safe_delete(ObjectConstantBufferPtr);
        safe_delete(SceneConstantBufferPtr);
        safe_delete(LightViewConstantBufferPtr);
        safe_delete(ShadowShaderProgramPtr);
        safe_delete(SimpleShaderProgramPtr);
        safe_delete(BlitShaderProgramPtr);
        SafeRelease(DefaultRasterState);
        SafeRelease(DefaultSamplerState);
        SafeRelease(CubeRenderingCommandList);
        SafeRelease(BlitRenderingCommandList);

        safe_release(mBackbufferDepthStencil);
        safe_release(mGfxDeviceDeferredContext);
        safe_release(mGfxDeviceImmediateContext);
        safe_release(mGfxDevice);
    }

    EGfxIntializationError RenderSystem::InitializeGfxDevice()
    {
        //CreateVertexBuffer Device and DeviceContext
        ComPtr<ID3D11Device> outD3DDevice = nullptr;
        ComPtr<ID3D11DeviceContext> outD3DDeviceImmediateContext = nullptr;
        ComPtr<ID3D11DeviceContext> outD3DDeferredContext = nullptr;
        ComPtr<IDXGISwapChain1> outSwapChain = nullptr;
        ComPtr<ID3D11Texture2D> outBackbufferTexture = nullptr;
        ComPtr<ID3D11RenderTargetView> outBackbufferRTV = nullptr;
        ComPtr<ID3D11ShaderResourceView> outBackbufferSRV = nullptr;
        D3D_FEATURE_LEVEL outFeatureLevel;

        IDXGIAdapter* defualtAdpater = nullptr;
        HMODULE noneSoftwareModule = nullptr;
        UINT deviceCreationFlag = 0;
        const int numFeatureLevel = 2;
        D3D_FEATURE_LEVEL featureLevels[numFeatureLevel] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

        HRESULT resultCreateDevice = D3D11CreateDevice(
            defualtAdpater,
            D3D_DRIVER_TYPE_HARDWARE,
            noneSoftwareModule,
            deviceCreationFlag,
            featureLevels, numFeatureLevel,
            D3D11_SDK_VERSION,
            &outD3DDevice,
            &outFeatureLevel,
            &outD3DDeviceImmediateContext
        );

        if (FAILED(resultCreateDevice))
        {
            ASSERT_SUCCEEDED(resultCreateDevice, "D3D11CreateDevice failed.");
            return EGfxIntializationError::DeviceCreationFail;
        }

        ///* CreateVertexBuffer SwapChain */
        bool usedForshader = true;
        mBackbufferRenderTarget = std::make_unique<GfxRenderTarget>(ERenderTargetFormat::UNormRGB10A2, usedForshader);
        mBackbufferRenderTarget->mTexture2DDesc.Width = mClientWidth;
        mBackbufferRenderTarget->mTexture2DDesc.Height = mClientHeight;

        DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
        swapchainDesc.Width = mBackbufferRenderTarget->mTexture2DDesc.Width;
        swapchainDesc.Height = mBackbufferRenderTarget->mTexture2DDesc.Height;
        swapchainDesc.BufferCount = 2;
        swapchainDesc.BufferUsage = usedForshader ? (DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT) : DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.Stereo = false;
        swapchainDesc.SampleDesc.Count = mBackbufferRenderTarget->mTexture2DDesc.SampleDesc.Count;
        swapchainDesc.SampleDesc.Quality = mBackbufferRenderTarget->mTexture2DDesc.SampleDesc.Quality;
        swapchainDesc.Flags = 0;
        swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDesc.Format = mBackbufferRenderTarget->mTexture2DDesc.Format;
        swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
        fullScreenDesc.RefreshRate.Numerator = 60;
        fullScreenDesc.RefreshRate.Denominator = 1;
        fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
        fullScreenDesc.Windowed = !mIsFullScreen;

        ComPtr<IDXGIFactory2> DXGIFactory = GetDXGIAdapterFromDevice(outD3DDevice);
        if (DXGIFactory == nullptr)
        {
            ASSERT(DXGIFactory != nullptr);
            return EGfxIntializationError::RetrieveDXGIFactoryFail;
        }

        IDXGIOutput* noneDXGIOutput = nullptr;
        HRESULT resultCreateSwapchain = DXGIFactory->CreateSwapChainForHwnd(outD3DDevice.Get(), mMainWindowHandle, &swapchainDesc, &fullScreenDesc, noneDXGIOutput, &outSwapChain);

        if (FAILED(resultCreateSwapchain))
        {
            ASSERT_SUCCEEDED(resultCreateSwapchain, "CreateSwapChainForHwnd failed.");
            return EGfxIntializationError::CreateSwapchainFail;
        }

        //CreateVertexBuffer RenderTargetView
        HRESULT resultGetBackbuffer = outSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &outBackbufferTexture);
        if (FAILED(resultGetBackbuffer))
        {
            ASSERT_SUCCEEDED(resultGetBackbuffer);
            return EGfxIntializationError::RetrieveBackbufferFail;
        }

        HRESULT resultCreateBackbufferRT = outD3DDevice->CreateRenderTargetView(outBackbufferTexture.Get(), &mBackbufferRenderTarget->mRenderTargetDesc, &outBackbufferRTV);
        if (FAILED(resultCreateBackbufferRT))
        {
            ASSERT_SUCCEEDED(resultCreateBackbufferRT);
            return EGfxIntializationError::CreateBackbufferRTVFail;
        }

        HRESULT resultCreateBackbufferSRV = outD3DDevice->CreateShaderResourceView(outBackbufferTexture.Get(), &mBackbufferRenderTarget->mShaderResourceDesc, &outBackbufferSRV);
        if (FAILED(resultCreateBackbufferSRV))
        {
            ASSERT_SUCCEEDED(resultCreateBackbufferSRV);
            return EGfxIntializationError::CreateBackbufferSRVFail;
        }

        HRESULT resultCreateDeferredContext = outD3DDevice->CreateDeferredContext(0, &outD3DDeferredContext);
        if (FAILED(resultCreateDeferredContext))
        {
            ASSERT_SUCCEEDED(resultCreateDeferredContext);
            return EGfxIntializationError::CreateDeferredContextFail;
        }

        mGfxDevice = std::make_unique<GfxDevice>(outD3DDevice.Detach());
        GfxDepthStencil* outDepthStencil = mGfxDevice->CreateDepthStencil(EDepthStencilFormat::UNormDepth24_UIntStencil8, mClientWidth, mClientHeight, false);
        if (outDepthStencil == nullptr)
        {
            ASSERT(outDepthStencil != nullptr);
            return EGfxIntializationError::CreateBackbufferDSFail;
        }

        mGfxDeviceImmediateContext = std::make_unique<GfxImmediateContext>(mGfxDevice.get(), outD3DDeviceImmediateContext.Detach());
        mGfxDeviceDeferredContext = std::make_unique<GfxDeferredContext>(mGfxDevice.get(), outD3DDeferredContext.Detach());
        mGfxSwapChain = outSwapChain;
        mBackbufferDepthStencil = std::unique_ptr<GfxDepthStencil>(outDepthStencil);
        mBackbufferRenderTarget->mTexturePtr = outBackbufferTexture;
        mBackbufferRenderTarget->mRenderTargetView = outBackbufferRTV;
        mBackbufferRenderTarget->mShaderResourceView = outBackbufferSRV;
        mTransientBufferRegistry = std::make_unique<TransientBufferRegistry>(mGfxDevice.get(), mBackbufferRenderTarget.get(), mBackbufferDepthStencil.get());
        return EGfxIntializationError::NoError;
    }

    void RenderSystem::RenderFrame(Scene& scene)
    {
        ViewConstantBufferData data;
        //Update ConstantBufferData
        {
            math::float4x4 viewMatrix = math::scale_matrix4x4f(2, 2, 2);
            math::float3 cameraPositionWS = math::float3(0.0f, 0.0f, -10.0f);
            math::float3 lightColor = math::float3(1.0f, 1.0f, 1.0f);
            float lightIntensity = 1.0f;
            math::float3 lightDirection = math::float3(1.0f, 0.0f, 0.0f);


            Camera* defaultCamera = scene.GetDefaultCameraInternal();
            if (defaultCamera != nullptr)
            {
                viewMatrix = defaultCamera->GetUpdatedViewMatrix();
                cameraPositionWS = defaultCamera->GetSceneNode()->GetWorldPosition();
            }

            GE::DirectionalLight* defaultLight = scene.GetDirectionalLight(0);

            lightColor = defaultLight->GetColor();
            lightIntensity = defaultLight->GetIntensity();
            lightDirection = defaultLight->GetSceneNode()->GetForwardDirection();

            data.ViewMatrix = viewMatrix;
            data.InvViewMatrix = math::inversed(viewMatrix);
            data.CameraPositionWS = cameraPositionWS;
            data.LightColor = lightColor * lightIntensity;
            data.LightDirection = lightDirection;
        }

        //Do Rendering.
        {
            RenderFrameGraph MainGraph(mTransientBufferRegistry.get());
            RFGRenderPass forwardPass = MainGraph.AddRenderPass("test.forward");
            RFGRenderPass shadowPass = MainGraph.AddRenderPass("test.shadowdepth");
            RFGRenderPass blitPass = MainGraph.AddRenderPass("test.blit");
            RFGResourceHandle renderTarget = MainGraph.RequestResource("test.rendertarget", MainGraph.GetBackbufferWidth(), MainGraph.GetBackbufferHeight(), MainGraph.GetBackbufferRTFormat());
            RFGResourceHandle shadowDepth = MainGraph.RequestResource("test.shadowdepth", 1024, 1024, EDepthStencilFormat::UNormDepth24_UIntStencil8);
            RFGResourceHandle depthStencil = MainGraph.RequestResource("test.depthstencil", MainGraph.GetBackbufferWidth(), MainGraph.GetBackbufferHeight(), MainGraph.GetBackbufferDSFormat());
            RFGResourceHandle blitRenderTarget = MainGraph.RequestResource("test.blitrendertarget", MainGraph.GetBackbufferWidth(), MainGraph.GetBackbufferHeight(), MainGraph.GetBackbufferRTFormat());

            ClearState state;
            state.ClearDepth = true;
            state.ClearStencil = true;
            state.ClearDepthValue= 1.0;
            state.ClearStencilValue = 0;
            shadowPass.BindWriting(shadowDepth, state);
            shadowPass.AttachJob(
                [&](GfxDeferredContext& context)
                {
                    if (DefaultRasterState == nullptr)
                    {
                        D3D11_RASTERIZER_DESC RasterizerDesc;
                        RasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
                        RasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
                        RasterizerDesc.FrontCounterClockwise = TRUE;
                        RasterizerDesc.DepthBias = 0;
                        RasterizerDesc.DepthBiasClamp = 0.0f;
                        RasterizerDesc.SlopeScaledDepthBias = 0.0;
                        RasterizerDesc.DepthClipEnable = TRUE;
                        RasterizerDesc.ScissorEnable = FALSE;
                        RasterizerDesc.MultisampleEnable = FALSE;
                        RasterizerDesc.AntialiasedLineEnable = FALSE;
                        mGfxDevice->mGfxDevice->CreateRasterizerState(&RasterizerDesc, &DefaultRasterState);
                    }
                    context.mGfxDeviceContext->RSSetState(DefaultRasterState);

                    D3D11_VIEWPORT Viewport;
                    Viewport.TopLeftX = 0.0f;
                    Viewport.TopLeftY = 0.0f;
                    Viewport.Width = (float)1024;
                    Viewport.Height = (float)1024;
                    Viewport.MinDepth = 0.0f;
                    Viewport.MaxDepth = 1.0f;

                    context.mGfxDeviceContext->RSSetViewports(1, &Viewport);

                    RenderScene(scene, context, data, true);

                    const bool DontRestoreContextState = false;
                    context.mGfxDeviceContext->FinishCommandList(DontRestoreContextState, &CubeRenderingCommandList);
                    mGfxDeviceImmediateContext->mGfxDeviceContext->ExecuteCommandList(CubeRenderingCommandList, DontRestoreContextState);
                }
            );

            state.ClearColor = true;
            state.ClearDepth = true;
            state.ClearStencil = true;
            state.ClearColorValue = math::float4{ 0.15f, 0.0f, 0.0f, 1.0f };
            forwardPass.BindReading(shadowDepth);
            forwardPass.BindWriting(renderTarget, state);
            forwardPass.BindWriting(depthStencil, state);
            forwardPass.AttachJob(
                [&](GfxDeferredContext& context)
                {
                    if (DefaultSamplerState == nullptr)
                    {
                        // Create a texture sampler state description.
                        D3D11_SAMPLER_DESC samplerDesc;
                        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                        samplerDesc.MipLODBias = 0.0f;
                        samplerDesc.MaxAnisotropy = 1;
                        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
                        samplerDesc.BorderColor[0] = 0;
                        samplerDesc.BorderColor[1] = 0;
                        samplerDesc.BorderColor[2] = 0;
                        samplerDesc.BorderColor[3] = 0;
                        samplerDesc.MinLOD = 0;
                        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
                        mGfxDevice->mGfxDevice->CreateSamplerState(&samplerDesc, &DefaultSamplerState);
                    }

                    context.mGfxDeviceContext->PSSetSamplers(0, 1, &DefaultSamplerState);
                    context.mGfxDeviceContext->RSSetState(DefaultRasterState);

                    D3D11_VIEWPORT Viewport;
                    Viewport.TopLeftX = 0.0f;
                    Viewport.TopLeftY = 0.0f;
                    Viewport.Width = (float)mClientWidth;
                    Viewport.Height = (float)mClientHeight;
                    Viewport.MinDepth = 0.0f;
                    Viewport.MaxDepth = 1.0f;

                    context.mGfxDeviceContext->RSSetViewports(1, &Viewport);

                    RenderScene(scene, context, data, false);

                    const bool DontRestoreContextState = false;
                    context.mGfxDeviceContext->FinishCommandList(DontRestoreContextState, &CubeRenderingCommandList);
                    mGfxDeviceImmediateContext->mGfxDeviceContext->ExecuteCommandList(CubeRenderingCommandList, DontRestoreContextState);
                }
            );

            state.ClearColorValue.set(0.0f, 0.0f, 0.0f, 1.0f);
            blitPass.BindReading(renderTarget);
            blitPass.BindWriting(blitRenderTarget, state);
            blitPass.BindWriting(depthStencil, state);
            blitPass.AttachJob(
                [&](GfxDeferredContext& context)
                {
                    if (DefaultSamplerState == nullptr)
                    {
                        // Create a texture sampler state description.
                        D3D11_SAMPLER_DESC samplerDesc;
                        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                        samplerDesc.MipLODBias = 0.0f;
                        samplerDesc.MaxAnisotropy = 1;
                        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
                        samplerDesc.BorderColor[0] = 0;
                        samplerDesc.BorderColor[1] = 0;
                        samplerDesc.BorderColor[2] = 0;
                        samplerDesc.BorderColor[3] = 0;
                        samplerDesc.MinLOD = 0;
                        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
                        mGfxDevice->mGfxDevice->CreateSamplerState(&samplerDesc, &DefaultSamplerState);
                    }
                    context.mGfxDeviceContext->PSSetSamplers(0, 1, &DefaultSamplerState);
                    context.mGfxDeviceContext->RSSetState(DefaultRasterState);


                    if (BlitShaderProgramPtr == nullptr)
                    {
                        const std::string vsEntryName = "VSMain";
                        const std::string psEntryName = "PSMain";
                        const std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutArray(InputLayout, InputLayout + InputLayoutCount);
                        auto VertexShader = CreateVertexShader(mGfxDevice->mGfxDevice, BlitVertexShaderSource, vsEntryName, InputLayoutArray);
                        auto PixelShader = CreatePixelShader(mGfxDevice->mGfxDevice, BlitPixelShaderSource, psEntryName);
                        BlitShaderProgramPtr = LinkShader(VertexShader, PixelShader);
                        ASSERT(BlitShaderProgramPtr != nullptr);

                        FullscreenQuadVertexBuffer = mGfxDevice->CreateDefaultVertexBufferImpl(fullscreen_quad::FSQuadVertexCount);
                        mGfxDeviceImmediateContext->UploadEntireBufferFromMemory(FullscreenQuadVertexBuffer, fullscreen_quad::FSQuadVertices);
                    }

                    D3D11_VIEWPORT Viewport;
                    Viewport.TopLeftX = 0.0f;
                    Viewport.TopLeftY = 0.0f;
                    Viewport.Width = (float)mClientWidth;
                    Viewport.Height = (float)mClientHeight;
                    Viewport.MinDepth = 0.0f;
                    Viewport.MaxDepth = 1.0f;

                    context.mGfxDeviceContext->RSSetViewports(1, &Viewport);
                    context.mGfxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    context.mGfxDeviceContext->IASetInputLayout(BlitShaderProgramPtr->VertexShader->mInputLayout.Get());
                    context.mGfxDeviceContext->VSSetShader(BlitShaderProgramPtr->VertexShader->mVertexShader.Get(), nullptr, 0);
                    context.mGfxDeviceContext->PSSetShader(BlitShaderProgramPtr->PixelShader->mPixelShader.Get(), nullptr, 0);

                    context.SetVertexBuffer(FullscreenQuadVertexBuffer, 0);
                    context.mGfxDeviceContext->Draw(3, 0);

                    const bool DontRestoreContextState = false;
                    context.mGfxDeviceContext->FinishCommandList(DontRestoreContextState, &BlitRenderingCommandList);
                    mGfxDeviceImmediateContext->mGfxDeviceContext->ExecuteCommandList(BlitRenderingCommandList, DontRestoreContextState);
                }
            );
            MainGraph.BindOutput(blitRenderTarget);
            MainGraph.BindOutput(depthStencil);
            MainGraph.Compile();
            MainGraph.Execute(*mGfxDeviceDeferredContext);
        }

        mGfxSwapChain->Present(0, 0);
    }

    bool RenderSystem::OnResizeWindow(void* hWindow, int width, int height)
    {
        if (mMainWindowHandle == (HWND)hWindow)
        {
            mWindowWidth = width;
            mWindowHeight = height;

            RECT clientRect;
            ::GetClientRect(mMainWindowHandle, &clientRect);
            mClientWidth = clientRect.right - clientRect.left;
            mClientHeight = clientRect.bottom - clientRect.top;
            return true;
        }
        return false;
    }

    void RenderSystem::RenderScene(Scene& scene, GfxDeferredContext& context, const ViewConstantBufferData& data, bool shadow)
    {
        static bool initialize = false;
        static bool initialize_error = false;
        if (!initialize)
        {
            const unsigned int SceneConstBufferLength = sizeof(math::float4x4) * 3 + sizeof(math::float4) * 2;
            SceneConstantBufferPtr = mGfxDevice->CreateDynamicConstantBuffer(SceneConstBufferLength);

            const unsigned int ObjectConstBufferLength = sizeof(ObjectConstantDesc);
            ObjectConstantBufferPtr = mGfxDevice->CreateDynamicConstantBuffer(ObjectConstBufferLength);

            const unsigned int LightViewConstBufferLength = sizeof(LightViewConstantDesc);
            LightViewConstantBufferPtr = mGfxDevice->CreateDynamicConstantBuffer(LightViewConstBufferLength);

            // 创建顶点着色器
            const std::string vsEntryName = "VSMain";
            const std::string psEntryName = "PSMain";
            const std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutArray(InputLayout, InputLayout + InputLayoutCount);
            auto VertexShader = CreateVertexShader(mGfxDevice->mGfxDevice, DefaultVertexShaderSourceCode, vsEntryName, InputLayoutArray);
            auto PixelShader = CreatePixelShader(mGfxDevice->mGfxDevice, SimpleColorPixelShaderSourceCode, psEntryName);
            SimpleShaderProgramPtr = LinkShader(VertexShader, PixelShader);


            if (ShadowShaderProgramPtr == nullptr)
            {
                const std::string vsEntryName = "VSMain";
                const std::string psEntryName = "PSMain";
                const std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutArray(InputLayout, InputLayout + InputLayoutCount);
                auto VertexShader = CreateVertexShader(mGfxDevice->mGfxDevice, ShadowVertexShaderSource, vsEntryName, InputLayoutArray);
                auto PixelShader = CreatePixelShader(mGfxDevice->mGfxDevice, ShadowPixelShaderSource, psEntryName);
                ShadowShaderProgramPtr = LinkShader(VertexShader, PixelShader);
                ASSERT(ShadowShaderProgramPtr != nullptr);
            }


            if (SceneConstantBufferPtr == nullptr || ObjectConstantBufferPtr == nullptr
                || SimpleShaderProgramPtr == nullptr)
            {
                initialize_error = true;
                safe_delete(SceneConstantBufferPtr);
                safe_delete(ObjectConstantBufferPtr);
                safe_delete(SimpleShaderProgramPtr);
                safe_delete(BlitShaderProgramPtr);
            }
            initialize = true;
        }

        if (initialize && !initialize_error)
        {
            float aspect = mWindowWidth / (float)mWindowHeight;
            math::float2 zNearFar(1.0f, 50.0f);

            // TODO: Vertex and pixel use same constant buffer here.
            math::float4* pConstBuffer = context.MapBuffer<math::float4>(*SceneConstantBufferPtr);
            {
                pConstBuffer[0] = data.ViewMatrix.rows[0];
                pConstBuffer[1] = data.ViewMatrix.rows[1];
                pConstBuffer[2] = data.ViewMatrix.rows[2];
                pConstBuffer[3] = data.ViewMatrix.rows[3];

                pConstBuffer[4] = data.InvViewMatrix.rows[0];
                pConstBuffer[5] = data.InvViewMatrix.rows[1];
                pConstBuffer[6] = data.InvViewMatrix.rows[2];
                pConstBuffer[7] = data.InvViewMatrix.rows[3];

                math::float4x4 projMatrix = math::perspective_lh_matrix4x4f(math::degree<float>(45), aspect, zNearFar);
                pConstBuffer[8] = projMatrix.rows[0];
                pConstBuffer[9] = projMatrix.rows[1];
                pConstBuffer[10] = projMatrix.rows[2];
                pConstBuffer[11] = projMatrix.rows[3];

                pConstBuffer[12] = math::float4(data.LightColor, 1.0f);
                pConstBuffer[13] = math::float4(data.LightDirection, 0.0f);
            }
            context.UnmapBuffer(*SceneConstantBufferPtr);

            LightViewConstantDesc* pLightViewConstBuffer = context.MapBuffer<LightViewConstantDesc>(*LightViewConstantBufferPtr);
            {
                //TODO:
                engine::DirectionalLight* pLight = dynamic_cast<engine::DirectionalLight*>(scene.GetDirectionalLight(0));
                pLightViewConstBuffer->MatrixView = pLight->GetViewMatrix();
                pLightViewConstBuffer->MatrixProj = math::ortho_lh_matrix4x4f(20, 20, -50, 50);
            }
            context.UnmapBuffer(*LightViewConstantBufferPtr);

            context.mGfxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context.mGfxDeviceContext->IASetInputLayout(SimpleShaderProgramPtr->VertexShader->mInputLayout.Get());
            if (shadow)
            {
                context.mGfxDeviceContext->VSSetShader(ShadowShaderProgramPtr->VertexShader->mVertexShader.Get(), nullptr, 0);
                context.SetVSConstantBufferImpl(1, LightViewConstantBufferPtr);
            }
            else
            {
                context.mGfxDeviceContext->VSSetShader(SimpleShaderProgramPtr->VertexShader->mVertexShader.Get(), nullptr, 0);
                context.SetVSConstantBufferImpl(1, SceneConstantBufferPtr);
                context.SetVSConstantBufferImpl(2, LightViewConstantBufferPtr);
            }
            if (shadow)
                context.mGfxDeviceContext->PSSetShader(ShadowShaderProgramPtr->PixelShader->mPixelShader.Get(), nullptr, 0);
            else
                context.mGfxDeviceContext->PSSetShader(SimpleShaderProgramPtr->PixelShader->mPixelShader.Get(), nullptr, 0);
            context.SetPSConstantBufferImpl(0, SceneConstantBufferPtr);
            scene.RecursiveRender(mGfxDeviceDeferredContext.get());
        }

    }


    void RenderSystem::SetObjectToWorldMatrixForTest(const math::float4x4& matrix)
    {
        ObjectConstantDesc* pConstBuffer = mGfxDeviceDeferredContext->MapBuffer<ObjectConstantDesc>(*ObjectConstantBufferPtr);
        {
            pConstBuffer->MatrixObjectToWorld = matrix;
            pConstBuffer->MatrixWorldToObject = inversed(matrix);
        }
        mGfxDeviceDeferredContext->UnmapBuffer(*ObjectConstantBufferPtr);
        mGfxDeviceDeferredContext->SetVSConstantBufferImpl(0, ObjectConstantBufferPtr);
    }
}
