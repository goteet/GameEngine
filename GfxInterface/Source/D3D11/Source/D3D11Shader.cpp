#include "D3D11Shader.h"


namespace GFXI
{
    ShaderBinaryD3D11::ShaderBinaryD3D11(EShaderType Type, ID3DBlob* Binary, const std::string& Name, const std::string& EntryPoint)
        : mShaderType(Type), mShaderBinary(Binary), mName(Name), mEntryPoint(EntryPoint)
    {
    }

    ShaderBinaryD3D11::~ShaderBinaryD3D11()
    {
        mShaderBinary.Reset();
    }

    void ShaderBinaryD3D11::Release()
    {
        delete this;
    }
    EShaderType ShaderBinaryD3D11::GetShaderType()
    {
        return mShaderType;
    }

    const char* ShaderBinaryD3D11::GetShaderName()
    {
        return mName.c_str();
    }

    const char* ShaderBinaryD3D11::GetEntryPointName()
    {
        return mEntryPoint.c_str();
    }

    void* ShaderBinaryD3D11::GetBytecode()
    {
        return mShaderBinary->GetBufferPointer();
    }

    unsigned int ShaderBinaryD3D11::GetBytecodeLength()
    {
        return static_cast<unsigned int>(mShaderBinary->GetBufferSize());
    }

    ComPtr<ID3DBlob> ShaderBinaryD3D11::GetShaderBinaryBlob()
    {
        return mShaderBinary;
    }
}
