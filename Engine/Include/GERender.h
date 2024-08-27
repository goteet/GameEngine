#pragma once
#include <Foundation/Math/Matrix.h>
#include <GfxInterface.h>
#include "GEPredefinedMacros.h"


namespace GE
{
    //enum class GameEngineAPI BlendMode
    //{
    //    None,  // src*1 + dst*0 = src
    //    Normal,  // src*src_a + dst*(1-src_a)
    //    Additve, // src*1 + dst*1
    //};
    //
    //struct GeometryVertex
    //{
    //    math::float3 Position;
    //    math::float3 Texcoord;
    //    math::float4 VertexColor;
    //};
    //
    ///*
    //** User-defined mesh, it is a render resource.
    //** Mesh data save in memory, render system will upload
    //** datas to video memory when rendering, depends on which
    //** material be used.
    //*/
    //struct GameEngineAPI Mesh : public GEObject
    //{
    //public:
    //    static Mesh* Create(unsigned int vertexCount, unsigned int indexCount);
    //
    //    virtual const GeometryVertex* GetRawVertices() const = 0;        // length = sizeof(GeometryVertex) * VertexCount;
    //    virtual GeometryVertex* GetRawVertices() = 0;
    //    virtual const unsigned int* GetRawIndices() const = 0;        // length = sizeof(unsigned int) * IndexCount.
    //    virtual unsigned int* GetRawIndices() = 0;
    //    virtual unsigned int GetVertexCount() const = 0;        // length = sizeof(GeometryVertex) * VertexCount;
    //    virtual unsigned int GetIndexCount() const = 0;        // length = sizeof(unsigned int) * IndexCount;
    //    virtual void ResizeVertexArray(unsigned int vertexCount) = 0;        // Resize the length of vertex data memory, old data will be keeped.
    //    virtual void ResizeIndexArray(unsigned int indexCount) = 0;// Resize the length of index data memory, old data will be keeped.
    //    virtual bool Merge(Mesh* other, const math::float4x4& transform) = 0;
    //
    //    virtual void Release() = 0;
    //};


    struct GameEngineAPI Mesh : public GEObject
    {
        static Mesh* CreateMesh();
        virtual void SetVertices(math::float3* vertices, uint32_t count) = 0;
        virtual const math::float3* GetVertices() const = 0;
        virtual size_t GetVerticesCount() const = 0;
        virtual void SetIndices(uint32_t* indices, uint32_t count) = 0;
        virtual const uint32_t* GetIndices() const = 0;
        virtual size_t GetIndicesCount() const = 0;
        virtual void SetNormals(math::float3* normals, uint32_t count) = 0;
        virtual const math::float3* GetNormals() const = 0;
        virtual size_t GetNormalsCount() const = 0;
        virtual void SetUVs(uint32_t uvindex, math::float2* uvs, uint32_t count) = 0;
        virtual const math::float2* GetUVs(uint32_t uvindex) const = 0;
        virtual size_t GetUVsCount(uint32_t uvindex) const = 0;
        virtual void Release() = 0;
    };

    //
    //struct GameEngineAPI Texture : public GEObject
    //{
    //    static Texture* LoadFromFile(const char* path);
    //    virtual const char* Identifier() const = 0;
    //    virtual bool IsSame(Texture* other) const = 0;
    //    virtual void AddRef() = 0;
    //    virtual void Release() = 0;
    //};
    //
    ////struct GameEngineAPI Pass : public GEObject
    ////{
    ////    virtual const char* GetVertexShaderName() const = 0;
    ////    virtual const char* GetPixelShaderName() const = 0;
    ////    virtual bool IsSame(Pass* other) const = 0;
    ////    virtual void SetBlendMode(BlendMode mode) = 0;
    ////    virtual void SetVSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) = 0;
    ////    virtual void SetPSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) = 0;
    ////    virtual void SetTexture(unsigned int index, Texture*, bool autoRelease) = 0;
    ////    virtual Texture* GetTextureByIndex(unsigned int index) const = 0;
    ////    virtual unsigned int GetTextureCount() const = 0;
    ////    virtual const float* GetVSConstant() const = 0;
    ////    virtual unsigned int GetVSConstantLength() const = 0;
    ////    virtual const float* GetPSConstant() const = 0;
    ////    virtual unsigned int GetPSConstantLength() const = 0;
    ////    virtual BlendMode GetBlendMode() const = 0;
    ////};
    //
    //struct GameEngineAPI Material : public GEObject
    //{
    //    static Material* CreateColorTexture();
    //    static Material* CreateSimpleTexture();
    //    static Material* CreateSimpleColor();
    //    //virtual void Release() = 0;
    //    //virtual Pass* GetPassByIndex(unsigned int index) const = 0;
    //    //virtual unsigned int GetPassCount() const = 0;
    //    //virtual bool IsSame(Material* other) const = 0;
    //    //virtual Material* Clone() const = 0;
    //};

    struct GfxBuffer : public GEObject
    {
        virtual void Release() = 0;
    };
    struct GfxVertexBuffer : public GfxBuffer {};
    struct GfxIndexBuffer : public GfxBuffer {};

    struct GfxDefaultVertexBuffer : public GfxVertexBuffer{};
    struct GfxDefaultIndexBuffer : public GfxIndexBuffer{};

    struct GameEngineAPI GfxDevice : public GEObject
    {
        virtual GfxDefaultVertexBuffer* CreateDefaultVertexBuffer(unsigned int vertexCount) = 0;
        virtual GfxDefaultIndexBuffer* CreateDefaultIndexBuffer(unsigned int indexCount) = 0;
    };

    struct GameEngineAPI GfxDeviceImmediateContext : public GEObject
    {
        virtual void UploadEntireBufferFromStagingMemory(GfxBuffer*, const void* data) = 0;
        virtual void UploadEntireBufferFromMemory(GfxBuffer*, const void* data) = 0;
    };

    struct GameEngineAPI RenderSystem : public GEObject
    {
        virtual GFXI::GraphicDevice* GetGfxDevice() = 0;
        virtual unsigned int GetWindowWidth() const = 0;
        virtual unsigned int GetWindowHeight() const = 0;
        virtual math::point3d<float> ScreenToView(const math::point3d<int>& screen) const = 0;
        virtual math::point3d<int> ViewToScreen(const math::point3d<float>& view) const = 0;
        virtual void SetRenderingWorldMatrixForTest(const math::float4x4&) = 0;
        virtual void FillEntireEntireBufferFromMemory(GFXI::Buffer*, const void* data) = 0;
    };
}
