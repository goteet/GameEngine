#pragma once
#include <cstdint>

// clang-format off
#ifdef _MSC_VER
    #ifdef GFX_RHI_EXPORT
        #define GfxInterfaceAPI __declspec(dllexport)
    #else
        #define GfxInterfaceAPI __declspec(dllimport)
    #endif
#else
    #ifdef GFX_RHI_EXPORT
        #define GfxInterfaceAPI __attribute__ ((visibility ("default"))) 
    #else
        #define GfxInterfaceAPI 
    #endif
#endif
// clang-format on

namespace GFXI
{
    enum class GfxInterfaceAPI ECompareFunction : uint8_t
    {
        Never           = 0,
        Always          = 1,
        Equal           = 2,
        NotEqual        = 3,
        Less            = 4,
        LessEqual       = 5,
        Greater         = 6,
        GreaterEqual    = 7,
    };

    struct GfxInterfaceAPI Object
    {
        virtual ~Object() = default;
        virtual void Release() = 0;
    };

    enum class EDataUsage : uint8_t
    {
        Default = 0, Immutable = 1, Dynamic = 2, Staging = 3
    };

    struct GfxInterfaceAPI Buffer : public Object
    {
        enum class EBinding : uint8_t
        {
            Vertex = 0, Index = 1, Uniform = 2
        };

        virtual EBinding    GetBinding() = 0;
        virtual EDataUsage  GetUsage() = 0;
        virtual uint32_t    GetBufferSize() = 0;
    };

    struct GfxInterfaceAPI VertexBuffer : Buffer
    {
        struct CreateInfo
        {
            EDataUsage  DataUsage = EDataUsage::Default;
            uint32_t    BufferSize = 0;
            void*       InitializedBufferDataPtr = nullptr;
        };
    };

    struct GfxInterfaceAPI IndexBuffer : Buffer
    {
        enum class EFormat : uint8_t
        {
            UInt32 = 0, UInt16 = 1,
        };
        struct CreateInfo
        {
            EFormat     DataFormat  = EFormat::UInt32;
            EDataUsage  DataUsage   = EDataUsage::Default;
            uint32_t    BufferSize  = 0;
            void*       InitializedBufferDataPtr = nullptr;
        };

        virtual EFormat     GetIndexFormat() = 0;
    };

    struct GfxInterfaceAPI UniformBuffer : Buffer
    {
        struct CreateInfo
        {
            EDataUsage  DataUsage = EDataUsage::Default;
            uint32_t    BufferSize = 0;
            void*       InitializedBufferDataPtr = nullptr;
        };
    };

    struct GfxInterfaceAPI Texture2D : public Object
    {
        virtual uint32_t    GetWidth() = 0;
        virtual uint32_t    GetHeight() = 0;
        virtual bool        IsUsedByShader() = 0;
    };

    struct GfxInterfaceAPI ShaderResourceView : public Object
    {

    };

    struct GfxInterfaceAPI RenderTargetView : public Texture2D
    {
        enum class EFormat: uint8_t
        {
            R10G10B10A2_UNormInt,
            R8G8B8A8_UNormInt,
        };
        struct CreateInfo
        {
            EFormat     Format          = EFormat::R8G8B8A8_UNormInt;
            EDataUsage  DataUsage       = EDataUsage::Default;
            uint32_t    Width;
            uint32_t    Height;
            bool        UsedByShader    = false;
        };

        virtual EFormat             GetFormat() = 0;
        virtual EDataUsage          GetUsage() = 0;
        virtual ShaderResourceView* GetShaderResourceView() = 0;
    };

    struct GfxInterfaceAPI DepthStencilView : public Texture2D
    {
        enum class EFormat : uint8_t
        {
            D24_UNormInt_S8_UInt,
            D32_SFloat,
        };
        struct CreateInfo
        {
            EFormat     Format          = EFormat::D24_UNormInt_S8_UInt;
            uint32_t    Width;
            uint32_t    Height;
            bool        UsedByShader    = false;
        };

        virtual EFormat             GetFormat() = 0;
        virtual EDataUsage          GetUsage() = 0;
        virtual ShaderResourceView* GetShaderResourceView() = 0;
    };

    enum class GfxInterfaceAPI EShaderType : uint32_t
    {
        VertexShader    = 0,
        PixelShader     = 1,
        GeometryShader  = 2,
        DomainShader    = 3,
        HullShader      = 4,
        ComputeShader   = 5,
        Num = 6
    };

    struct GfxInterfaceAPI ShaderBinary : public Object
    {
        struct CreateInfo
        {
            EShaderType ShaderType              = EShaderType::VertexShader;
            const char* ShaderNameString        = nullptr;
            const char* EntryNameString         = nullptr;
            const void* ShaderSourceCodeData    = nullptr;
            uint32_t    ShaderSourceCodeLength  = 0;
            //Includes
            //Definitions
        };

        virtual EShaderType GetShaderType() = 0;
        virtual const char* GetShaderName() = 0;
        virtual const char* GetEntryPointName() = 0;
        virtual void*       GetBytecode() = 0;
        virtual uint32_t    GetBytecodeLength() = 0;
    };

    struct GfxInterfaceAPI Shader : public Object
    {
        struct CreateInfo
        {
            ShaderBinary* ShaderBinary;
        };

        virtual EShaderType GetShaderType() = 0;
        virtual const char* GetShaderName() = 0;
        virtual const char* GetEntryPointName() = 0;
        virtual void*       GetBytecode() = 0;
        virtual uint32_t    GetBytecodeLength() = 0;
    };


    struct GfxInterfaceAPI DescriptorSetLayout : public Object
    {
        enum class EDescriptorType : uint32_t
        {
            Sampler = 0,
            CombinedImageSampler = 1,
            UniformBuffer = 2,
            StorageBuffer = 3,
            UniformBufferDynamic = 4,
            StorageBufferDynamic = 5,
        };

        enum EShaderStageFlags : uint32_t
        {
            VertexShaderStageBits   = 0x00000001,
            PixelShaderStageBits    = 0x00000002,
            GeometryShaderStageBits = 0x00000004,
            DomainShaderStageBits   = 0x00000008,
            HullShaderStageBits     = 0x00000010,
            ComputeShaderStageBits  = 0x00000020,

            BasicGraphicsStagesBits = VertexShaderStageBits | PixelShaderStageBits,
            AllGraphicsStagesBits   = VertexShaderStageBits | PixelShaderStageBits | GeometryShaderStageBits | DomainShaderStageBits | HullShaderStageBits,
            AllStageBits            = 0x7FFFFFFF,
        };
        struct CreateInfo
        {
            struct DescriptorDesc
            {
                EDescriptorType DescriptorType = EDescriptorType::UniformBuffer;
                uint32_t        NumDescriptors = 1;
                uint32_t        ShaderStageFlags = 0;
            };

            uint32_t        NumDescriptorBindings   = 0;
            DescriptorDesc* DescriptorBindings      = nullptr;
        };


    };

    struct GfxInterfaceAPI CommandQueue : public Object
    {

    };

    struct GfxInterfaceAPI ViewportInfo
    {
        float X        = 0.0f;
        float Y        = 0.0f;
        float Width    = 1.0f;
        float Height   = 1.0f;
        float MinDepth = 0.0f;
        float MaxDepth = 1.0;
    };

    struct GfxInterfaceAPI GraphicPipelineState : public Object
    {
        enum class EShaderStage : uint8_t
        {
            Vertex      = 0,
            Pixel       = 1,
            Geometry    = 2,
            Domain      = 3,
            Hull        = 4,
            Num         = 5
        };
        enum class EPrimitiveTopology : uint8_t
        {
            PointList               = 0,
            LineList                = 1,
            TriangleList            = 2,
            TriangleStrip           = 3,
            AdjacentLineListList    = 4,
            AdjacentTriangleList    = 5,
            AdjacentTriangleStrip   = 6
        };
        struct VertexInputLayout
        {
            enum class EInputRate : uint8_t
            {
                Vertex = 0, Instance = 1
            };
            enum class EVertexFormat : uint8_t
            {
                RGBA32_SFloat,
                RGBA32_UInt,
                RGBA32_SInt,
                RGB32_SFloat,
                RGB32_UInt,
                RGB32_SInt,
                RG32_SFloat,
                RG32_UInt,
                RG32_SInt,
            };
            struct Binding
            {
                EInputRate      InputRate   = EInputRate::Vertex;
                uint32_t        Index       = 0;
                uint32_t        Stride      = 0;
            };
            struct Attribute
            {
                EVertexFormat   VertexFormat    = EVertexFormat::RGBA32_SFloat;
                const char*     SemanticName;
                uint32_t        SemanticIndex   = 0;
                uint32_t        BingdingIndex   = 0;
                uint32_t        LocationIndex   = 0;
                uint32_t        VertexOffset    = 0;
            };
            
            uint32_t    NumBindingArray     = 0;
            uint32_t    NumAttributeArray   = 0;
            Binding*    BindingArray        = nullptr;
            Attribute*  AttributeArray      = nullptr;
        };
        struct ColorBlendState
        {
            enum class EBlendOp : uint8_t
            {
                Add     = 0,
                Sub     = 1,
                InvSub  = 2,
                Min     = 3,
                Max     = 4
            };
            enum class EBlendFactor : uint8_t
            {
                Zero                = 0,
                One                 = 1,
                SrcColor            = 2,
                DstColor            = 3,
                Constant            = 4,
                OneMinusSrcColor    = 5,
                OneMinusDstColor    = 6,
                OneMinusConstant    = 7,
                SrcAlpha            = 8,
                DstAlpha            = 9,
                OneMinusSrcAlpha    = 10,
                OneMinusDstAlpha    = 11,
            };
            struct AttachmentBlendState
            {
                bool            UsingBlend      = true;
                bool            ColorWriteR     = true;
                bool            ColorWriteG     = true;
                bool            ColorWriteB     = true;
                bool            ColorWriteA     = true;
                EBlendOp        ColorBlendOp    = EBlendOp::Add;
                EBlendOp        AlphaBlendOp    = EBlendOp::Add;
                EBlendFactor    ColorSrcFactor  = EBlendFactor::Zero;
                EBlendFactor    ColorDstFactor  = EBlendFactor::One;
                EBlendFactor    AlphaSrcFactor  = EBlendFactor::Zero;
                EBlendFactor    AlphaDstFactor  = EBlendFactor::One;
            };

            uint32_t                NumAttachments = 0;
            AttachmentBlendState    AttachmentBlendStates[8];
            float                   BlendConstant[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        };
        struct DepthStencilState
        {
            enum class EStencilOp : uint8_t
            {
                Zero            = 0,
                Keep            = 1,
                Replace         = 2,
                IncreaseClamp   = 3,
                IncreaseWrap    = 4,
                DecraseClamp    = 5,
                DecraseWrap     = 6,
                Invert          = 7
            };
            struct StencilDesc
            {
                ECompareFunction    StencilTestFunction = ECompareFunction::Never;
                EStencilOp          StenciFailOp        = EStencilOp::Zero;
                EStencilOp          DepthFailOp         = EStencilOp::Zero;
                EStencilOp          StencilPassOp       = EStencilOp::Zero;
            };

            bool                UsingDepthTest      = true;
            bool                EnableDepthWrite    = true;
            bool                UsingStencilTest    = false;
            ECompareFunction    DepthTestFunction   = ECompareFunction::LessEqual;
            uint8_t             StencilReadMask     = 0xFF;
            uint8_t             StencilWriteMask    = 0xFF;
            uint8_t             StencilReference    = 0;
            StencilDesc         StencilFrontFace;
            StencilDesc         StencilBackFace;
        };
        struct RasterizationState
        {
            enum class EFillMode : uint8_t
            {
                Solid = 0,
                Wireframe = 1
            };
            enum class ECullMode : uint8_t
            {
                None = 0,
                Frontface = 1,
                Backface = 2
            };
            enum class EFrontFace
            {
                CounterClockwise = 0,
                Clockwise = 1
            };

            EFillMode       FillMode                = EFillMode::Solid;
            ECullMode       CullMode                = ECullMode::Backface;
            EFrontFace      Frontface               = EFrontFace::CounterClockwise;
            int32_t         DepthBias               = 0;
            float           DepthBiasClamp          = 0.0f;
            float           SlopeScaledDepthBias    = 0.0f;
            bool            UseScissorClip          = false;
            bool            UseAntialiasedline      = false;
            ViewportInfo    ViewportInfo;
        };
        struct ShaderModuleDesc
        {
            Shader* StageShaders[(uint32_t)EShaderStage::Num] = { nullptr, nullptr, nullptr, nullptr, nullptr };
            uint32_t NumDescriptorSetLayouts = 0;
            DescriptorSetLayout** DescriptorSetLayouts = nullptr;
        };
        struct CreateInfo
        {
            ShaderModuleDesc    ShaderModuleDesc;
            EPrimitiveTopology  PrimitiveTopology = EPrimitiveTopology::TriangleList;
            VertexInputLayout   VertexInputLayout;
            ColorBlendState     ColorBlendState;
            DepthStencilState   DepthStencilState;
            RasterizationState  RasterizationState;
        };
    };

    struct GfxInterfaceAPI ComputePipelineState : public Object
    {
        struct CreateInfo
        {

        };
    };

    struct GfxInterfaceAPI SamplerState : public Object
    {
        enum class EFilterMethod : uint8_t
        {
            Point = 0, Linear = 1, None = 2
        };
        template <EFilterMethod _Min, EFilterMethod _Mag, EFilterMethod _Mip, bool _Anisotropic = false, bool _Comparison = false>
        struct SamplingFilterDesc
        {
            static const EFilterMethod Min  = _Anisotropic ? EFilterMethod::None : _Min;
            static const EFilterMethod Mag  = _Anisotropic ? EFilterMethod::None : _Mag;
            static const EFilterMethod Mip  = _Anisotropic ? EFilterMethod::None : _Mip;
            static const bool IsAnisotropic = _Anisotropic;
            static const bool IsComparison  = _Comparison;
            static const uint32_t Value = ((uint32_t)_Min) | (((uint32_t)_Mag) << 2) | (((uint32_t)_Mip) << 4) | ((_Comparison ? 0x1 : 0x0) << 6) | ((_Anisotropic ? 0x1 : 0x0) << 8);
        };
        enum class ESamplingFilter : uint32_t
        {
            MinPoint_MagPoint_MipPoint      = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Point,   EFilterMethod::Point,   false>::Value,
            MinPoint_MagPoint_MipLinear     = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Point,   EFilterMethod::Linear,  false>::Value,
            MinPoint_MagLinear_MipPoint     = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Linear,  EFilterMethod::Point,   false>::Value,
            MinPoint_MagLinear_MipLinear    = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Linear,  EFilterMethod::Linear,  false>::Value,
            MinLinear_MagPoint_MipPoint     = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Point,   EFilterMethod::Point,   false>::Value,
            MinLinear_MagPoint_MipLinear    = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Point,   EFilterMethod::Linear,  false>::Value,
            MinLinear_MagLinear_MipPoint    = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Linear,  EFilterMethod::Point,   false>::Value,
            MinLinear_MagLinear_MipLinear   = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Linear,  EFilterMethod::Linear,  false>::Value,
            Anisotropic                     = SamplingFilterDesc<EFilterMethod::None,   EFilterMethod::None,    EFilterMethod::None,    true >::Value
        };
        enum class ETextureAddressMode : uint8_t
        {
            Wrap = 0, Clamp = 1, Mirror = 2, Border = 3, MirrorOnce = 5
        };
        struct CreateInfo
        {
            ESamplingFilter     SamplingFilter      = ESamplingFilter::MinLinear_MagLinear_MipLinear;
            ETextureAddressMode SamplingAddressU    = ETextureAddressMode::Clamp;
            ETextureAddressMode SamplingAddressV    = ETextureAddressMode::Clamp;
            ETextureAddressMode SamplingAddressW    = ETextureAddressMode::Clamp;
            uint32_t            MaxAnisotropy       = 1;
            ECompareFunction    ComparisonFunction  = ECompareFunction::Always;
            float               MinLOD              = 0.0f;
            float               MaxLOD              = 32.0f;
            float               MipLODBias          = 0.0f;
            float               BorderColor[4]      = { 1.0f, 1.0f, 1.0f, 1.0f };
        };
    };

    struct GfxInterfaceAPI DeviceContext : public Object
    {
        enum class EMapMethod : uint8_t
        {
            Read            = 1,
            Write           = 2,
            ReadAndWrite    = 3,
            DiscardWrite    = 4,        //VB:yes, IB:yes, UB:yes, Tex:yes, 
            WriteWithoutOverwrite = 5,  //VB:yes, IB:yes, UB:no, DynamicTexture:no
        };
        struct VertexBufferBinding
        {
            VertexBuffer*   VertexBuffer    = nullptr;
            uint32_t        ElementStride   = 0;
            uint32_t        BufferOffset    = 0;  //offset from first element in buffer array.
        };
        struct MappedBuffer
        {
            void*       DataPtr     = nullptr;
            uint32_t    RowPitch    = 0;
        };

        virtual void SetViewport(const ViewportInfo&) = 0;
        virtual void SetViewports(uint32_t count, const ViewportInfo*) = 0;
        virtual void SetGraphicPipelineState(GraphicPipelineState*) = 0;
        virtual void SetStencilReferenceValue(uint32_t) = 0;
        virtual void SetGraphicUniformBuffers(GraphicPipelineState::EShaderStage, uint32_t startSlot, uint32_t count, UniformBuffer**) = 0;
        virtual void SetGraphicShaderResources(GraphicPipelineState::EShaderStage, uint32_t startSlot, uint32_t count, ShaderResourceView**) = 0;
        virtual void SetGraphicSamplerStates(GraphicPipelineState::EShaderStage, uint32_t startSlot, uint32_t count, SamplerState**) = 0;
        virtual void SetVertexBuffers(uint32_t startSlot, uint32_t bufferBindingCount, const VertexBufferBinding* bufferBindings) = 0;
        virtual void SetIndexBuffer(IndexBuffer*, uint32_t offset) = 0;
        virtual void SetRenderTargets(uint32_t count, RenderTargetView** rts, DepthStencilView* ds) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexOffset) = 0;
        virtual void ClearRenderTarget(RenderTargetView* renderTarget, const float clearColor[4]) = 0;
        virtual void ClearDepthBuffer(DepthStencilView* ds, float depth) = 0;
        virtual void ClearStencilBuffer(DepthStencilView* ds, uint8_t stencil) = 0;
        virtual void ClearDepthStencil(DepthStencilView* ds, float depth, uint8_t stencil) = 0;
        virtual void ClearDepthStencil(DepthStencilView* ds, bool d, bool s, float dv, uint8_t sv) = 0;
        virtual MappedBuffer MapBuffer(Buffer* buffer, EMapMethod mapMethod) = 0;
        virtual void UnmapBuffer(Buffer* buffer) = 0;
        virtual void UpdateBuffer(Buffer*, const void* data) = 0;
    };

    struct GfxInterfaceAPI ImmediateContext : public DeviceContext
    {
        //virtual void FillEntireBuffer(Buffer*, const void* dataPtr) = 0;
        virtual void ExecuteCommandQueue(CommandQueue*, bool bRestoreToDefaultState) = 0;
    };

    struct GfxInterfaceAPI DeferredContext : public DeviceContext
    {
        virtual void            StartRecordCommandQueue() = 0;
        virtual CommandQueue*   FinishRecordCommandQueue(bool bRestoreToDefaultState) = 0;
    };

    struct GfxInterfaceAPI SwapChain : public Object
    {
        virtual RenderTargetView*   GetRenderTargetView() = 0;
        virtual void                Present() = 0;
    };

    struct GfxInterfaceAPI GraphicDevice : public Object
    {
        virtual SwapChain*              CreateSwapChain(void* WindowHandle, int32_t WindowWidth, int32_t WindowHeight, bool IsFullscreen) = 0;
        virtual GraphicPipelineState*   CreateGraphicPipelineState(const GraphicPipelineState::CreateInfo&) = 0;
        virtual ComputePipelineState*   CreateComputePipelineState(const ComputePipelineState::CreateInfo&) = 0;
        virtual DescriptorSetLayout*    CreateDescriptorSetLayout(const DescriptorSetLayout::CreateInfo&) = 0;
        virtual SamplerState*           CreateSamplerState(const SamplerState::CreateInfo&) = 0;
        virtual ShaderBinary*           CompileShader(const ShaderBinary::CreateInfo&) = 0;
        virtual Shader*                 CreateShader(const Shader::CreateInfo&) = 0;
        virtual VertexBuffer*           CreateVertexBuffer(const VertexBuffer::CreateInfo&) = 0;
        virtual IndexBuffer*            CreateIndexBuffer(const IndexBuffer::CreateInfo&) = 0;
        virtual UniformBuffer*          CreateUniformbuffer(const UniformBuffer::CreateInfo&) = 0;
        virtual RenderTargetView*       CreateRenderTargetView(const RenderTargetView::CreateInfo&) = 0;
        virtual DepthStencilView*       CreateDepthStencilView(const DepthStencilView::CreateInfo&) = 0;
        virtual ImmediateContext*       GetImmediateContext() = 0;
        virtual DeferredContext*        GetDeferredContext() = 0;
    };

    struct GfxInterfaceAPI GraphicModule : public Object
    {
        virtual bool            IsHardwareSupported() = 0;
        virtual GraphicDevice*  CreateDevice() = 0;
    };
}

extern "C" GfxInterfaceAPI GFXI::GraphicModule * CreateGfxModule();
