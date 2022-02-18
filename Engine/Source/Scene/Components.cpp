#include "Components.h"

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
}
