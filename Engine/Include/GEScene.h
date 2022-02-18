#pragma once
#include <Foundation/Math/Matrix.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>
#include "GEPredefinedMacros.h"

namespace GE
{
    using CameraVisibleMask = uint32_t;
    using RenderingOrder = uint32_t;
    using ComponentExecutionOrder = uint32_t;
    struct SceneNode;
    struct Component;
    struct Scene;

    constexpr CameraVisibleMask DefaultCameraVisibkeMask = 0xFFFFFFFF;
    constexpr ComponentExecutionOrder DefaultComponentExecutionOrder = 0x5000;

    enum AutoReleaseComponentHint { AutoReleaseComponent };
    enum RecursiveSetCameraVisibleMaskHint { RecursiveSetCameraVisibleMask };

    struct GameEngineAPI SceneNode : public GEObject
    {
        virtual Scene* GetScene() = 0;
        virtual SceneNode* GetPrevSiblingNode() = 0;
        virtual SceneNode* GetNextSiblingNode() = 0;
        virtual SceneNode* GetParentNode() = 0;
        //virtual SceneNode* GetFirstChild() = 0;
        //virtual SceneNode* GetLastChild() = 0;
        //virtual SceneNode* GetChildByIndex(unsigned int index) = 0;
        //virtual unsigned int GetChildCount() const = 0;
        //virtual SceneNode* CreateChild() = 0;
        //virtual void MoveToFirst() = 0;
        //virtual void MoveToLast() = 0;
        //virtual void MovePrev() = 0;
        //virtual void MoveNext() = 0;
        virtual bool AddComponent(Component*) = 0;
        virtual bool AddComponent(Component*, AutoReleaseComponentHint) = 0;
        virtual bool RemoveComponent(Component*) = 0;
        virtual bool RemoveComponentWithoutRelease(Component*) = 0;
        virtual bool HasComponent(Component*) const = 0;
        virtual bool IsComponentAutoRelease(Component*) const = 0;
        virtual Component* GetComponentByIndex(unsigned int index) = 0;
        virtual unsigned int GetComponentCount() const = 0;
        virtual const math::float4x4& GetLocalMatrix() = 0;
        virtual const math::float4x4& GetWorldMatrix() = 0;
        virtual void SetLocalPosition(const math::point3d<float>& Position) = 0;
        virtual math::point3d<float> GetLocalPosition() const = 0;
        virtual void SetWorldPosition(const math::point3d<float>& Position) = 0;
        virtual math::point3d<float> GetWorldPosition() = 0;
        virtual void SetRightDirection(const math::normalized_float3& right) = 0;
        virtual const math::normalized_float3 GetRightDirection() = 0;
        virtual void SetUpDirection(const math::normalized_float3& up) = 0;
        virtual const math::normalized_float3 GetUpDirection() = 0;
        virtual void SetForwardDirection(const math::normalized_float3& forward) = 0;
        virtual const math::normalized_float3 GetForwardDirection() = 0;
        virtual void SetLocalScale(const math::float3& scale) = 0;
        virtual const math::float3 GetLocalScale() const = 0;
        //virtual SceneNode* SetLocalRotation(math::radian<float> r) = 0;
        //virtual math::radian<float> GetLocalRotation() const = 0;
        virtual void SetVisible(bool) = 0;
        virtual bool IsVisible() const = 0;
        virtual void SetStatic(bool) = 0;
        virtual bool IsStatic() const = 0;
        virtual void SetCameraVisibleMask(CameraVisibleMask mask) = 0;
        virtual void SetCameraVisibleMask(CameraVisibleMask mask, RecursiveSetCameraVisibleMaskHint) = 0;
        virtual CameraVisibleMask GetCameraVisibleMask() const = 0;
        //virtual unsigned int GetChildIndex() const = 0;
        virtual unsigned int GetIndex() const = 0;
        //virtual bool IsRemoved() const = 0;
        virtual math::point3d<float> WorldToLocal(const math::point3d<float>& pos) = 0;
        virtual math::point3d<float> LocalToWorld(const math::point3d<float>& pos) = 0;
        //virtual math::point3d<float> WorldToParent(const math::point3d<float>& pos) = 0;
    protected:
        void OnComponentAttach(Component* comp);
    };



    struct GameEngineAPI Component : public GEObject
    {
        SceneNode* GetSceneNode() const { return mSceneNode; }

        math::point3d<float> GetSceneNodeLocalPosition() const { return mSceneNode->GetLocalPosition(); }
        math::point3d<float> GetSceneNodeWorldPosition() const { return mSceneNode->GetWorldPosition(); }
        //math::radian<float> GetNodeRotation() const { return m}
        math::float3 GetSceneNodeLocalScale() const { return mSceneNode->GetLocalScale(); }
        CameraVisibleMask GetSceneNodeCameraVisibleMask() const { return mSceneNode->GetCameraVisibleMask(); }
        //virtual const math::aabb<float>& GetLocalAABB() const { static aabb2d<float> b; return b; }
        //virtual math::aabb2d<float> GetWorldAABB() const;
        virtual ComponentExecutionOrder GetExecutionOrder() const { return DefaultComponentExecutionOrder; }

        /********************************************************************
        * Event Listener
        *********************************************************************/
        virtual void OnInitial() { }
        virtual void OnUpdate(unsigned int deltaTime) { }
        virtual void OnRender() { }
        virtual void OnRemove() { }
        //virtual void OnPositionChanging(const math::point3d<float>& newPos) { }
        //virtual void OnPositionChanged(const math::point3d<float>& newPos) { }
        //virtual void OnPivotChanging(const math::point3d<float>& newPos) { }
        //virtual void OnPivotChanged(const math::point3d<float>& newPos) { }
        //virtual void OnRotateChanging(radian<float> r) { }
        //virtual void OnRotateChanged(radian<float> r) { }
        //virtual void OnScaleChanging(const math::float3& newScaler) { }
        //virtual void OnScaleChanged(const math::float3& newScaler) { }
        //virtual void OnPostUpdateTransformChanged() { }
        //virtual void OnMessage(const Message& message) { }
        //virtual void OnCursorEnterFrom(SceneNode* adjacency, const Mouse&, const Keyboard&) { }
        //virtual void OnCursorHovering(const Mouse&, const Keyboard&) { }
        //virtual void OnCursorLeaveTo(SceneNode* adjacency, const Mouse&, const Keyboard&) { }
        //virtual void OnLClick(const Mouse&, const Keyboard&) { }
        //virtual void OnRClick(const Mouse&, const Keyboard&) { }
        //virtual void OnMClick(const Mouse&, const Keyboard&) { }
        //virtual void OnLDoubleClick(const Mouse&, const Keyboard&) { }
        //virtual void OnRDoubleClick(const Mouse&, const Keyboard&) { }
        //virtual void OnMDoubleClick(const Mouse&, const Keyboard&) { }
        //virtual void OnLDragBegin(const Mouse&, const Keyboard&) { }
        //virtual void OnRDragBegin(const Mouse&, const Keyboard&) { }
        //virtual void OnMDragBegin(const Mouse&, const Keyboard&) { }
        //virtual void OnLDragging(const Mouse&, const Keyboard&) { }
        //virtual void OnRDragging(const Mouse&, const Keyboard&) { }
        //virtual void OnMDragging(const Mouse&, const Keyboard&) { }
        //virtual void OnLDragEnd(const Mouse&, const Keyboard&) { }
        //virtual void OnRDragEnd(const Mouse&, const Keyboard&) { }
        //virtual void OnMDragEnd(const Mouse&, const Keyboard&) { }
        //virtual void OnLDropping(SceneNode* dropped, const Mouse&, const Keyboard&) { }
        //virtual void OnRDropping(SceneNode* dropped, const Mouse&, const Keyboard&) { }
        //virtual void OnMDropping(SceneNode* dropped, const Mouse&, const Keyboard&) { }
        //virtual void OnLDropTo(SceneNode* dropped, const Mouse&, const Keyboard&) { }
        //virtual void OnRDropTo(SceneNode* dropped, const Mouse&, const Keyboard&) { }
        //virtual void OnMDropTo(SceneNode* dropped, const Mouse&, const Keyboard&) { }
        //virtual void OnKeyPress(Message::EKeyCode key, const Mouse&, const Keyboard& keyboard) { }
        //virtual void OnKeyPressingBegin(Message::EKeyCode key, const Mouse&, const Keyboard& keyboard) { }
        //virtual void OnKeyPressing(Message::EKeyCode key, const Mouse&, const Keyboard& keyboard) { }
        //virtual void OnKeyPressingEnd(Message::EKeyCode key, const Mouse&, const Keyboard& keyboard) { }

        virtual void Release() { delete this; }

    private:
        friend struct SceneNode;
        void _SetSceneNode_Internal(SceneNode* node) { mSceneNode = node; }
        void _SetRenderingOrder_Internal(RenderingOrder& order) { mRenderingOrder = mRenderingOrder; }
        RenderingOrder _GetRenderingOrder_Internal() const { return mRenderingOrder; }
        SceneNode* mSceneNode = nullptr;
        RenderingOrder mRenderingOrder = 0xFFFFFFFF;
    };

    struct GameEngineAPI Camera : public Component
    {
        virtual unsigned int GetCameraIndex() const = 0;
        virtual void SetEyePosition(const math::point3d<float>& p) = 0;
        virtual void Lookat(const math::point3d<float>& target) = 0;
        virtual void SetRenderingOrder(int order) = 0;
        virtual int GetRenderingOrder() const = 0;
        virtual void SetCameraVisibleMask(CameraVisibleMask mask) = 0;
        virtual CameraVisibleMask GetCameraVisibleMask() const = 0;
        virtual const math::float4x4& GetViewMatrixWithoutUpdate() const = 0;
        virtual const math::float4x4& GetUpdatedViewMatrix() = 0;
        //virtual math::point3d<float> ScreenToWorld(const math::point2d<int>& pos) const = 0;
        //virtual math::point2d<int> WorldToScreen(const math::point3d<float>& pos) const = 0;
    };

    struct GameEngineAPI Scene : public GEObject
    {
        virtual Camera* CreateAdditionalCameraNode() = 0;
        virtual Camera* GetDefaultCamera() = 0;
        virtual Camera* GetCameraByIndex(unsigned int index) = 0;
        virtual unsigned int GetCameraCount() const = 0;
        virtual SceneNode* CreateSceneNode() = 0;
        virtual SceneNode* GetSceneNodeByIndex(unsigned int index) = 0;
        virtual unsigned int GetSceneNodeCount() const = 0;
    };


    }
