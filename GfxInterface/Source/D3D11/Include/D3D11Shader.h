#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct ShaderBinaryD3D11 : public ShaderBinary
    {
        ShaderBinaryD3D11(EShaderType, ID3DBlob* blobBinary);
        virtual ~ShaderBinaryD3D11();
        virtual void Release() override;

        virtual EShaderType     GetShaderType() override { return mShaderType; }
        virtual unsigned int    GetBytecodeLength() override { return static_cast<unsigned int>(mShaderBinary->GetBufferSize()); }
        virtual void*           GetBytecode() override { return mShaderBinary->GetBufferPointer(); }

        ComPtr<ID3DBlob> GetShaderBinaryBlob() { return mShaderBinary; }
    private:
        EShaderType mShaderType;
        ComPtr<ID3DBlob> mShaderBinary;
    };


    template<typename D3D11ShaderType>
    struct TShaderD3D11 : public Shader
    {
        TShaderD3D11(EShaderType shaderType, ComPtr<ID3DBlob> shaderBinary, D3D11ShaderType* shader)
            : mShaderType(shaderType), mShaderBinary(shaderBinary), mShader(shader) { }
        virtual ~TShaderD3D11() { mShaderBinary.Reset(); mShader.Reset(); }
        virtual void Release() override { delete this; }

        virtual EShaderType     GetShaderType() override { return mShaderType; }
        virtual unsigned int    GetBytecodeLength() override { return static_cast<unsigned int>(mShaderBinary->GetBufferSize()); }
        virtual void*           GetBytecode() override { return mShaderBinary->GetBufferPointer(); }

        virtual void* GetRawHandle() override { return mShader.Get(); }
        ComPtr<D3D11ShaderType> GetShaderPtr() { return mShader; }

    private:
        EShaderType mShaderType;
        ComPtr<ID3DBlob> mShaderBinary;
        ComPtr<D3D11ShaderType> mShader;
    };

    template <typename D3D11ShaderType>
    inline ComPtr<D3D11ShaderType> GetShaderPtr(Shader* shader)
    {
        return shader ? dynamic_cast<TShaderD3D11<D3D11ShaderType>*>(shader)->GetShaderPtr() : nullptr;
    }


    template<typename D3D11ShaderType>
    inline TShaderD3D11<D3D11ShaderType>* NewTShaderInstance(EShaderType shaderType, ComPtr<ID3DBlob> shaderBinary, D3D11ShaderType* shader)
    {
        return new TShaderD3D11<D3D11ShaderType>(shaderType, shaderBinary, shader);
    }

    using VertexShaderD3D11      = TShaderD3D11<ID3D11VertexShader>;
    using PixelShaderD3D11       = TShaderD3D11<ID3D11PixelShader>;
    using GeometryShaderD3D11    = TShaderD3D11<ID3D11GeometryShader>;
    using DomainShaderD3D11      = TShaderD3D11<ID3D11DomainShader>;
    using HullShaderD3D11        = TShaderD3D11<ID3D11HullShader>;
    using ComputeShaderD3D11     = TShaderD3D11<ID3D11ComputeShader>;
}
