#pragma once

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
    enum class GfxInterfaceAPI ECompareFunction : unsigned char
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

    enum class EDataUsage : unsigned char
    {
        Default = 0, Immutable = 1, Dynamic = 2, Staging = 3
    };

    struct GfxInterfaceAPI Buffer : public Object
    {
        enum class EBinding : unsigned char
        {
            Vertex = 0, Index = 1, Uniform = 2
        };

        virtual EBinding        GetBinding() = 0;
        virtual EDataUsage      GetUsage() = 0;
        virtual unsigned int    GetBufferSize() = 0;
    };

    struct GfxInterfaceAPI VertexBuffer : Buffer
    {
        struct CreateInfo
        {
            EDataUsage      DataUsage = EDataUsage::Default;
            unsigned int    BufferSize = 0;
            void*           InitializedBufferDataPtr = nullptr;
        };
    };

    struct GfxInterfaceAPI IndexBuffer : Buffer
    {
        enum class EFormat : unsigned char
        {
            UInt32 = 0, UInt16 = 1,
        };
        struct CreateInfo
        {
            EFormat         DataFormat  = EFormat::UInt32;
            EDataUsage      DataUsage   = EDataUsage::Default;
            unsigned int    BufferSize  = 0;
            void*           InitializedBufferDataPtr = nullptr;
        };

        virtual EFormat     GetIndexFormat() = 0;
    };

    struct GfxInterfaceAPI UniformBuffer : Buffer
    {
        struct CreateInfo
        {
            EDataUsage      DataUsage = EDataUsage::Default;
            unsigned int    BufferSize = 0;
            void*           InitializedBufferDataPtr = nullptr;
        };
    };

    struct GfxInterfaceAPI Texture2D : public Object
    {
        virtual unsigned int    GetWidth() = 0;
        virtual unsigned int    GetHeight() = 0;
        virtual bool            IsUsedByShader() = 0;
    };

    struct GfxInterfaceAPI ShaderResourceView : public Object
    {

    };

    struct GfxInterfaceAPI RenderTargetView : public Texture2D
    {
        enum class EFormat: unsigned char
        {
            R10G10B10A2_UNormInt,
            R8G8B8A8_UNormInt,
        };
        struct CreateInfo
        {
            EFormat         Format          = EFormat::R8G8B8A8_UNormInt;
            EDataUsage      DataUsage       = EDataUsage::Default;
            unsigned int    Width;
            unsigned int    Height;
            bool            UsedByShader    = false;
        };

        virtual EFormat             GetFormat() = 0;
        virtual EDataUsage          GetUsage() = 0;
        virtual ShaderResourceView* GetShaderResourceView() = 0;
    };

    struct GfxInterfaceAPI DepthStencilView : public Texture2D
    {
        enum class EFormat : unsigned char
        {
            D24_UNormInt_S8_UInt,
            D32_SFloat,
        };
        struct CreateInfo
        {
            EFormat         Format          = EFormat::D24_UNormInt_S8_UInt;
            unsigned int    Width;
            unsigned int    Height;
            bool            UsedByShader    = false;
        };

        virtual EFormat             GetFormat() = 0;
        virtual EDataUsage          GetUsage() = 0;
        virtual ShaderResourceView* GetShaderResourceView() = 0;
    };

    enum class GfxInterfaceAPI EShaderType : unsigned char
    {
        VertexShader    = 0,
        PixelShader     = 1,
        GeometryShader  = 2,
        DomainShader    = 3,
        HullShader      = 4,
        ComputeShader   = 5,
    };

    struct GfxInterfaceAPI ShaderBinary : public Object
    {
        struct CreateInfo
        {
            EShaderType ShaderType              = EShaderType::VertexShader;
            const char* ShaderName              = nullptr;
            const char* EntryNameString         = nullptr;
            const void* ShaderSourceCodeData    = nullptr;
            unsigned int ShaderSourceCodeLength = 0;
            //Includes
            //Definitions
        };

        virtual EShaderType     GetShaderType() = 0;
        virtual unsigned int    GetBytecodeLength() = 0;
        virtual void*           GetBytecode() = 0;
    };

    struct GfxInterfaceAPI Shader : public Object
    {
        struct CreateInfo
        {
            ShaderBinary* ShaderBinary;
            //unsigned int CodeSize;
            //unsigned char* CodeDataPtr;
        };

        virtual EShaderType     GetShaderType() = 0;
        virtual unsigned int    GetBytecodeLength() = 0;
        virtual void*           GetBytecode() = 0;

        virtual void* GetRawHandle() = 0;
    };

    struct GfxInterfaceAPI CommandQueue : public Object
    {

    };

    struct GfxInterfaceAPI GraphicPipelineState : public Object
    {
        static const int kNumShaderStage = 5;
        enum class EShaderStage : unsigned char
        {
            Vertex      = 0,
            Pixel       = 1,
            Geometry    = 2,
            Domain      = 3,
            Hull        = 4,
        };
        enum class EPrimitiveTopology : unsigned char
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
            enum class EInputRate : unsigned char
            {
                Vertex = 0, Instance = 1
            };
            enum class EVertexFormat : unsigned char
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
            struct InputElement
            {
                EInputRate      InputRate       = EInputRate::Vertex;
                EVertexFormat   VertexFormat    = EVertexFormat::RGBA32_SFloat;
                const char*     SemanticName;
                unsigned int    SemanticIndex   = 0;
                unsigned int    BingdingIndex   = 0;
                unsigned int    Offset          = 0;
                unsigned int    InstancedRate   = 0;
            };

            int ElementCount = 0;
            InputElement* ElementArray;
        };
        struct ColorBlendState
        {
            enum class EBlendOp : unsigned char
            {
                Add     = 0,
                Sub     = 1,
                InvSub  = 2,
                Min     = 3,
                Max     = 4
            };
            enum class EBlendFactor : unsigned char
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
                EBlendFactor    ColorSrcFactor;
                EBlendFactor    ColorDstFactor;
                EBlendFactor    AlphaSrcFactor;
                EBlendFactor    AlphaDstFactor;
            };

            int AttachmentCount = 0;
            AttachmentBlendState AttachmentBlendStates[8];
            float BlendConstant[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        };
        struct DepthStencilState
        {
            enum class EStencilOp : unsigned char
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
                ECompareFunction StencilTestFunction    = ECompareFunction::Never;
                EStencilOp StenciFailOp                 = EStencilOp::Zero;
                EStencilOp DepthFailOp                  = EStencilOp::Zero;
                EStencilOp StencilPassOp                = EStencilOp::Zero;
            };

            bool UsingDepthTest     = true;
            bool EnableDepthWrite   = true;
            bool UsingStencilTest   = false;
            ECompareFunction    DepthTestFunction   = ECompareFunction::LessEqual;
            unsigned char       StencilReadMask     = 0xFF;
            unsigned char       StencilWriteMask    = 0xFF;
            StencilDesc         StencilFrontFace;
            StencilDesc         StencilBackFace;
        };
        struct RasterizerState
        {
            enum class EFillMode : unsigned char
            {
                Solid = 0,
                Wireframe = 1
            };
            enum class ECullMode : unsigned char
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

            EFillMode   FillMode = EFillMode::Solid;
            ECullMode   CullMode = ECullMode::Backface;
            EFrontFace  Frontface = EFrontFace::CounterClockwise;
            int         DepthBias = 0;
            float       DepthBiasClamp = 0.0f;
            float       SlopeScaledDepthBias = 0.0f;
            bool        UseScissorClip = false;
            bool        UseAntialiasedline = false;
        };
        struct CreateInfo
        {
            Shader*             StageShaders[kNumShaderStage] = { 0, 0, 0, 0, 0 };
            EPrimitiveTopology  PrimitiveTopology = EPrimitiveTopology::TriangleList;
            VertexInputLayout   VertexInputLayout;
            ColorBlendState     ColorBlendState;
            DepthStencilState   DepthStencilState;
            RasterizerState     RasterizerState;
        };
    };

    struct GfxInterfaceAPI ComputePipelineState : public Object
    {
        struct CreateInfo
        {

        };
    };

    struct GfxInterfaceAPI ViewportInfo
    {
        float X         = 0.0f;
        float Y         = 0.0f;
        float Width     = 1.0f;
        float Height    = 1.0f;
        float MinDepth  = 0.0f;
        float MaxDepth  = 1.0;
    };

    struct GfxInterfaceAPI SamplerState : public Object
    {
        enum class EFilterMethod : unsigned char
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
            static const unsigned int Value = ((unsigned int)_Min) | (((unsigned int)_Mag) << 2) | (((unsigned int)_Mip) << 4) | ((_Comparison ? 0x1 : 0x0) << 6) | ((_Anisotropic ? 0x1 : 0x0) << 8);
        };
        enum class ESamplingFilter : unsigned int
        {
            MinPoint_MagPoint_MipPoint      = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Point,   EFilterMethod::Point>::Value,
            MinPoint_MagPoint_MipLinear     = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Point,   EFilterMethod::Linear>::Value,
            MinPoint_MagLinear_MipPoint     = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Linear,  EFilterMethod::Point>::Value,
            MinPoint_MagLinear_MipLinear    = SamplingFilterDesc<EFilterMethod::Point,  EFilterMethod::Linear,  EFilterMethod::Linear>::Value,
            MinLinear_MagPoint_MipPoint     = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Point,   EFilterMethod::Point>::Value,
            MinLinear_MagPoint_MipLinear    = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Point,   EFilterMethod::Linear>::Value,
            MinLinear_MagLinear_MipPoint    = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Linear,  EFilterMethod::Point>::Value,
            MinLinear_MagLinear_MipLinear   = SamplingFilterDesc<EFilterMethod::Linear, EFilterMethod::Linear,  EFilterMethod::Linear>::Value,
            Anisotropic                     = SamplingFilterDesc<EFilterMethod::None,   EFilterMethod::None,    EFilterMethod::None, true>::Value
        };
        enum class ETextureAddressMode : unsigned char
        {
            Wrap = 0, Clamp = 1, Mirror = 2, Border = 3, MirrorOnce = 5
        };
        struct CreateInfo
        {
            ESamplingFilter SamplingFilter          = ESamplingFilter::MinLinear_MagLinear_MipLinear;
            ETextureAddressMode SamplingAddressU    = ETextureAddressMode::Clamp;
            ETextureAddressMode SamplingAddressV    = ETextureAddressMode::Clamp;
            ETextureAddressMode SamplingAddressW    = ETextureAddressMode::Clamp;
            unsigned int MaxAnisotropy              = 1;
            ECompareFunction ComparisonFunction     = ECompareFunction::Always;
            float MinLOD        =  0.0f;
            float MaxLOD        = 32.0f;
            float MipLODBias    =  0.0f;
            float BorderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        };
    };

    struct GfxInterfaceAPI DeviceContext : public Object
    {
        enum class EMapMethod : unsigned char
        {
            Read            = 1,
            Write           = 2,
            ReadAndWrite    = 3,
            DiscardWrite    = 4,        //VB:yes, IB:yes, UB:yes, Tex:yes, 
            WriteWithoutOverwrite = 5,  //VB:yes, IB:yes, UB:no, DynamicTexture:no
        };
        struct VertexBufferBinding
        {
            VertexBuffer* VertexBuffer = nullptr;
            unsigned int ElementStride = 0;
            unsigned int BufferOffset = 0;
        };
        struct MappedBuffer
        {
            void* DataPtr = nullptr;
            unsigned int RowPitch = 0;
        };

        virtual void SetViewport(const ViewportInfo&) = 0;
        virtual void SetViewports(unsigned int count, const ViewportInfo*) = 0;
        virtual void SetGraphicPipelineState(GraphicPipelineState*) = 0;
        virtual void SetStencilReferenceValue(unsigned int) = 0;
        virtual void SetGraphicUniformBuffers(GraphicPipelineState::EShaderStage, unsigned int startSlot, unsigned int count, UniformBuffer**) = 0;
        virtual void SetGraphicShaderResources(GraphicPipelineState::EShaderStage, unsigned int startSlot, unsigned int count, ShaderResourceView**) = 0;
        virtual void SetGraphicSamplerStates(GraphicPipelineState::EShaderStage, unsigned int startSlot, unsigned int count, SamplerState**) = 0;
        virtual void SetVertexBuffers(unsigned int startSlot, unsigned int bufferBindingCount, const VertexBufferBinding* bufferBindings) = 0;
        virtual void SetIndexBuffer(IndexBuffer*, unsigned int offset) = 0;
        virtual void SetRenderTargets(unsigned int count, RenderTargetView** rts, DepthStencilView* ds) = 0;
        virtual void Draw(unsigned int vertexCount, unsigned int startVertexLocation) = 0;
        virtual void DrawIndexed(unsigned int indexCount, unsigned int startIndexLocation, unsigned int baseVertexOffset) = 0;
        virtual void ClearRenderTarget(RenderTargetView* renderTarget, const float clearColor[4]) = 0;
        virtual void ClearDepthBuffer(DepthStencilView* ds, float depth) = 0;
        virtual void ClearStencilBuffer(DepthStencilView* ds, unsigned char stencil) = 0;
        virtual void ClearDepthStencil(DepthStencilView* ds, float depth, unsigned char stencil) = 0;
        virtual void ClearDepthStencil(DepthStencilView* ds, bool d, bool s, float dv, unsigned char sv) = 0;
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
        virtual void StartRecordCommandQueue() = 0;
        virtual CommandQueue* FinishRecordCommandQueue(bool bRestoreToDefaultState) = 0;
    };

    struct GfxInterfaceAPI SwapChain : public Object
    {
        virtual RenderTargetView* GetRenderTargetView() = 0;
        virtual void Present() = 0;
    };

    struct GfxInterfaceAPI GraphicDevice : public Object
    {
        virtual SwapChain*              CreateSwapChain(void* WindowHandle, int WindowWidth, int WindowHeight, bool IsFullscreen) = 0;
        virtual GraphicPipelineState*   CreateGraphicPipelineState(const GraphicPipelineState::CreateInfo&) = 0;
        virtual ComputePipelineState*   CreateComputePipelineState(const ComputePipelineState::CreateInfo&) = 0;
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
        virtual bool IsHardwareSupported() = 0;
        virtual GraphicDevice* CreateDevice() = 0;
    };
}

extern "C" GfxInterfaceAPI GFXI::GraphicModule * CreateGfxModule();
