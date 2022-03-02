#include "Mesh.h"

namespace engine
{
    void Mesh::SetVertices(math::float3* vertices, uint32_t count)
    {
        mVertices.reserve(count);
        memcpy(mVertices.data(), vertices, count * sizeof(math::float3));
    }

    const math::float3* Mesh::GetVertices() const
    {
        return mVertices.data();
    }

    size_t Mesh::GetVerticesCount() const
    {
        return mVertices.size();
    }

    void Mesh::SetIndices(uint32_t* indices, uint32_t count)
    {
        mIndices.reserve(count);
        memcpy(mIndices.data(), indices, count * sizeof(uint32_t));
    }

    const uint32_t* Mesh::GetIndices() const
    {
        return mIndices.data();
    }

    size_t Mesh::GetIndicesCount() const
    {
        return mIndices.size();
    }

    void Mesh::SetNormals(math::float3* indices, uint32_t count)
    {
        mNormals.reserve(count);
        memcpy(mNormals.data(), indices, count * sizeof(uint32_t));
    }

    const math::float3* Mesh::GetNormals() const
    {
        return mNormals.data();
    }

    size_t Mesh::GetNormalsCount() const
    {
        return mNormals.size();
    }

    void Mesh::SetUVs(uint32_t uvindex, math::float2* uvs, uint32_t count)
    {
        // TODO: GameEngine only supports UV0 for now.
        ASSERT(uvindex == 0);

        mUV0.reserve(count);
        memcpy(mUV0.data(), uvs, count * sizeof(math::float2));
    }

    const math::float2* Mesh::GetUVs(uint32_t uvindex) const
    {
        // TODO: GameEngine only supports UV0 for now.
        ASSERT(uvindex == 0);
        return mUV0.data();
    }

    size_t Mesh::GetUVsCount(uint32_t uvindex) const
    {
        // TODO: GameEngine only supports UV0 for now.
        ASSERT(uvindex == 0);
        return mUV0.size();
    }
}


GE::Mesh* GE::Mesh::CreateMesh()
{
    return new engine::Mesh();
}
