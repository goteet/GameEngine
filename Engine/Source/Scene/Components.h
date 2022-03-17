#pragma once
#include "PreIncludeFiles.h"
#include "SceneNode.h"

namespace engine
{
    class Camera : public GE::Camera
    {
    public:
        DefineRTTI;

        virtual unsigned int GetCameraIndex() const override { return mCameraIndex; }
        virtual void SetEyePosition(const math::point3d<float>& p) override;
        virtual void Lookat(const math::point3d<float>& target) override;
        virtual void SetRenderingOrder(int order) override;
        virtual int GetRenderingOrder() const override { return mRenderingOrder; }
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask) override;
        virtual GE::CameraVisibleMask GetCameraVisibleMask() const override { return mVisibleMask; }
        virtual const math::float4x4& GetViewMatrixWithoutUpdate() const override { return mMatrixView; }
        virtual const math::float4x4& GetUpdatedViewMatrix() override;
        //virtual math::point3d<float> ScreenToWorld(const math::point2d<int>& pos) const { return pos; }
        //virtual math::point2d<int> WorldToScreen(const math::point3d<float>& pos) const { return pos; }

        Camera(uint32_t cameraIndex);
    private:
        void UpdateViewMatrix();

        uint32_t mCameraIndex = 0;
        int mRenderingOrder = 0;
        GE::CameraVisibleMask mVisibleMask = GE::DefaultCameraVisibkeMask;
        bool mMatrixViewDirty = true;
        math::float4x4 mMatrixView;
        math::float4x4 mMatrixViewInverse;
    };

    class DirectionalLight : public GE::DirectionalLight
    {
    public:
        DefineRTTI;

        virtual void SetColor(const math::float3& color) override;
        virtual math::float3 GetColor() const override { return mColor; }
        virtual void SetIntensity(float intensity) override;
        virtual float GetIntensity() const override { return mIntensity; }
        math::view_matrix4x4f GetViewMatrix() const;

    private:
        float mIntensity;
        math::float3 mColor;
    };

    class MeshRenderer : public GE::MeshRenderer
    {
    public:
        DefineRTTI;

        ~MeshRenderer();
        virtual void OnRender(GE::GfxDeferredContext* context) override;

        bool IntializeGeometryHWResource(GE::MeshRenderer::EMeshType type);

    private:
        GE::GfxDefaultVertexBuffer* CubeVertexBufferPtr = nullptr;
        GE::GfxDefaultIndexBuffer* CubeIndexBufferPtr = nullptr;
    };
}
