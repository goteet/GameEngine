#include "RenderSystem.h"
#include <Foundation/Base/ScopeHelper.h>
#include <Foundation/Base/MemoryHelper.h>
#include <vector>
#include <d3dcompiler.h>
#include "Core/GameEngine.h"
#include "Scene/Components.h"

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

struct VertexLayout
{
    math::float4 Position;
    math::float3 Normal;
    math::float2 Texcoord;
};

const unsigned int InputLayoutCount = 3;
D3D11_INPUT_ELEMENT_DESC InputLayout[InputLayoutCount]
{
    D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const unsigned int CubeVertexCount = 24;
VertexLayout CubeVertices[CubeVertexCount] =
{
    { math::float4(-1, +1, -1, 1), math::normalized_float3::unit_y(),  math::float2(0, 0) },
    { math::float4(-1, +1, +1, 1), math::normalized_float3::unit_y(),  math::float2(0, 1) },
    { math::float4(+1, +1, +1, 1), math::normalized_float3::unit_y(),  math::float2(1, 1) },
    { math::float4(+1, +1, -1, 1), math::normalized_float3::unit_y(),  math::float2(1, 0) },

    { math::float4(-1, -1, +1, 1), math::normalized_float3::unit_y_neg(),  math::float2(0, 0) },
    { math::float4(-1, -1, -1, 1), math::normalized_float3::unit_y_neg(),  math::float2(0, 1) },
    { math::float4(+1, -1, -1, 1), math::normalized_float3::unit_y_neg(),  math::float2(1, 1) },
    { math::float4(+1, -1, +1, 1), math::normalized_float3::unit_y_neg(),  math::float2(1, 0) },


    { math::float4(+1, -1, -1, 1), math::normalized_float3::unit_x(),  math::float2(0, 0) },
    { math::float4(+1, +1, -1, 1), math::normalized_float3::unit_x(),  math::float2(0, 1) },
    { math::float4(+1, +1, +1, 1), math::normalized_float3::unit_x(),  math::float2(1, 1) },
    { math::float4(+1, -1, +1, 1), math::normalized_float3::unit_x(),  math::float2(1, 0) },

    { math::float4(-1, -1, +1, 1), math::normalized_float3::unit_x_neg(),  math::float2(0, 0) },
    { math::float4(-1, +1, +1, 1), math::normalized_float3::unit_x_neg(),  math::float2(0, 1) },
    { math::float4(-1, +1, -1, 1), math::normalized_float3::unit_x_neg(),  math::float2(1, 1) },
    { math::float4(-1, -1, -1, 1), math::normalized_float3::unit_x_neg(),  math::float2(1, 0) },

    { math::float4(+1, -1, +1, 1), math::normalized_float3::unit_z(),  math::float2(0, 0) },
    { math::float4(+1, +1, +1, 1), math::normalized_float3::unit_z(),  math::float2(0, 1) },
    { math::float4(-1, +1, +1, 1), math::normalized_float3::unit_z(),  math::float2(1, 1) },
    { math::float4(-1, -1, +1, 1), math::normalized_float3::unit_z(),  math::float2(1, 0) },

    { math::float4(-1, -1, -1, 1), math::normalized_float3::unit_z_neg(),  math::float2(0, 0) },
    { math::float4(-1, +1, -1, 1), math::normalized_float3::unit_z_neg(),  math::float2(0, 1) },
    { math::float4(+1, +1, -1, 1), math::normalized_float3::unit_z_neg(),  math::float2(1, 1) },
    { math::float4(+1, -1, -1, 1), math::normalized_float3::unit_z_neg(),  math::float2(1, 0) }
};

const unsigned int CubeIndexCount = 36;
unsigned int CubeIndices[CubeIndexCount] =
{
    0, 3, 2, 2, 1, 0,
    4, 7, 6, 6, 5, 4,
    8, 11, 10, 10, 9, 8,
    12, 15, 14, 14, 13, 12,
    16, 19, 18, 18, 17, 16,
    20, 23, 22, 22, 21, 20,
};



struct GfxBaseBuffer
{
    virtual ~GfxBaseBuffer() = default;

    GfxBaseBuffer()
    {
        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage
        // Use Default/Immutable/Dynamic,
        //  For any default buffer, use UpdateSubresource with Staging Buffer to Upload data.
        mBufferDesc.Usage = D3D11_USAGE_DEFAULT;

        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_bind_flag
        mBufferDesc.BindFlags = 0;

        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_cpu_access_flag
        //0 if no CPU access is necessary. 
        mBufferDesc.CPUAccessFlags = 0;

        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag
        mBufferDesc.MiscFlags = 0;

        //https://docs.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-cs-resources#structured-buffer
        //Structure Buffer :
        mBufferDesc.StructureByteStride = 0;
    }

    ID3D11Device* mGfxDevicePtr = nullptr;
    D3D11_BUFFER_DESC mBufferDesc = { 0 };
    ComPtr<ID3D11Buffer> mBufferPtr = nullptr;
};

struct GfxImmutableBuffer : public GfxBaseBuffer
{
    GfxImmutableBuffer()
    {
        mBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        mBufferDesc.CPUAccessFlags = 0;
    }
};

struct GfxDynamicBuffer : public GfxBaseBuffer
{
    GfxDynamicBuffer()
    {
        mBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        mBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    }
};

struct GfxStagingBuffer : GfxBaseBuffer
{
    GfxStagingBuffer(bool CPURead, bool CPUWrite = true)
    {
        mBufferDesc.Usage = D3D11_USAGE_STAGING;
        mBufferDesc.BindFlags = 0;
        mBufferDesc.MiscFlags = 0;
        UINT WriteFlag = CPUWrite ? D3D11_CPU_ACCESS_WRITE : 0;
        UINT ReadFlag = CPURead ? D3D11_CPU_ACCESS_READ : 0;
        mBufferDesc.CPUAccessFlags = ReadFlag | WriteFlag;
        mBufferDesc.StructureByteStride = 0;
    }
};

struct GfxBaseVertexBuffer
{
    virtual ~GfxBaseVertexBuffer() = default;
};

struct GfxDefaultVertexBuffer : public GfxBaseBuffer, public GfxBaseVertexBuffer
{
    GfxDefaultVertexBuffer()
    {
        mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    }
};

struct GfxImmutableVertexBuffer : public GfxImmutableBuffer, public GfxBaseVertexBuffer
{
    GfxImmutableVertexBuffer()
    {
        mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    }
};

struct GfxDynamicVertexBuffer : public GfxDynamicBuffer, public GfxBaseVertexBuffer
{
    GfxDynamicVertexBuffer()
    {
        mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    }
};

struct GfxBaseIndexBuffer
{
    virtual ~GfxBaseIndexBuffer() = default;
};


struct GfxDefaultIndexBuffer : GfxBaseBuffer, GfxBaseIndexBuffer
{
    GfxDefaultIndexBuffer()
    {
        mBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    }
};

struct GfxDynamicConstBuffer : public GfxDynamicBuffer
{
    GfxDynamicConstBuffer()
    {
        mBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    }
};

enum EMapMethod
{
    Default, Discard, NoOverwrite
};

namespace context_private_impl
{
    bool CreateBuffer(ID3D11Device* GfxDevice,
        GfxBaseBuffer& OutBuffer,
        unsigned int BufferLength,
        D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
    {
        OutBuffer.mBufferDesc.ByteWidth = BufferLength;
        HRESULT resultBufferCreation = GfxDevice->CreateBuffer(
            &OutBuffer.mBufferDesc,
            SubResourceDataPtr,
            &OutBuffer.mBufferPtr);

        if (SUCCEEDED(resultBufferCreation))
        {
            OutBuffer.mGfxDevicePtr = GfxDevice;
            return true;
        }
        else
        {
            return false;
        }
    }

    void* MapBufferWrite(ID3D11DeviceContext* GfxDeviceContext, GfxBaseBuffer& buffer, UINT subresource, EMapMethod method)
    {
        D3D11_MAP mapType = method == Default
            ? D3D11_MAP_WRITE
            : (method == Discard
                ? D3D11_MAP_WRITE_DISCARD
                : D3D11_MAP_WRITE_NO_OVERWRITE);
        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map_flag
        //do-not-wait-when-gpu-busy
        UINT mapFlags = 0;
        D3D11_MAPPED_SUBRESOURCE outMappedSubresource;

        HRESULT result = GfxDeviceContext->Map(buffer.mBufferPtr.Get(), subresource, mapType, mapFlags, &outMappedSubresource);
        if (SUCCEEDED(result))
        {
            return outMappedSubresource.pData;
        }
        return nullptr;
    }

    void UnmapBufferWrite(ID3D11DeviceContext* GfxDeviceContext, GfxBaseBuffer& buffer, UINT subresource)
    {
        GfxDeviceContext->Unmap(buffer.mBufferPtr.Get(), subresource);
    }

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


template<typename VertexBufferType>
bool CreateVertexBuffer(ID3D11Device* GfxDevice,
    VertexBufferType& OutVertexBuffer,
    unsigned int VertexStride, unsigned int VertexCount,
    D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
{
    using context_private_impl::CreateBuffer;
    return CreateBuffer(GfxDevice, OutVertexBuffer, VertexStride * VertexCount, SubResourceDataPtr);
}

bool CreateIndexBuffer(ID3D11Device* GfxDevice,
    GfxDefaultIndexBuffer& OutIndexBuffer,
    unsigned int IndexCount,
    D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
{
    using context_private_impl::CreateBuffer;
    unsigned int IndexStride = sizeof(int);
    return CreateBuffer(GfxDevice, OutIndexBuffer, IndexStride * IndexCount, SubResourceDataPtr);
}

template<typename GfxBufferType>
bool CreateConstBuffer(ID3D11Device* GfxDevice,
    GfxBufferType& OutIndexBuffer,
    unsigned int BufferLength,
    D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
{
    using context_private_impl::CreateBuffer;
    return CreateBuffer(GfxDevice, OutIndexBuffer, BufferLength, SubResourceDataPtr);
}

void* MapBuffer(ID3D11DeviceContext* GfxDeviceContext, GfxStagingBuffer& buffer, UINT subresource = 0)
{
    using context_private_impl::MapBufferWrite;
    return MapBufferWrite(GfxDeviceContext, buffer, subresource, EMapMethod::Default);
}

void* MapBuffer(ID3D11DeviceContext* GfxDeviceContext, GfxDynamicBuffer& buffer, UINT subresource = 0)
{
    using context_private_impl::MapBufferWrite;
    return MapBufferWrite(GfxDeviceContext, buffer, subresource, EMapMethod::Discard);
}

void UnmapBuffer(ID3D11DeviceContext* GfxDeviceContext, GfxStagingBuffer& buffer, UINT subresource = 0)
{
    using context_private_impl::UnmapBufferWrite;
    UnmapBufferWrite(GfxDeviceContext, buffer, subresource);
}

void UnmapBuffer(ID3D11DeviceContext* GfxDeviceContext, GfxDynamicBuffer& buffer, UINT subresource = 0)
{
    using context_private_impl::UnmapBufferWrite;
    UnmapBufferWrite(GfxDeviceContext, buffer, subresource);
}

bool UploadEntireBuffer(ID3D11Device* GfxDevice, ID3D11DeviceContext* GfxDeviceContext,
    GfxDefaultVertexBuffer& Buffer, const VertexLayout* vertices)
{
    /*
    1. CreateVertexBuffer a 2nd buffer with D3D11_USAGE_STAGING;
    -fill the second buffer using ID3D11DeviceContext::Map, ID3D11DeviceContext::Unmap;
    -use ID3D11DeviceContext::CopyResource to copy from the staging buffer to the default buffer.

    2. Use ID3D11DeviceContext::UpdateSubresource to copy data from memory.
    */
    GfxStagingBuffer StagingBuffer(false);
    if (CreateVertexBuffer<GfxStagingBuffer>(GfxDevice, StagingBuffer, sizeof(VertexLayout), CubeVertexCount))
    {
        VertexLayout* pDataPtr = (VertexLayout* )MapBuffer(GfxDeviceContext, StagingBuffer);
        memcpy(pDataPtr, CubeVertices, Buffer.mBufferDesc.ByteWidth);
        UnmapBuffer(GfxDeviceContext, StagingBuffer);
        GfxDeviceContext->CopyResource(Buffer.mBufferPtr.Get(), StagingBuffer.mBufferPtr.Get());
        return true;
    }
    else
    {
        return false;
    }
}

bool UploadEntireBuffer(ID3D11DeviceContext* GfxDeviceContext,
    GfxDefaultIndexBuffer& Buffer, const unsigned int* indices)
{
    //2. Use ID3D11DeviceContext::UpdateSubresource to copy data from memory.
    //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11calcsubresource
    UINT subresource = 0;
    UINT sourceRowPitch = 0;
    UINT sourceDepthPitch = 0;
    GfxDeviceContext->UpdateSubresource(Buffer.mBufferPtr.Get(),
        subresource, nullptr,
        indices, sourceRowPitch, sourceDepthPitch);

    return true;
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
    ComPtr<ID3DBlob> vsBlob =  CompileShaderSource(source, entry, profile);

    if (vsBlob == nullptr)
    {
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
        return nullptr;
    }

    HRESULT rstCreateInputLayout = GfxDevice->CreateInputLayout(
        &(layout[0]), (UINT)layout.size(),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &inputLayout);

    if (FAILED(rstCreateInputLayout))
    {
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
        return nullptr;
    }

    ComPtr<ID3D11PixelShader> pixelShader = nullptr;
    HRESULT rstCreatePixelShader = GfxDevice->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        NULL,
        &pixelShader);

    if (FAILED(rstCreatePixelShader))
    {
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
        return nullptr;

    ShaderProgram* program = new ShaderProgram();
    program->VertexShader = vertexShaderImpl;
    program->PixelShader = pixelShaderImpl;
    return program;
}

const std::string DefaultVertexShaderSourceCode = R"(
    cbuffer scene
    {
        float4x4 MatrixView;
        float4x4 MatrixProj;
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
        float3 Color     : TEXCOORD0;
    };
    VertexOutput VSMain(VertexLayout input)
    {
        VertexOutput output;
        float4 ModelPosition = float4(input.Position.xyz * 0.5, input.Position.w);
        float4 ViewPosition  = mul(ModelPosition, MatrixView);
        output.Position      = mul(ViewPosition, MatrixProj);
        output.Color = ModelPosition.xyz + 0.5;
        return output;
    }
)";

const std::string SimpleColorPixelShaderSourceCode = R"(
    struct VertexOutput
    {
        float4 Position : SV_POSITION;
        float3 Color    : TEXCOORD0;
    };
    float4 PSMain(VertexOutput input) : SV_TARGET
    {
        return float4(input.Color, 1.0);
    }
)";

namespace engine
{
    GfxDynamicBuffer* VertexShaderBufferPtr = nullptr;
    GfxDefaultVertexBuffer* CubeVertexBufferPtr = nullptr;
    GfxDefaultIndexBuffer* CubeIndexBufferPtr = nullptr;
    ShaderProgram* SimpleShaderProgramPtr = nullptr;

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

    RenderSystem::~RenderSystem()
    {
        safe_delete(CubeVertexBufferPtr);
        safe_delete(CubeIndexBufferPtr);
        safe_delete(VertexShaderBufferPtr);
        safe_delete(SimpleShaderProgramPtr);
    }

    bool RenderSystem::InitializeGfxDevice()
    {
        //CreateVertexBuffer Device and DeviceContext
        ComPtr<ID3D11Device> outD3DDevice = nullptr;
        ComPtr<ID3D11DeviceContext> outD3DDeviceImmediateContext = nullptr;
        ComPtr<IDXGISwapChain1> outSwapChain = nullptr;
        ComPtr<ID3D11Texture2D> outBackbuffer = nullptr;
        ComPtr<ID3D11RenderTargetView> outBackbufferRTV = nullptr;
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
            return false;
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
            return false;
        }

        IDXGIOutput* noneDXGIOutput = nullptr;
        HRESULT resultCreateSwapchain = DXGIFactory->CreateSwapChainForHwnd(outD3DDevice.Get(), mMainWindowHandle, &swapchainDesc, &fullScreenDesc, noneDXGIOutput, &outSwapChain);

        if (FAILED(resultCreateSwapchain))
        {
            return false;
        }

        //CreateVertexBuffer RenderTargetView
        HRESULT resultGetBackbuffer = outSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &outBackbuffer);
        if (FAILED(resultGetBackbuffer))
        {
            return false;
        }


        HRESULT resultCreateBackbufferRT = outD3DDevice->CreateRenderTargetView(outBackbuffer.Get(), nullptr, &outBackbufferRTV);
        if (FAILED(resultCreateBackbufferRT))
        {
            return false;
        }

        mGfxDevice = outD3DDevice;
        mGfxDeviceContext = outD3DDeviceImmediateContext;
        mGfxSwapChain = outSwapChain;
        mBackbuffer = outBackbuffer;
        mBackbufferRTV = outBackbufferRTV;

        return true;
    }

    void RenderSystem::RenderFrame()
    {
        math::float4x4 viewMatrix = math::scale_matrix4x4f(2,2,2);
        GameEngine* Engine = GetEngineInstance();
        Scene* defaultScene = Engine->GetDefaultSceneInternal();
        if (defaultScene != nullptr)
        {
            Camera* defaultCamera = defaultScene->GetDefaultCameraInternal();
            if (defaultCamera != nullptr)
            {
                viewMatrix = defaultCamera->GetUpdatedViewMatrix();
            }
        }

        auto RenderTagetView = mBackbufferRTV.Get();
        float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        mGfxDeviceContext->OMSetRenderTargets(1, &RenderTagetView, nullptr);
        mGfxDeviceContext->ClearRenderTargetView(RenderTagetView, ClearColor);

        D3D11_VIEWPORT Viewport;
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = 0.0f;
        Viewport.Width = (float)mClientWidth;
        Viewport.Height = (float)mClientHeight;
        Viewport.MinDepth = 0.0f;
        Viewport.MaxDepth = 1.0f;

        mGfxDeviceContext->RSSetViewports(1, &Viewport);

        RenderSimpleBox(viewMatrix);

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
    void RenderSystem::RenderSimpleBox(const math::float4x4& viewMatrix)
    {
        static bool initialize = false;
        static bool initialize_error = false;
        if (!initialize)
        {
            CubeVertexBufferPtr = new GfxDefaultVertexBuffer();
            CubeIndexBufferPtr = new GfxDefaultIndexBuffer();
            VertexShaderBufferPtr = new GfxDynamicConstBuffer();
            const unsigned int VertexStride = sizeof(VertexLayout);
            const unsigned int VertexBufferLength = sizeof(math::float4x4) * 2;

            bool vbc = CreateVertexBuffer(mGfxDevice.Get(), *CubeVertexBufferPtr, VertexStride, CubeVertexCount);
            bool ibc = CreateIndexBuffer(mGfxDevice.Get(), *CubeIndexBufferPtr, CubeIndexCount);
            bool cbc = CreateConstBuffer(mGfxDevice.Get(), *VertexShaderBufferPtr, VertexBufferLength);

            ComPtr<ID3DBlob> blob;

            // 创建顶点着色器
            const std::string vsEntryName = "VSMain";
            const std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayoutArray(InputLayout, InputLayout + InputLayoutCount);
            auto VertexShader = CreateVertexShader(mGfxDevice.Get(), DefaultVertexShaderSourceCode, vsEntryName, InputLayoutArray);

            const std::string psEntryName = "PSMain";
            auto PixelShader = CreatePixelShader(mGfxDevice.Get(), SimpleColorPixelShaderSourceCode, psEntryName);
            SimpleShaderProgramPtr = LinkShader(VertexShader, PixelShader);

            if (vbc && ibc && cbc && SimpleShaderProgramPtr)
            {
                //upload cube vertex data to gfx vertex buffer.
                if (!UploadEntireBuffer(mGfxDevice.Get(), mGfxDeviceContext.Get(), *CubeVertexBufferPtr, CubeVertices)
                    || !UploadEntireBuffer(mGfxDeviceContext.Get(), *CubeIndexBufferPtr, CubeIndices))
                {
                    initialize_error = true;
                    safe_delete(CubeVertexBufferPtr);
                    safe_delete(CubeIndexBufferPtr);
                    safe_delete(VertexShaderBufferPtr);
                    safe_delete(SimpleShaderProgramPtr);
                }
            }
            else
            {
                initialize_error = true;
                safe_delete(CubeVertexBufferPtr);
                safe_delete(CubeIndexBufferPtr);
                safe_delete(VertexShaderBufferPtr);
                safe_delete(SimpleShaderProgramPtr);
            }
            initialize = true;
        }

        if (initialize && !initialize_error)
        {
            float aspect = mWindowWidth / (float)mWindowHeight;
            math::float2 zNearFar(1.0f, 50.0f);
            math::float4x4* pConstBuffer = (math::float4x4*)MapBuffer(mGfxDeviceContext.Get(), *VertexShaderBufferPtr);
            pConstBuffer[0] = viewMatrix;
            pConstBuffer[1] = math::perspective_lh_matrix4x4f(math::degree<float>(45), aspect, zNearFar);
            UnmapBuffer(mGfxDeviceContext.Get(), *VertexShaderBufferPtr);

            ID3D11Buffer* VertexBuffer = CubeVertexBufferPtr->mBufferPtr.Get();
            ID3D11Buffer* VertexShaderBuffer = VertexShaderBufferPtr->mBufferPtr.Get();
            UINT Stride = sizeof(VertexLayout);
            UINT Offset = 0;
            mGfxDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            mGfxDeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
            mGfxDeviceContext->IASetIndexBuffer(CubeIndexBufferPtr->mBufferPtr.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
            mGfxDeviceContext->IASetInputLayout(SimpleShaderProgramPtr->VertexShader->mInputLayout.Get());
            mGfxDeviceContext->VSSetConstantBuffers(0, 1, &VertexShaderBuffer);
            mGfxDeviceContext->VSSetShader(SimpleShaderProgramPtr->VertexShader->mVertexShader.Get(), nullptr, 0);
            mGfxDeviceContext->PSSetShader(SimpleShaderProgramPtr->PixelShader->mPixelShader.Get(), nullptr, 0);
            mGfxDeviceContext->DrawIndexed(CubeIndexCount, 0, 0);
        }
    }
}
