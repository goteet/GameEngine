#include <vector>
#include <string>
#include <Foundation/Base/ScopeHelper.h>
#include <Foundation/Base/MemoryHelper.h>
#include <GfxInterface.h>
#include <d3dcompiler.h>

#include "RenderSystem.h"
#include "Core/GameEngine.h"
#include "Scene/Components.h"
#include "Scene/Scene.h"
#include "Render/FrameGraph.h"

#define USE_VULKAN 1

using VertexInputLayout = GFXI::GraphicPipelineState::VertexInputLayout;
const uint32_t NumVertexBinding = 1;
const uint32_t NumVertexAttribute = 3;
GFXI::GraphicPipelineState::VertexInputLayout::Binding InputVertexBinding = { VertexInputLayout::EInputRate::Vertex, 0, (16 + 12 + 8) };
GFXI::GraphicPipelineState::VertexInputLayout::Attribute InputVertexAttributes[NumVertexAttribute] =
{
    { VertexInputLayout::EVertexFormat::RGBA32_SFloat,  "POSITION", 0, 0, 0, 0},
    { VertexInputLayout::EVertexFormat::RGB32_SFloat,   "NORMAL",   0, 0, 1, 16 },
    { VertexInputLayout::EVertexFormat::RG32_SFloat,    "TEXCOORD", 0, 0, 2, 28 }
};

GFXI::Shader* CreateVertexShader(GFXI::GraphicDevice* GfxDevice,const std::string& source, const std::string& entry, const std::string& name)
{
    GFXI::ShaderBinary::CreateInfo binaryCreateInfo;
    binaryCreateInfo.ShaderType = GFXI::EShaderType::VertexShader;
    binaryCreateInfo.ShaderNameString = name.c_str();
    binaryCreateInfo.EntryNameString = entry.c_str();
    binaryCreateInfo.ShaderSourceCodeData = source.c_str();
    binaryCreateInfo.ShaderSourceCodeLength = static_cast<unsigned int>(source.size());


    GFXI::ShaderBinary* vsBinary = GfxDevice->CompileShader(binaryCreateInfo);
    if (vsBinary)
    {
        GFXI::Shader::CreateInfo shaderCreateInfo;
        shaderCreateInfo.ShaderBinary = vsBinary;
        GFXI::Shader* vertexShader = GfxDevice->CreateShader(shaderCreateInfo);
        vsBinary->Release();
        return vertexShader;
    }
    else
    {
        return nullptr;
    }
}

GFXI::Shader* CreatePixelShader(GFXI::GraphicDevice* GfxDevice, const std::string& source, const std::string& entry, const std::string& name)
{
    GFXI::ShaderBinary::CreateInfo binaryCreateInfo;
    binaryCreateInfo.ShaderType = GFXI::EShaderType::PixelShader;
    binaryCreateInfo.ShaderNameString = name.c_str();
    binaryCreateInfo.EntryNameString = entry.c_str();
    binaryCreateInfo.ShaderSourceCodeData = source.c_str();
    binaryCreateInfo.ShaderSourceCodeLength = static_cast<unsigned int>(source.size());


    GFXI::ShaderBinary* psBinary = GfxDevice->CompileShader(binaryCreateInfo);
    if (psBinary)
    {
        GFXI::Shader::CreateInfo shaderCreateInfo;
        shaderCreateInfo.ShaderBinary = psBinary;
        GFXI::Shader* pixelShader = GfxDevice->CreateShader(shaderCreateInfo);
        psBinary->Release();
        return pixelShader;
    }
    else
    {
        return nullptr;
    }
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


const std::string DefaultVertexShaderSourceCodeD3D11 = R"(
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
        //return float4(shadowDepthTexcoord,0,1);
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


const std::string ShadowVertexShaderSourceD3D11 = R"(
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

const std::string ShadowPixelShaderSourceD3D11 = R"(
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

const std::string ShadowVertexShaderSourceVulkan = R"(
    #version 440

    layout(std140, binding = 0) uniform ObjectDataBuffer
    {
        mat4 MatrixObjectToWorld;
        mat4 MatrixWorldToObject;
    } ObjectData;

    layout(std140, binding = 1) uniform LightDataBuffer
    {
        mat4 LightMatrixView;
        mat4 LightMatrixProj;
    } LightData;


    layout(location = 0) in vec4  i_Position;
    layout(location = 1) in vec3  i_Normal;
    layout(location = 2) in vec2  i_Texcoord0;
    layout(location = 0) out vec2 o_DepthPos;

    out gl_PerVertex
    {
        vec4 gl_Position;
    };

    void main()
    {
        vec4 ModelPosition  = ObjectData.MatrixObjectToWorld * i_Position;
        vec4 ViewPosition   = LightData.LightMatrixView      * ModelPosition;
        vec4 HomoPosition   = LightData.LightMatrixProj      * ViewPosition;
        o_DepthPos          = HomoPosition.zw;
        gl_Position         = HomoPosition;
    }
)";

const std::string ShadowPixelShaderSourceVulkan = R"(
    #version 440
    layout(location = 0) in vec2 i_DepthPos;
    layout(location = 0) out vec4 o_FragColor;
    void main()
    {
        o_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
)";

const std::string DefaultVertexShaderSourceCodeVulkan = R"(
    #version 440

    layout(std140, binding = 0) uniform ObjectDataBuffer
    {
        mat4 MatrixObjectToWorld;
        mat4 MatrixWorldToObject;
    } ObjectData;

    layout(std140, binding = 1) uniform SceneDataBuffer
    {
        mat4 MatrixView;
        mat4 InvMatrixView;
        mat4 MatrixProj;
        vec3   LightColor;
        float  Padding0;
        vec3   LightDirection;
        float  Padding1;
    } SceneData;

    layout(std140, binding = 2) uniform LightDataBuffer
    {
        mat4 LightMatrixView;
        mat4 LightMatrixProj;
    } LightData;

    layout(location = 0) in vec4 i_Position;
    layout(location = 1) in vec3 i_Normal;
    layout(location = 2) in vec2 i_Texcoord0;

    layout(location = 0) out vec3 o_Normal;
    layout(location = 1) out vec3 o_ViewDirWS;
    layout(location = 2) out vec4 o_LightPos;

    out gl_PerVertex
    {
        vec4 gl_Position;
    };

    void main()
    {
        vec4 ModelPosition  = ObjectData.MatrixObjectToWorld * i_Position;
        vec4 ViewPosition   = SceneData.MatrixView * ModelPosition;
        vec4 ModelNormal    = vec4(i_Normal, 0.0f);
        mat4 InvMatrixView = SceneData.InvMatrixView;
        vec3 CameraPositionWS = vec3(InvMatrixView[0][3], InvMatrixView[1][3], InvMatrixView[2][3]);

        gl_Position = SceneData.MatrixProj * ViewPosition;
        o_Normal    = normalize( (transpose(ObjectData.MatrixWorldToObject) * ModelNormal).xyz );
        o_ViewDirWS = CameraPositionWS - ModelPosition.xyz;
        o_LightPos  = (LightData.LightMatrixView * LightData.LightMatrixProj) * ModelPosition;
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


#if USE_VULKAN
const wchar_t* GFXModuleName = L"GfxInterfaceVulkan.dll";
const std::string& DefaultVertexShaderSourceCode = DefaultVertexShaderSourceCodeVulkan;
const std::string& ShadowVertexShaderSource = ShadowVertexShaderSourceVulkan;
const std::string& ShadowPixelShaderSource = ShadowPixelShaderSourceVulkan;
const std::string kVsEntryName = "main";
const std::string kPsEntryName = "main";
#else
const wchar_t* GFXModuleName = L"GfxInterfaceD3D11.dll";
const std::string& DefaultVertexShaderSourceCode = DefaultVertexShaderSourceCodeD3D11;
const std::string& ShadowVertexShaderSource = ShadowVertexShaderSourceD3D11;
const std::string& ShadowPixelShaderSource = ShadowPixelShaderSourceD3D11;
const std::string kVsEntryName = "VSMain";
const std::string kPsEntryName = "PSMain";
#endif

namespace engine
{
    void RenderSystem::FillEntireEntireBufferFromMemory(GFXI::Buffer* buffer, const void* data)
    {
        mGfxDevice->GetImmediateContext()->UpdateBuffer(buffer, data);
    }
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
        SafeRelease(mFullsceenQuadVertices);
        SafeRelease(mSceneUniformBuffer);
        SafeRelease(mObjectUniformBuffer);
        SafeRelease(mLightViewUniformBuffer);

        SafeRelease(mShadowPassVS);
        SafeRelease(mShadowPassPS);
        SafeRelease(mSimpleDrawPassVS);
        SafeRelease(mSimpleDrawPassPS);
        SafeRelease(mBlitPassVS);
        SafeRelease(mBlitPassPS);

        

        SafeRelease(mCubeRenderingQueueShadow);
        SafeRelease(mCubeRenderingQueueFinal);
        SafeRelease(mBlitRenderingQueue);
        SafeRelease(mDefaultSamplerState);
        SafeRelease(mShadowPassState);
        SafeRelease(mSimpleDrawPassState);
        SafeRelease(mBlitPassState);

        
        SafeRelease(mMainWindowDepthStencil);

        SafeRelease(mMainWindowSwapChain);
        SafeRelease(mGfxDevice);
        SafeRelease(mGfxModule);
    }


    GFXI::GraphicModule* LoadGfxLibrary(const TCHAR* LibraryName)
    {
        HMODULE Module = ::LoadLibraryW(LibraryName);
        if (Module != NULL)
        {
            typedef GFXI::GraphicModule* (*CreateGfxModule)();
            CreateGfxModule CreateGfxModulePtr = (CreateGfxModule)GetProcAddress(Module, "CreateGfxModule");
            if (CreateGfxModulePtr != nullptr)
            {
                GFXI::GraphicModule* GfxModule = CreateGfxModulePtr();
                return GfxModule;
            }
            FreeLibrary(Module);
        }
        return nullptr;
    }


    EGfxIntializationError RenderSystem::InitializeGfxDevice()
    {
        mGfxModule = LoadGfxLibrary(GFXModuleName);
        mGfxDevice = mGfxModule->CreateDevice();
        mMainWindowSwapChain = mGfxDevice->CreateSwapChain(mMainWindowHandle, mClientWidth, mClientHeight, mIsFullScreen);

        GFXI::GraphicPipelineState::CreateInfo PipelineCreateInfo;
        PipelineCreateInfo.VertexInputLayout.NumBindingArray = NumVertexBinding;
        PipelineCreateInfo.VertexInputLayout.BindingArray = &InputVertexBinding;
        PipelineCreateInfo.VertexInputLayout.NumAttributeArray = NumVertexAttribute;
        PipelineCreateInfo.VertexInputLayout.AttributeArray = InputVertexAttributes;
        PipelineCreateInfo.ColorBlendState.NumAttachments = 1;

        //if (mShadowPassState == nullptr)
        {
            mShadowPassVS = CreateVertexShader(mGfxDevice, ShadowVertexShaderSource, kVsEntryName, std::string("ShadowVS"));
            mShadowPassPS = CreatePixelShader(mGfxDevice, ShadowPixelShaderSource, kPsEntryName, std::string("ShadowPS"));
            PipelineCreateInfo.StageShaders[static_cast<int>(GFXI::GraphicPipelineState::EShaderStage::Vertex)] = mShadowPassVS;
            PipelineCreateInfo.StageShaders[static_cast<int>(GFXI::GraphicPipelineState::EShaderStage::Pixel)] = mShadowPassPS;
            PipelineCreateInfo.RasterizationState.ViewportInfo.X = 0.0f;
            PipelineCreateInfo.RasterizationState.ViewportInfo.Y = 0.0f;
            PipelineCreateInfo.RasterizationState.ViewportInfo.Width = (float)1024;
            PipelineCreateInfo.RasterizationState.ViewportInfo.Height = (float)1024;
            PipelineCreateInfo.RasterizationState.ViewportInfo.MinDepth = 0.0f;
            PipelineCreateInfo.RasterizationState.ViewportInfo.MaxDepth = 1.0f;
            mShadowPassState = mGfxDevice->CreateGraphicPipelineState(PipelineCreateInfo);
        }
        //if (mSimpleDrawPassState == nullptr)
        {
            mSimpleDrawPassVS = CreateVertexShader(mGfxDevice, DefaultVertexShaderSourceCode, kVsEntryName, std::string("ForwardVS"));
            mSimpleDrawPassPS = CreatePixelShader(mGfxDevice, SimpleColorPixelShaderSourceCode, kPsEntryName, std::string("ForwardPS"));
            PipelineCreateInfo.StageShaders[static_cast<int>(GFXI::GraphicPipelineState::EShaderStage::Vertex)] = mSimpleDrawPassVS;
            PipelineCreateInfo.StageShaders[static_cast<int>(GFXI::GraphicPipelineState::EShaderStage::Pixel)] = mSimpleDrawPassPS;
            PipelineCreateInfo.RasterizationState.ViewportInfo.Width  = mClientWidth;
            PipelineCreateInfo.RasterizationState.ViewportInfo.Height = mClientHeight;
            mSimpleDrawPassState = mGfxDevice->CreateGraphicPipelineState(PipelineCreateInfo);
        }
        //if (mBlitPassState == nullptr)
        {
            mBlitPassVS = CreateVertexShader(mGfxDevice, BlitVertexShaderSource, kVsEntryName, std::string("BlitVS"));
            mBlitPassPS = CreatePixelShader(mGfxDevice, BlitPixelShaderSource, kPsEntryName, std::string("BlitPS"));
            PipelineCreateInfo.StageShaders[static_cast<int>(GFXI::GraphicPipelineState::EShaderStage::Vertex)] = mBlitPassVS;
            PipelineCreateInfo.StageShaders[static_cast<int>(GFXI::GraphicPipelineState::EShaderStage::Pixel)] = mBlitPassPS;
            PipelineCreateInfo.RasterizationState.ViewportInfo.Width  = mClientWidth;
            PipelineCreateInfo.RasterizationState.ViewportInfo.Height = mClientHeight;
            mBlitPassState = mGfxDevice->CreateGraphicPipelineState(PipelineCreateInfo);

            GFXI::VertexBuffer::CreateInfo vertexBufferCreateInfo;
            vertexBufferCreateInfo.DataUsage = GFXI::EDataUsage::Immutable;
            vertexBufferCreateInfo.BufferSize = fullscreen_quad::FSQuadVertexCount * sizeof(engine::VertexLayout);
            vertexBufferCreateInfo.InitializedBufferDataPtr = fullscreen_quad::FSQuadVertices;
            mFullsceenQuadVertices = mGfxDevice->CreateVertexBuffer(vertexBufferCreateInfo);
        }

        GFXI::DepthStencilView::CreateInfo depthStencilCreateInfo;
        depthStencilCreateInfo.Format = GFXI::DepthStencilView::EFormat::D24_UNormInt_S8_UInt;
        depthStencilCreateInfo.Width  = mClientWidth;
        depthStencilCreateInfo.Height = mClientHeight;
        depthStencilCreateInfo.UsedByShader = false;

        mMainWindowDepthStencil = mGfxDevice->CreateDepthStencilView(depthStencilCreateInfo);
        mTransientBufferRegistry = std::make_unique<TransientBufferRegistry>(mGfxDevice, mMainWindowSwapChain->GetRenderTargetView(), mMainWindowDepthStencil);
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
        const bool bRestoreToDefaultState = true;
        {
            auto CreateDefaultSamplerState = [&]()
            {
                if (mDefaultSamplerState == nullptr)
                {
                    GFXI::SamplerState::CreateInfo createInfo;
                    createInfo.SamplingFilter = GFXI::SamplerState::ESamplingFilter::MinLinear_MagLinear_MipLinear;
                    createInfo.BorderColor[0] = 0;
                    createInfo.BorderColor[1] = 0;
                    createInfo.BorderColor[2] = 0;
                    createInfo.BorderColor[3] = 0;
                    mDefaultSamplerState = mGfxDevice->CreateSamplerState(createInfo);
                }
            };

            RenderFrameGraph MainGraph(mTransientBufferRegistry.get());
            RFGRenderPass forwardPass = MainGraph.AddRenderPass("test.forward");
            RFGRenderPass shadowPass = MainGraph.AddRenderPass("test.shadowdepth");
            RFGRenderPass blitPass = MainGraph.AddRenderPass("test.blit");
            RFGResourceHandle renderTarget = MainGraph.RequestResource("test.rendertarget", MainGraph.GetBackbufferWidth(), MainGraph.GetBackbufferHeight(), MainGraph.GetBackbufferRTFormat());
            RFGResourceHandle shadowDepth = MainGraph.RequestResource("test.shadowdepth", 1024, 1024, GFXI::DepthStencilView::EFormat::D24_UNormInt_S8_UInt);
            RFGResourceHandle depthStencil = MainGraph.RequestResource("test.depthstencil", MainGraph.GetBackbufferWidth(), MainGraph.GetBackbufferHeight(), MainGraph.GetBackbufferDSFormat());
            RFGResourceHandle blitRenderTarget = MainGraph.RequestResource("test.blitrendertarget", MainGraph.GetBackbufferWidth(), MainGraph.GetBackbufferHeight(), MainGraph.GetBackbufferRTFormat());

            ClearState state;
            state.ClearDepth = true;
            state.ClearStencil = true;
            state.ClearDepthValue= 1.0;
            state.ClearStencilValue = 0;
            shadowPass.BindWriting(shadowDepth, state);
            shadowPass.AttachJob(
                [&](GFXI::DeferredContext& deviceContext)
                {
                    if (mCubeRenderingQueueShadow == nullptr)
                    {
                        GFXI::ViewportInfo viewport;
                        viewport.X = 0.0f;
                        viewport.Y = 0.0f;
                        viewport.Width =  (float)1024;
                        viewport.Height = (float)1024;
                        viewport.MinDepth = 0.0f;
                        viewport.MaxDepth = 1.0f;

                        mGfxDevice->GetDeferredContext()->StartRecordCommandQueue();
                        mGfxDevice->GetDeferredContext()->SetViewport(viewport);
                        mGfxDevice->GetDeferredContext()->SetGraphicPipelineState(mShadowPassState);
                        RenderScene(scene, deviceContext, data, true);
                        mCubeRenderingQueueShadow = mGfxDevice->GetDeferredContext()->FinishRecordCommandQueue(bRestoreToDefaultState);
                    }
                    mGfxDevice->GetImmediateContext()->ExecuteCommandQueue(mCubeRenderingQueueShadow, bRestoreToDefaultState);
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
                [&](GFXI::DeferredContext& deviceContext)
                {
                    CreateDefaultSamplerState();

                    if (mCubeRenderingQueueFinal == nullptr)
                    {
                        GFXI::ViewportInfo viewport;
                        viewport.X = 0.0f;
                        viewport.Y = 0.0f;
                        viewport.Width =  (float)mClientWidth;
                        viewport.Height = (float)mClientHeight;
                        viewport.MinDepth = 0.0f;
                        viewport.MaxDepth = 1.0f;

                        mGfxDevice->GetDeferredContext()->StartRecordCommandQueue();
                        mGfxDevice->GetDeferredContext()->SetGraphicSamplerStates(GFXI::GraphicPipelineState::EShaderStage::Pixel, 0, 1, &mDefaultSamplerState);
                        mGfxDevice->GetDeferredContext()->SetViewport(viewport);
                        mGfxDevice->GetDeferredContext()->SetGraphicPipelineState(mSimpleDrawPassState);

                        RenderScene(scene, deviceContext, data, false);

                        mCubeRenderingQueueFinal = mGfxDevice->GetDeferredContext()->FinishRecordCommandQueue(bRestoreToDefaultState);
                    }                
                    mGfxDevice->GetImmediateContext()->ExecuteCommandQueue(mCubeRenderingQueueFinal, bRestoreToDefaultState);
                }
            );

            state.ClearColorValue.set(0.0f, 0.0f, 0.0f, 1.0f);
            blitPass.BindReading(renderTarget);
            blitPass.BindWriting(blitRenderTarget, state);
            blitPass.BindWriting(depthStencil, state);
            blitPass.AttachJob(
                [&](GFXI::DeferredContext& deviceContext)
                {
                    CreateDefaultSamplerState();

                    
                    GFXI::ViewportInfo viewport;
                    viewport.X = 0.0f;
                    viewport.Y = 0.0f;
                    viewport.Width =  (float)mClientWidth;
                    viewport.Height = (float)mClientHeight;
                    viewport.MinDepth = 0.0f;
                    viewport.MaxDepth = 1.0f;

                    if(mBlitRenderingQueue == nullptr)
                    {
                        mGfxDevice->GetDeferredContext()->StartRecordCommandQueue();
                        mGfxDevice->GetDeferredContext()->SetViewport(viewport);
                        mGfxDevice->GetDeferredContext()->SetGraphicPipelineState(mBlitPassState);
                        mGfxDevice->GetDeferredContext()->SetGraphicSamplerStates(GFXI::GraphicPipelineState::EShaderStage::Pixel, 0, 1, &mDefaultSamplerState);

                        GFXI::DeviceContext::VertexBufferBinding binding;
                        binding.VertexBuffer = mFullsceenQuadVertices;
                        binding.ElementStride = sizeof(engine::VertexLayout);
                        binding.BufferOffset = 0;
                        mGfxDevice->GetDeferredContext()->SetVertexBuffers(0, 1, &binding);
                        mGfxDevice->GetDeferredContext()->Draw(3, 0);
                        mBlitRenderingQueue = mGfxDevice->GetDeferredContext()->FinishRecordCommandQueue(bRestoreToDefaultState);
                    }
                    mGfxDevice->GetImmediateContext()->ExecuteCommandQueue(mBlitRenderingQueue, bRestoreToDefaultState);
                }
            );
            MainGraph.BindOutput(blitRenderTarget);
            MainGraph.BindOutput(depthStencil);
            MainGraph.Compile();
            MainGraph.Execute(*mGfxDevice->GetDeferredContext());
        }

        mMainWindowSwapChain->Present();
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

    template<typename DataLayout>
    DataLayout* Map(GFXI::DeferredContext& context, GFXI::UniformBuffer* buffer)
    {
        GFXI::DeviceContext::MappedBuffer mb = context.MapBuffer(buffer, GFXI::DeviceContext::EMapMethod::DiscardWrite);
        return reinterpret_cast<DataLayout*>(mb.DataPtr);
    }
    void RenderSystem::RenderScene(Scene& scene, GFXI::DeferredContext& context, const ViewConstantBufferData& data, bool shadow)
    {
        static bool initialize = false;
        static bool initialize_error = false;
        if (!initialize)
        {
            GFXI::UniformBuffer::CreateInfo sceneUBCreateInfo;
            sceneUBCreateInfo.DataUsage = GFXI::EDataUsage::Dynamic;
            sceneUBCreateInfo.BufferSize = sizeof(math::float4x4) * 3 + sizeof(math::float4) * 2;
            mSceneUniformBuffer = mGfxDevice->CreateUniformbuffer(sceneUBCreateInfo);


            GFXI::UniformBuffer::CreateInfo objUBCreateInfo;
            objUBCreateInfo.DataUsage = GFXI::EDataUsage::Dynamic;
            objUBCreateInfo.BufferSize = sizeof(ObjectConstantDesc);
            mObjectUniformBuffer = mGfxDevice->CreateUniformbuffer(objUBCreateInfo);

            GFXI::UniformBuffer::CreateInfo lightViewUBCreateInfo;
            lightViewUBCreateInfo.DataUsage = GFXI::EDataUsage::Dynamic;
            lightViewUBCreateInfo.BufferSize = sizeof(LightViewConstantDesc);
            mLightViewUniformBuffer = mGfxDevice->CreateUniformbuffer(lightViewUBCreateInfo);

            if (mSceneUniformBuffer == nullptr || mObjectUniformBuffer == nullptr || mLightViewUniformBuffer == nullptr)
            {
                initialize_error = true;
                SafeRelease(mSceneUniformBuffer);
                SafeRelease(mObjectUniformBuffer);
                SafeRelease(mLightViewUniformBuffer);
            }
            initialize = true;
        }

        if (initialize && !initialize_error)
        {
            float aspect = mWindowWidth / (float)mWindowHeight;
            math::float2 zNearFar(1.0f, 50.0f);

            // TODO: Vertex and pixel use same constant buffer here.
            math::float4* pConstBuffer = Map<math::float4>(context , mSceneUniformBuffer);
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
            context.UnmapBuffer(mSceneUniformBuffer);
            LightViewConstantDesc* pLightViewConstBuffer = Map<LightViewConstantDesc>(context, mLightViewUniformBuffer);
            {
                //TODO:
                engine::DirectionalLight* pLight = dynamic_cast<engine::DirectionalLight*>(scene.GetDirectionalLight(0));
                pLightViewConstBuffer->MatrixView = pLight->GetViewMatrix();
                pLightViewConstBuffer->MatrixProj = math::ortho_lh_matrix4x4f(20, 20, -50, 50);
            }
            context.UnmapBuffer(mLightViewUniformBuffer);

            GFXI::UniformBuffer* Buffers[2];
            if (shadow)
            {
                Buffers[0] = mLightViewUniformBuffer;
                context.SetGraphicUniformBuffers(GFXI::GraphicPipelineState::EShaderStage::Vertex, 1, 1, Buffers);
            }
            else
            {
                Buffers[0] = mSceneUniformBuffer;
                Buffers[1] = mLightViewUniformBuffer;
                context.SetGraphicUniformBuffers(GFXI::GraphicPipelineState::EShaderStage::Vertex, 1, 2, Buffers);
            }
            context.SetGraphicUniformBuffers(GFXI::GraphicPipelineState::EShaderStage::Pixel, 0, 1, &mSceneUniformBuffer);
            scene.RecursiveRender(context);
        }
    }


    void RenderSystem::SetObjectToWorldMatrixForTest(const math::float4x4& matrix)
    {
        ObjectConstantDesc* pConstBuffer = Map<ObjectConstantDesc>(*mGfxDevice->GetDeferredContext(), mObjectUniformBuffer);
        {
            pConstBuffer->MatrixObjectToWorld = matrix;
            pConstBuffer->MatrixWorldToObject = inversed(matrix);
        }
        mGfxDevice->GetDeferredContext()->UnmapBuffer(mObjectUniformBuffer);
        mGfxDevice->GetDeferredContext()->SetGraphicUniformBuffers(GFXI::GraphicPipelineState::EShaderStage::Vertex, 0, 1, &mObjectUniformBuffer);
    }
}
