#include "RenderSystem.h"
#include <Foundation/Base/ScopeHelper.h>
#include <Foundation/Base/MemoryHelper.h>
#include <vector>
#include <string>
#include <d3dcompiler.h>
#include "Core/GameEngine.h"
#include "Scene/Components.h"
#include "Scene/Scene.h"

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
        output.ViewDirWS        = CameraPositionWS - ModelPosition.xyz;
        return output;
    }
)";

const std::string SimpleColorPixelShaderSourceCode = R"(
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
    };
    float4 PSMain(VertexOutput input) : SV_TARGET
    {
        float3 N = normalize(input.Normal);
        float3 L = -LightDirection;
        float3 V = normalize(input.ViewDirWS);
        float3 H = normalize(L + V);
        float NoL = max(0.0, dot(L, N));
        float NoH = max(0.0, dot(N, H));
        float3 Ambient = float3(0.15, 0.1, 0.1);
        float3 Diffuse = float3(1.0, 1.0, 1.0) * NoL;
        float3 Specular = pow(NoH, 128);
        float4 Color = float4((Diffuse + Specular) * LightColor + Ambient, 1.0);
        return Color;
    }
)";

namespace engine
{
    GfxDynamicConstantBuffer* ObjectConstantBufferPtr = nullptr;
    GfxDynamicConstantBuffer* SceneConstantBufferPtr = nullptr;
    ShaderProgram* SimpleShaderProgramPtr = nullptr;
    ID3D11CommandList* CubeRenderingCommandList = nullptr;

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
        safe_delete(ObjectConstantBufferPtr);
        safe_delete(SceneConstantBufferPtr);
        safe_delete(SimpleShaderProgramPtr);
        SafeRelease(CubeRenderingCommandList);

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
        ComPtr<ID3D11Texture2D> outBackbuffer = nullptr;
        ComPtr<ID3D11Texture2D> outBackbufferDS = nullptr;
        ComPtr<ID3D11RenderTargetView> outBackbufferRTV = nullptr;
        ComPtr<ID3D11DepthStencilView> outBackbufferDSV = nullptr;
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

        DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
        swapchainDesc.Width = mClientWidth;
        swapchainDesc.Height = mClientHeight;
        swapchainDesc.BufferCount = 2;
        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.Stereo = false;
        swapchainDesc.SampleDesc.Count = 1;
        swapchainDesc.SampleDesc.Quality = 0;
        swapchainDesc.Flags = 0;
        swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
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
        HRESULT resultGetBackbuffer = outSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &outBackbuffer);
        if (FAILED(resultGetBackbuffer))
        {
            ASSERT_SUCCEEDED(resultGetBackbuffer);
            return EGfxIntializationError::RetrieveBackbufferFail;
        }

        HRESULT resultCreateBackbufferRT = outD3DDevice->CreateRenderTargetView(outBackbuffer.Get(), nullptr, &outBackbufferRTV);
        if (FAILED(resultCreateBackbufferRT))
        {
            ASSERT_SUCCEEDED(resultCreateBackbufferRT);
            return EGfxIntializationError::CreateBackbufferRTVFail;
        }

        D3D11_TEXTURE2D_DESC depthStencilDesc;
        depthStencilDesc.Width = mClientWidth;
        depthStencilDesc.Height = mClientHeight;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.CPUAccessFlags = 0;
        depthStencilDesc.MiscFlags = 0;
        HRESULT resultCreateDSBuffer = outD3DDevice->CreateTexture2D(&depthStencilDesc, nullptr, &outBackbufferDS);
        if (FAILED(resultCreateDSBuffer))
        {
            ASSERT_SUCCEEDED(resultCreateDSBuffer);
            return EGfxIntializationError::CreateBackbufferDSFail;
        }

        HRESULT resultCreateDSV = outD3DDevice->CreateDepthStencilView(outBackbufferDS.Get(), nullptr, &outBackbufferDSV);
        ASSERT_SUCCEEDED(resultCreateDSV);
        if (FAILED(resultCreateDSV))
        {
            ASSERT_SUCCEEDED(resultCreateDSV);
            return EGfxIntializationError::CreateBackbufferDSVFail;
        }

        HRESULT resultCreateDeferredContext = outD3DDevice->CreateDeferredContext(0, &outD3DDeferredContext);
        if (FAILED(resultCreateDeferredContext))
        {
            ASSERT_SUCCEEDED(resultCreateDeferredContext);
            return EGfxIntializationError::CreateDeferredContextFail;
        }


        mGfxDevice = std::make_unique<GfxDevice>(outD3DDevice.Detach());
        mGfxDeviceImmediateContext = std::make_unique<GfxImmediateContext>(mGfxDevice.get(), outD3DDeviceImmediateContext.Detach());
        mGfxDeviceDeferredContext = std::make_unique<GfxDeferredContext>(mGfxDevice.get(), outD3DDeferredContext.Detach());
        mGfxSwapChain = outSwapChain;
        mBackbuffer = outBackbuffer;
        mBackbufferDS = outBackbufferDS;
        mBackbufferRTV = outBackbufferRTV;
        mBackbufferDSV = outBackbufferDSV;
        return EGfxIntializationError::NoError;
    }

    void RenderSystem::RenderFrame(Scene& scene)
    {
        ViewConstantBufferData data;

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

        if (CubeRenderingCommandList == nullptr)
        {
            auto RenderTagetView = mBackbufferRTV.Get();
            float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

            D3D11_RASTERIZER_DESC RasterizerDesc;
            RasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
            RasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
            RasterizerDesc.FrontCounterClockwise = TRUE;
            RasterizerDesc.DepthBias = 0;
            RasterizerDesc.DepthBiasClamp = 0.0f;
            RasterizerDesc.SlopeScaledDepthBias = 0.0;
            RasterizerDesc.DepthClipEnable = TRUE;
            RasterizerDesc.ScissorEnable = FALSE;
            RasterizerDesc.MultisampleEnable = TRUE;
            RasterizerDesc.AntialiasedLineEnable = TRUE;

            ComPtr<ID3D11RasterizerState> pRasterState;
            mGfxDevice->mGfxDevice->CreateRasterizerState(&RasterizerDesc, pRasterState.ReleaseAndGetAddressOf());
            mGfxDeviceDeferredContext->mGfxDeviceContext->RSSetState(pRasterState.Get());

            mGfxDeviceDeferredContext->mGfxDeviceContext->OMSetRenderTargets(1, &RenderTagetView, mBackbufferDSV.Get());
            mGfxDeviceDeferredContext->mGfxDeviceContext->ClearRenderTargetView(RenderTagetView, ClearColor);
            mGfxDeviceDeferredContext->mGfxDeviceContext->ClearDepthStencilView(mBackbufferDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            D3D11_VIEWPORT Viewport;
            Viewport.TopLeftX = 0.0f;
            Viewport.TopLeftY = 0.0f;
            Viewport.Width = (float)mClientWidth;
            Viewport.Height = (float)mClientHeight;
            Viewport.MinDepth = 0.0f;
            Viewport.MaxDepth = 1.0f;

            mGfxDeviceDeferredContext->mGfxDeviceContext->RSSetViewports(1, &Viewport);

            RenderScene(scene, data);
            mGfxDeviceDeferredContext->mGfxDeviceContext->FinishCommandList(false, &CubeRenderingCommandList);
        }
        mGfxDeviceImmediateContext->mGfxDeviceContext->ExecuteCommandList(CubeRenderingCommandList, false);
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

    void RenderSystem::RenderScene(Scene& scene, const ViewConstantBufferData& data)
    {
        static bool initialize = false;
        static bool initialize_error = false;
        if (!initialize)
        {
            const unsigned int SceneConstBufferLength = sizeof(math::float4x4) * 3 + sizeof(math::float4) * 2;
            SceneConstantBufferPtr = mGfxDevice->CreateDynamicConstantBuffer(SceneConstBufferLength);

            const unsigned int ObjectConstBufferLength = sizeof(ObjectConstantDesc);
            ObjectConstantBufferPtr = mGfxDevice->CreateDynamicConstantBuffer(ObjectConstBufferLength);

            // 创建顶点着色器
            const std::string vsEntryName = "VSMain";
            const std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutArray(InputLayout, InputLayout + InputLayoutCount);
            auto VertexShader = CreateVertexShader(mGfxDevice->mGfxDevice, DefaultVertexShaderSourceCode, vsEntryName, InputLayoutArray);

            const std::string psEntryName = "PSMain";
            auto PixelShader = CreatePixelShader(mGfxDevice->mGfxDevice, SimpleColorPixelShaderSourceCode, psEntryName);
            SimpleShaderProgramPtr = LinkShader(VertexShader, PixelShader);

            if (SceneConstantBufferPtr == nullptr || ObjectConstantBufferPtr == nullptr || SimpleShaderProgramPtr == nullptr)
            {
                initialize_error = true;
                safe_delete(SceneConstantBufferPtr);
                safe_delete(ObjectConstantBufferPtr);
                safe_delete(SimpleShaderProgramPtr);
            }
            initialize = true;
        }

        if (initialize && !initialize_error)
        {
            float aspect = mWindowWidth / (float)mWindowHeight;
            math::float2 zNearFar(1.0f, 50.0f);

            // TODO: Vertex and pixel use same constant buffer here.
            math::float4* pConstBuffer = mGfxDeviceDeferredContext->MapBuffer<math::float4>(*SceneConstantBufferPtr);
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
            mGfxDeviceDeferredContext->UnmapBuffer(*SceneConstantBufferPtr);

            mGfxDeviceDeferredContext->mGfxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            mGfxDeviceDeferredContext->mGfxDeviceContext->IASetInputLayout(SimpleShaderProgramPtr->VertexShader->mInputLayout.Get());
            mGfxDeviceDeferredContext->mGfxDeviceContext->VSSetShader(SimpleShaderProgramPtr->VertexShader->mVertexShader.Get(), nullptr, 0);

            mGfxDeviceDeferredContext->SetVSConstantBufferImpl(1, SceneConstantBufferPtr);
            mGfxDeviceDeferredContext->mGfxDeviceContext->PSSetShader(SimpleShaderProgramPtr->PixelShader->mPixelShader.Get(), nullptr, 0);
            mGfxDeviceDeferredContext->SetPSConstantBufferImpl(0, SceneConstantBufferPtr);
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
