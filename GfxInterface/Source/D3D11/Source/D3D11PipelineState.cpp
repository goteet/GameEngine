#include "D3D11PipelineState.h"


namespace GFXI
{
    GraphicPipelineStateD3D11::GraphicPipelineStateD3D11(
        const D3D11_PRIMITIVE_TOPOLOGY& primitiveTopology,      ID3D11InputLayout* inputLayout,
        const D3D11_DEPTH_STENCIL_DESC& depthStencilStateDesc,  ID3D11DepthStencilState* depthStencilState,
        const D3D11_BLEND_DESC& colorBlendStateDesc,            ID3D11BlendState* colorBlendState, const float blendConstants[4],
        const D3D11_RASTERIZER_DESC& rasterizerStateDesc,       ID3D11RasterizerState* rasterizerState,
        ComPtr<ID3D11VertexShader>      vertexShader,
        ComPtr<ID3D11PixelShader>       pixelShader,
        ComPtr<ID3D11GeometryShader>    geometryShader,
        ComPtr<ID3D11DomainShader>      domainShader,
        ComPtr<ID3D11HullShader>        hulllShader
    )
        : mPrimitiveTopology(primitiveTopology)
        , mInputLayout(inputLayout)
        , mDepthStencilStateDesc(depthStencilStateDesc)
        , mColorBlendStateDesc(colorBlendStateDesc)
        , mRasterizationStateDesc(rasterizerStateDesc)
        , mDepthStencilState(depthStencilState)
        , mColorBlendState(colorBlendState)
        , mRasterizationState(rasterizerState)
        , mVertexShader(vertexShader)
        , mPixelShader(pixelShader)
        , mGeometryShader(geometryShader)
        , mDomainShader(domainShader)
        , mHullShader(hulllShader)
    {
        for (int index = 0; index < 4; index++)
        {
            mBlendConstants[index] = blendConstants[index];
        }
    }

    GraphicPipelineStateD3D11::~GraphicPipelineStateD3D11()
    {
        mVertexShader.Reset();
        mPixelShader.Reset();
        mGeometryShader.Reset();
        mDomainShader.Reset();
        mHullShader.Reset();

        mInputLayout.Reset();
        mDepthStencilState.Reset();
        mColorBlendState.Reset();
        mRasterizationState.Reset();
    }

    void GraphicPipelineStateD3D11::Release()
    {
        delete this;
    }
    void GraphicPipelineStateD3D11::SetupContext(ID3D11DeviceContext* context, unsigned int stencilValue)
    {
        UINT kDefaultSampleCoverageMask = 0xFFFFFFFF;

        context->VSSetShader(mVertexShader.Get(), nullptr, 0);
        context->PSSetShader(mPixelShader.Get(), nullptr, 0);
        context->GSSetShader(mGeometryShader.Get(), nullptr, 0);
        context->DSSetShader(mDomainShader.Get(), nullptr, 0);
        context->HSSetShader(mHullShader.Get(), nullptr, 0);
        context->IASetPrimitiveTopology(mPrimitiveTopology);
        context->IASetInputLayout(mInputLayout.Get());

        context->RSSetState(mRasterizationState.Get());
        context->OMSetDepthStencilState(mDepthStencilState.Get(), stencilValue);
        context->OMSetBlendState(mColorBlendState.Get(), mBlendConstants, kDefaultSampleCoverageMask);
    }
}
