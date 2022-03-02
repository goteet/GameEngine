#pragma once
#include "PreIncludeFiles.h"
#include <vector>
namespace engine
{
    class Mesh : public GE::Mesh
    {
    public:
        DefineRTTI;

        virtual ~Mesh() = default;

        virtual void SetVertices(math::float3* vertices, uint32_t count) override;
        virtual const math::float3* GetVertices() const override;
        virtual size_t GetVerticesCount() const override;
        virtual void SetIndices(uint32_t* indices, uint32_t count) override;
        virtual const uint32_t* GetIndices() const override;
        virtual size_t GetIndicesCount() const override;
        virtual void SetNormals(math::float3* normals, uint32_t count) override;
        virtual const math::float3* GetNormals() const override;
        virtual size_t GetNormalsCount() const override;
        virtual void SetUVs(uint32_t uvindex, math::float2* uv, uint32_t count) override;
        virtual const math::float2* GetUVs(uint32_t uvindex) const override;
        virtual size_t GetUVsCount(uint32_t uvindex) const override;
        virtual void Release() override { delete this; }
    private:
        std::vector<math::float3> mVertices;
        std::vector<uint32_t> mIndices;
        std::vector<math::normalized_float3> mNormals;
        std::vector<math::float2> mUV0;
    };
}
