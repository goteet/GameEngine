#include <GfxInterface.h>
#include "Components.h"
#include "Core/GameEngine.h"

namespace engine
{
    void Camera::SetEyePosition(const math::point3d<float>& p)
    {
        GetSceneNode()->SetWorldPosition(p);
        mMatrixViewDirty = true;
    }

    void Camera::Lookat(const math::point3d<float>& target)
    {
        math::float3 forward = target - GetSceneNode()->GetWorldPosition();
        GetSceneNode()->SetForwardDirection(forward);
        mMatrixViewDirty = true;
    }

    void Camera::SetRenderingOrder(int order)
    {
        mRenderingOrder = order;
    }

    void Camera::SetCameraVisibleMask(GE::CameraVisibleMask mask)
    {
        mVisibleMask = mask;
    }

    Camera::Camera(uint32_t cameraIndex)
        : mCameraIndex(cameraIndex)
    {

    }

    const math::float4x4& Camera::GetUpdatedViewMatrix()
    {
        UpdateViewMatrix();
        return mMatrixView;
    }

    void Camera::UpdateViewMatrix()
    {
        if (mMatrixViewDirty)
        {
            const math::point3df& eye = GetSceneNode()->GetWorldPosition();
            const math::point3df& forward = GetSceneNode()->GetForwardDirection();
            const math::point3df& up = GetSceneNode()->GetUpDirection();
            const math::point3df& right = GetSceneNode()->GetRightDirection();
            mMatrixView = math::view_matrix4x4f(eye, forward, up, right);
            mMatrixViewInverse = math::inversed(mMatrixView);
        }
    }

    void DirectionalLight::SetColor(const math::float3& color)
    {
        mColor = color;
    }

    void DirectionalLight::SetIntensity(float intensity)
    {
        mIntensity = intensity;
    }

    math::view_matrix4x4f DirectionalLight::GetViewMatrix() const
    {
        const math::point3df& eye = GetSceneNode()->GetWorldPosition();
        const math::point3df& forward = GetSceneNode()->GetForwardDirection();
        const math::point3df& up = GetSceneNode()->GetUpDirection();
        const math::point3df& right = GetSceneNode()->GetRightDirection();
        return math::view_matrix4x4f(eye, forward, up, right);
    }


    namespace cube_geometry_desc
    {
        const unsigned int CubeVertexCount = 24;
        engine::VertexLayout CubeVertices[CubeVertexCount] =
        {
            { math::float4(-0.5f, +0.5f, -0.5f, 1), math::normalized_float3::unit_y(),  math::float2(0, 0) },
            { math::float4(-0.5f, +0.5f, +0.5f, 1), math::normalized_float3::unit_y(),  math::float2(0, 1) },
            { math::float4(+0.5f, +0.5f, +0.5f, 1), math::normalized_float3::unit_y(),  math::float2(1, 1) },
            { math::float4(+0.5f, +0.5f, -0.5f, 1), math::normalized_float3::unit_y(),  math::float2(1, 0) },

            { math::float4(-0.5f, -0.5f, +0.5f, 1), math::normalized_float3::unit_y_neg(),  math::float2(0, 0) },
            { math::float4(-0.5f, -0.5f, -0.5f, 1), math::normalized_float3::unit_y_neg(),  math::float2(0, 1) },
            { math::float4(+0.5f, -0.5f, -0.5f, 1), math::normalized_float3::unit_y_neg(),  math::float2(1, 1) },
            { math::float4(+0.5f, -0.5f, +0.5f, 1), math::normalized_float3::unit_y_neg(),  math::float2(1, 0) },

            { math::float4(+0.5f, -0.5f, -0.5f, 1), math::normalized_float3::unit_x(),  math::float2(0, 0) },
            { math::float4(+0.5f, +0.5f, -0.5f, 1), math::normalized_float3::unit_x(),  math::float2(0, 1) },
            { math::float4(+0.5f, +0.5f, +0.5f, 1), math::normalized_float3::unit_x(),  math::float2(1, 1) },
            { math::float4(+0.5f, -0.5f, +0.5f, 1), math::normalized_float3::unit_x(),  math::float2(1, 0) },

            { math::float4(-0.5f, -0.5f, +0.5f, 1), math::normalized_float3::unit_x_neg(),  math::float2(0, 0) },
            { math::float4(-0.5f, +0.5f, +0.5f, 1), math::normalized_float3::unit_x_neg(),  math::float2(0, 1) },
            { math::float4(-0.5f, +0.5f, -0.5f, 1), math::normalized_float3::unit_x_neg(),  math::float2(1, 1) },
            { math::float4(-0.5f, -0.5f, -0.5f, 1), math::normalized_float3::unit_x_neg(),  math::float2(1, 0) },

            { math::float4(+0.5f, -0.5f, +0.5f, 1), math::normalized_float3::unit_z(),  math::float2(0, 0) },
            { math::float4(+0.5f, +0.5f, +0.5f, 1), math::normalized_float3::unit_z(),  math::float2(0, 1) },
            { math::float4(-0.5f, +0.5f, +0.5f, 1), math::normalized_float3::unit_z(),  math::float2(1, 1) },
            { math::float4(-0.5f, -0.5f, +0.5f, 1), math::normalized_float3::unit_z(),  math::float2(1, 0) },

            { math::float4(-0.5f, -0.5f, -0.5f, 1), math::normalized_float3::unit_z_neg(),  math::float2(0, 0) },
            { math::float4(-0.5f, +0.5f, -0.5f, 1), math::normalized_float3::unit_z_neg(),  math::float2(0, 1) },
            { math::float4(+0.5f, +0.5f, -0.5f, 1), math::normalized_float3::unit_z_neg(),  math::float2(1, 1) },
            { math::float4(+0.5f, -0.5f, -0.5f, 1), math::normalized_float3::unit_z_neg(),  math::float2(1, 0) }
        };

        const unsigned int CubeIndexCount = 36;
        unsigned int CubeIndices[CubeIndexCount] =
        {
            0, 3, 2, 2, 1, 0,
            4, 7, 6, 6, 5, 4,
            8, 11, 10, 10, 9, 8,
            12, 15, 14, 14, 13, 12,
            16, 19, 18, 18, 17, 16,
            20, 23, 22, 22, 21, 20,
        };
    }

    namespace plane_geomoetry_desc
    {
        const unsigned int PlaneVertexCount = 4;
        engine::VertexLayout PlaneVertices[PlaneVertexCount] =
        {
            { math::float4(-0.5f, 0.0f, -0.5f, 1), math::normalized_float3::unit_y(),  math::float2(0, 0) },
            { math::float4(-0.5f, 0.0f, +0.5f, 1), math::normalized_float3::unit_y(),  math::float2(0, 1) },
            { math::float4(+0.5f, 0.0f, +0.5f, 1), math::normalized_float3::unit_y(),  math::float2(1, 1) },
            { math::float4(+0.5f, 0.0f, -0.5f, 1), math::normalized_float3::unit_y(),  math::float2(1, 0) },
        };

        const unsigned int PlaneIndexCount = 6;
        unsigned int PlaneIndices[PlaneIndexCount] = { 0, 3, 2, 2, 1, 0 };
    }

    MeshRenderer::~MeshRenderer()
    {
        SafeRelease(mCubeVertexBuffer);
        SafeRelease(mCubeIndexBuffer);
    }

    void MeshRenderer::OnRender(GFXI::DeferredContext& context)
    {
        const math::float4x4& matrix = GetSceneNode()->GetWorldMatrix();
        GetEngineInstance()->GetRenderSystem()->SetRenderingWorldMatrixForTest(matrix);

        GFXI::DeviceContext::VertexBufferBinding binding;
        binding.VertexBuffer = mCubeVertexBuffer;
        binding.BufferOffset = 0;
        binding.ElementStride = sizeof(engine::VertexLayout);
        
        context.SetVertexBuffers(0, 1, &binding);
        context.SetIndexBuffer(mCubeIndexBuffer, 0);
        context.DrawIndexed(cube_geometry_desc::CubeIndexCount, 0, 0);
    }

    bool MeshRenderer::IntializeGeometryHWResource(GE::MeshRenderer::EMeshType type)
    {
        int vertexCount = 0;
        int indexCount = 0;
        engine::VertexLayout* vertices = nullptr;
        unsigned int* indices = nullptr;
        switch (type)
        {
        default:
            return false;
        case GE::MeshRenderer::EMeshType::Box:
            vertexCount = cube_geometry_desc::CubeVertexCount;
            indexCount = cube_geometry_desc::CubeIndexCount;
            vertices = cube_geometry_desc::CubeVertices;
            indices = cube_geometry_desc::CubeIndices;
            break;
        case GE::MeshRenderer::EMeshType::Plane:
            vertexCount = plane_geomoetry_desc::PlaneVertexCount;
            indexCount = plane_geomoetry_desc::PlaneIndexCount;
            vertices = plane_geomoetry_desc::PlaneVertices;
            indices = plane_geomoetry_desc::PlaneIndices;
            break;
        }


        GE::RenderSystem* renderSystem = GetEngineInstance()->GetRenderSystem();
        GFXI::GraphicDevice* devicePtr = renderSystem->GetGfxDevice();
        //GE::GfxDeviceImmediateContext* contextPtr = renderSystem->GetGfxDeviceImmediateContext();

        GFXI::VertexBuffer::CreateInfo vbCreateInfo;
        vbCreateInfo.BufferSize = vertexCount * sizeof(engine::VertexLayout);
        vbCreateInfo.DataUsage = GFXI::EDataUsage::Default;
        mCubeVertexBuffer   = devicePtr->CreateVertexBuffer(vbCreateInfo);

        GFXI::IndexBuffer::CreateInfo ibCreateInfo;
        ibCreateInfo.BufferSize = indexCount * sizeof(unsigned int);
        ibCreateInfo.DataUsage = GFXI::EDataUsage::Default;
        ibCreateInfo.DataFormat = GFXI::IndexBuffer::EFormat::UInt32;
        mCubeIndexBuffer    = devicePtr->CreateIndexBuffer(ibCreateInfo);
        if (mCubeVertexBuffer == nullptr || mCubeIndexBuffer == nullptr)
        {
            SafeRelease(mCubeVertexBuffer);
            SafeRelease(mCubeIndexBuffer);
            return false;
        }
        //upload cube vertex data to gfx vertex buffer.
        renderSystem->FillEntireEntireBufferFromMemory(mCubeVertexBuffer, vertices);
        renderSystem->FillEntireEntireBufferFromMemory(mCubeIndexBuffer, indices);
        return true;
    }
}


GE::MeshRenderer* GE::MeshRenderer::CreateMeshRenderer(GE::MeshRenderer::EMeshType type)
{
    engine::MeshRenderer* renderer = new engine::MeshRenderer();
    if (!renderer->IntializeGeometryHWResource(type))
    {
        safe_delete(renderer);
    }
    return renderer;
}
