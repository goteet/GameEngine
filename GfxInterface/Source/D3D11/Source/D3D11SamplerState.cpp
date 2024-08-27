#include "D3D11SamplerState.h"


namespace GFXI
{
    SamplerStateD3D11::SamplerStateD3D11(const D3D11_SAMPLER_DESC& samplerStateDesc, ID3D11SamplerState* samplerState)
        : mSamplerStateDesc(samplerStateDesc)
        , mSamplerState(samplerState)
    {
    }

    SamplerStateD3D11::~SamplerStateD3D11()
    {
        mSamplerState.Reset();
    }

    void SamplerStateD3D11::Release()
    {
        delete this;
    }
}
