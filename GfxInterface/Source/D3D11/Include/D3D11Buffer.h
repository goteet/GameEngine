#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct VertexBufferD3D11 : public VertexBuffer
    {
        VertexBufferD3D11(EDataUsage usage, unsigned int size, ID3D11Buffer* vertexBuffer);
        ~VertexBufferD3D11();
        virtual void Release() override;
        virtual EBinding GetBinding() override final { return EBinding::Vertex; }
        virtual EDataUsage GetUsage() override { return mBufferUsage; }
        virtual unsigned int GetBufferSize() override { return mBufferSize; }

        ID3D11Buffer* GetD3D11Buffer() { return mD3D11VertexBuffer.Get(); }

    private:
        EDataUsage mBufferUsage;
        unsigned int mBufferSize;
        ComPtr<ID3D11Buffer> mD3D11VertexBuffer;
    };

    struct IndexBufferD3D11 : public IndexBuffer
    {
        IndexBufferD3D11(EFormat indexFormat, EDataUsage usage, unsigned int size, ID3D11Buffer* vertexBuffer);
        ~IndexBufferD3D11();
        virtual void Release() override;
        virtual EBinding GetBinding() override final { return EBinding::Index; }
        virtual EDataUsage GetUsage() override { return mBufferUsage; }
        virtual unsigned int GetBufferSize() override { return mBufferSize; }
        virtual EFormat GetIndexFormat() override { return mIndexFormat; }

        ID3D11Buffer* GetD3D11Buffer() { return mD3D11IndexBuffer.Get(); }

    private:
        EDataUsage      mBufferUsage;
        EFormat         mIndexFormat;
        unsigned int    mBufferSize;
        ComPtr<ID3D11Buffer> mD3D11IndexBuffer;
    };

    struct UniformBufferD3D11 : public UniformBuffer
    {
        UniformBufferD3D11(EDataUsage usage, unsigned int size, ID3D11Buffer* vertexBuffer);
        ~UniformBufferD3D11();
        virtual void Release() override;
        virtual EBinding GetBinding() override final { return EBinding::Uniform; }
        virtual EDataUsage GetUsage() override { return mBufferUsage; }
        virtual unsigned int GetBufferSize() override { return mBufferSize; }

        ID3D11Buffer* GetD3D11Buffer() { return mD3D11UniformBuffer.Get(); }

    private:
        EDataUsage mBufferUsage;
        unsigned int mBufferSize;
        ComPtr<ID3D11Buffer> mD3D11UniformBuffer;
    };
}



