#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct ModuleD3D11;

    struct GraphicPipelineStateD3D11 : public GraphicPipelineState
    {
        GraphicPipelineStateD3D11(
            const D3D11_PRIMITIVE_TOPOLOGY& primitiveTopology, ID3D11InputLayout*,
            const D3D11_DEPTH_STENCIL_DESC&,    ID3D11DepthStencilState*,
            const D3D11_BLEND_DESC&,            ID3D11BlendState*, const float blendConstants[4],
            const D3D11_RASTERIZER_DESC&,       ID3D11RasterizerState*,
            ComPtr<ID3D11VertexShader> vertexShader,
            ComPtr<ID3D11PixelShader> pixelShader,
            ComPtr<ID3D11GeometryShader> geometryShader,
            ComPtr<ID3D11DomainShader> domainShader,
            ComPtr<ID3D11HullShader> hulllShader
        );
        virtual ~GraphicPipelineStateD3D11();
        virtual void Release() override;

        void SetupContext(ID3D11DeviceContext* context, unsigned int stencilValue);

    private:
        D3D11_PRIMITIVE_TOPOLOGY        mPrimitiveTopology;
        D3D11_DEPTH_STENCIL_DESC        mDepthStencilStateDesc;
        D3D11_BLEND_DESC                mColorBlendStateDesc;
        D3D11_RASTERIZER_DESC           mRasterizerStateDesc;

        ComPtr<ID3D11InputLayout>       mInputLayout;
        ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        ComPtr<ID3D11BlendState>        mColorBlendState;
        ComPtr<ID3D11RasterizerState>   mRasterizerState;
        float           mBlendConstants[4];

        ComPtr<ID3D11VertexShader>      mVertexShader;
        ComPtr<ID3D11PixelShader>       mPixelShader;
        ComPtr<ID3D11GeometryShader>    mGeometryShader;
        ComPtr<ID3D11DomainShader>      mDomainShader;
        ComPtr<ID3D11HullShader>        mHullShader;
    };
}
