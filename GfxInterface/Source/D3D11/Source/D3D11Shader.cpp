#include "D3D11Shader.h"


namespace GFXI
{
    ShaderBinaryD3D11::ShaderBinaryD3D11(EShaderType shaderType, ID3DBlob* blobBinary)
        : mShaderType(shaderType), mShaderBinary(blobBinary)
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
}
