#pragma once
#include <Foundation/Math/Matrix.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>
#include "GEPredefinedMacros.h"

namespace GE
{
    using CameraVisibleMask = unsigned int;
    using RenderingOrder = unsigned int;
    using ComponentOrder = int;
    struct SceneNode;
    struct Component;
    struct Scene;

    constexpr CameraVisibleMask DefaultCameraVisibkeMask = 0xFFFFFFFF;
    constexpr ComponentOrder DefaultComponentOrder = 0x5000;


    struct GameEngineAPI Component : public GEObject
    {
        SceneNode* GetSceneNode() const { return mAttachNode; }

        math::point3d<float> GetSceneNodePosition() const;
        math::point3d<float> GetSceneNodeWorldPosition() const;
        math::radian<float> GetSceneNodeRotation() const;
        math::float3 GetSceneNodeScale() const;
        CameraVisibleMask GetCameraVisibleMask() const;
        //virtual const aabb2d<float>& GetLocalAABB() const { static aabb2d<float> b; return b; }
        //virtual aabb2d<float> GetWorldAABB() const;
        virtual ComponentOrder GetExecuteOrder() const { return DefaultComponentOrder; }

        /********************************************************************
        * Event Listener
        *********************************************************************/
        virtual void OnInitial() { }
        virtual void OnUpdate(unsigned int deltaTime) { }
        virtual void OnRender() { }
        virtual void OnPositionChanging(const math::point3d<float>& newPos) { }
        virtual void OnPositionChanged(const math::point3d<float>& newPos) { }
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

        virtual void Release() = 0;

    private:
        //friend struct Scene;
        //void _SetRenderingOrder_Internal(RenderingOrder& order);
        //void _SetSceneNode_Internal(SceneNode* node);
        RenderingOrder _GetRenderingOrder_Internal() const { return mRenderingOrder; }
        SceneNode* mAttachNode = nullptr;
        RenderingOrder mRenderingOrder = 0xFFFFFFFF;
    };

    struct GameEngineAPI Camera : public Component
    {
    };

    enum AutoReleaseComponentHint { AutoReleaseComponent };
    enum RecursiveSetCameraVisibleMaskHint { RecursiveCameraVisibleMask };
    struct GameEngineAPI SceneNode : public GEObject
    {
        virtual Scene* GetScene() = 0;
        virtual SceneNode* GetPrevSiblingNode() = 0;
        virtual SceneNode* GetNextSiblingNode() = 0;
        //virtual SceneNode* GetParentNode() = 0;
        //virtual SceneNode* GetFirstChild() = 0;
        //virtual SceneNode* GetLastChild() = 0;
        //virtual SceneNode* GetChildByIndex(unsigned int index) = 0;
        //virtual unsigned int GetChildCount() const = 0;
        //virtual SceneNode* CreateChild() = 0;
        virtual void MoveToFirst() = 0;
        virtual void MoveToLast() = 0;
        virtual void MovePrev() = 0;
        virtual void MoveNext() = 0;
        virtual bool AddComponent(Component*) = 0;
        virtual bool AddComponent(Component*, AutoReleaseComponentHint) = 0;
        virtual bool RemoveComponent(Component*) = 0;
        virtual bool RemoveComponent(Component*, AutoReleaseComponentHint) = 0;
        virtual bool HasComponent(Component*) const = 0;
        virtual bool IsComponentAutoRelease(Component*) const = 0;
        virtual Component* GetComponentByIndex(unsigned int index) = 0;
        virtual unsigned int GetComponentCount() const = 0;
        virtual const math::float4x4& GetLocalMatrix() = 0;
        virtual const math::float4x4& GetWorldMatrix() = 0;
        virtual SceneNode* SetPosition(const math::point3d<float>& Position) = 0;
        virtual math::point3d<float> GetPosition() const = 0;
        virtual SceneNode* SetWorldPosition(const math::point3d<float>& Position) = 0;
        virtual math::point3d<float> GetWorldPosition() = 0;
        virtual SceneNode* SetRightDirection(const math::float3& right) = 0;
        virtual const math::float3 GetRightDirection() = 0;
        virtual SceneNode* SetUpDirection(const math::float3& up) = 0;
        virtual const math::float3 GetUpDirection() = 0;
        virtual SceneNode* SetForwardDirection(const math::float3& up) = 0;
        virtual const math::float3 GetForwardDirection() = 0;
        virtual SceneNode* SetLocalScale(const math::float3& scale) = 0;
        virtual const math::float3& GetLocalScale() const = 0;
        //virtual SceneNode* SetLocalRotation(math::radian<float> r) = 0;
        //virtual math::radian<float> GetLocalRotation() const = 0;
        virtual void SetVisible(bool) = 0;
        virtual bool IsVisible() const = 0;
        virtual void SetStatic(bool) = 0;
        virtual bool IsStatic() const = 0;
        virtual void SetCameraVisibleMask(CameraVisibleMask mask, RecursiveSetCameraVisibleMaskHint) = 0;
        virtual CameraVisibleMask GetCameraVisibleMask() const = 0;
        //virtual unsigned int GetChildIndex() const = 0;
        virtual unsigned int GetIndex() const = 0;
        //virtual bool IsRemoved() const = 0;
        virtual math::point3d<float> WorldToLocal(const math::point3d<float>& pos) = 0;
        virtual math::point3d<float> LocalToWorld(const math::point3d<float>& pos) = 0;
        //virtual math::point3d<float> WorldToParent(const math::point3d<float>& pos) = 0;

        virtual void Release() = 0;
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
