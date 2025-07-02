#pragma once
#include <string>
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct ShaderBinaryD3D11 : public ShaderBinary
    {
        ShaderBinaryD3D11(EShaderType, ID3DBlob* Binary, const std::string& Name, const std::string& EntryPoint);
        virtual ~ShaderBinaryD3D11();
        virtual void Release() override;

        virtual EShaderType     GetShaderType() override;
        virtual const char*     GetShaderName() override;
        virtual const char*     GetEntryPointName() override;
        virtual void*           GetBytecode() override;
        virtual unsigned int    GetBytecodeLength() override;

        ComPtr<ID3DBlob>        GetShaderBinaryBlob();
    private:
        EShaderType mShaderType;
        ComPtr<ID3DBlob> mShaderBinary;
        std::string mName;
        std::string mEntryPoint;
    };


    template<typename D3D11ShaderType>
    struct TShaderD3D11 : public Shader
    {
        TShaderD3D11(EShaderType Type, ComPtr<ID3DBlob> Binary, D3D11ShaderType* Shader, const std::string& Name, const std::string& EntryPoint)
            : mShaderType(Type), mShaderBinary(Binary), mShader(Shader), mName(Name), mEntryPoint(EntryPoint) { }
        virtual ~TShaderD3D11() { mShaderBinary.Reset(); mShader.Reset(); }
        virtual void Release() override { delete this; }
        virtual EShaderType     GetShaderType() override { return mShaderType; }
        virtual const char*     GetShaderName() override { return mName.c_str(); }
        virtual const char*     GetEntryPointName() override { return mEntryPoint.c_str(); }
        virtual void*           GetBytecode() override { return mShaderBinary->GetBufferPointer(); }
        virtual unsigned int    GetBytecodeLength() override { return static_cast<unsigned int>(mShaderBinary->GetBufferSize()); }

        ComPtr<D3D11ShaderType> GetShaderPtr() { return mShader; }
    private:
        EShaderType mShaderType;
        ComPtr<ID3DBlob> mShaderBinary;
        ComPtr<D3D11ShaderType> mShader;
        std::string mName;
        std::string mEntryPoint;
    };

    template <typename D3D11ShaderType>
    inline ComPtr<D3D11ShaderType> GetShaderPtr(Shader* shader)
    {
        return shader ? reinterpret_cast<TShaderD3D11<D3D11ShaderType>*>(shader)->GetShaderPtr() : nullptr;
    }


    template<typename D3D11ShaderType>
    inline TShaderD3D11<D3D11ShaderType>* NewTShaderInstance(EShaderType ShaderType, ComPtr<ID3DBlob> ShaderBinary, D3D11ShaderType* ShaderPtr, const std::string& Name, const std::string& EntryPoint)
    {
        return new TShaderD3D11<D3D11ShaderType>(ShaderType, ShaderBinary, ShaderPtr, Name, EntryPoint);
    }

    using VertexShaderD3D11      = TShaderD3D11<ID3D11VertexShader>;
    using PixelShaderD3D11       = TShaderD3D11<ID3D11PixelShader>;
    using GeometryShaderD3D11    = TShaderD3D11<ID3D11GeometryShader>;
    using DomainShaderD3D11      = TShaderD3D11<ID3D11DomainShader>;
    using HullShaderD3D11        = TShaderD3D11<ID3D11HullShader>;
    using ComputeShaderD3D11     = TShaderD3D11<ID3D11ComputeShader>;
}
