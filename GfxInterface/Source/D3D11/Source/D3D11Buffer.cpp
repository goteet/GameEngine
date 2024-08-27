#include "D3D11Buffer.h"


namespace GFXI
{

    VertexBufferD3D11::VertexBufferD3D11(EDataUsage usage, unsigned int size, ID3D11Buffer* vertexBuffer)
        : mBufferUsage(usage)
        , mBufferSize(size)
        , mD3D11VertexBuffer(vertexBuffer)
    {

    }
    VertexBufferD3D11::~VertexBufferD3D11()
    {
        mD3D11VertexBuffer.Reset();
    }
    void VertexBufferD3D11::Release()
    {
        delete this;
    }

    IndexBufferD3D11::IndexBufferD3D11(EFormat indexFormat, EDataUsage usage, unsigned int size, ID3D11Buffer* vertexBuffer)
        : mIndexFormat(indexFormat)
        , mBufferUsage(usage)
        , mBufferSize(size)
        , mD3D11IndexBuffer(vertexBuffer)
    {
    }
    IndexBufferD3D11::~IndexBufferD3D11()
    {
        mD3D11IndexBuffer.Reset();
    }
    void IndexBufferD3D11::Release()
    {
        delete this;
    }

    UniformBufferD3D11::UniformBufferD3D11(EDataUsage usage, unsigned int size, ID3D11Buffer* vertexBuffer)
        : mBufferUsage(usage)
        , mBufferSize(size)
        , mD3D11UniformBuffer(vertexBuffer)
    {

    }
    UniformBufferD3D11::~UniformBufferD3D11()
    {
        mD3D11UniformBuffer.Reset();
    }
    void UniformBufferD3D11::Release()
    {
        delete this;
    }
}
