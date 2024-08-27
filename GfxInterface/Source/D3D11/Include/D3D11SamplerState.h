#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct ModuleD3D11;

    struct SamplerStateD3D11 : public SamplerState
    {
        SamplerStateD3D11(const D3D11_SAMPLER_DESC&, ID3D11SamplerState*);
        virtual ~SamplerStateD3D11();
        virtual void Release() override;

        ID3D11SamplerState* GetD3D11SamplerState() { return mSamplerState.Get(); }

    private:
        D3D11_SAMPLER_DESC          mSamplerStateDesc;
        ComPtr<ID3D11SamplerState>  mSamplerState;
    };
}
