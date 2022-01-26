#pragma once
#include <vector>
#include "GEInclude.h"

namespace engine
{
    class Scene;

    class SceneNode : public GE::SceneNode
    {
        static math::float4x4 dummy;
    public:
        DefineRTTI;
        virtual GE::Scene* GetScene() override;
        virtual SceneNode* GetPrevSiblingNode() override { return nullptr; }
        virtual SceneNode* GetNextSiblingNode() override { return nullptr; }
        virtual void MoveToFirst() override { }
        virtual void MoveToLast() override { }
        virtual void MovePrev() override { }
        virtual void MoveNext() override { }
        virtual bool AddComponent(GE::Component*) override { return false; }
        virtual bool AddComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool RemoveComponent(GE::Component*) override { return false; }
        virtual bool RemoveComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool HasComponent(GE::Component*) const override { return false; }
        virtual bool IsComponentAutoRelease(GE::Component*) const override { return false; }
        virtual GE::Component* GetComponentByIndex(unsigned int index) override { return nullptr; }
        virtual unsigned int GetComponentCount() const override { return 0; }
        virtual const math::float4x4& GetLocalMatrix() override { return dummy; }
        virtual const math::float4x4& GetWorldMatrix() override { return dummy; }
        virtual SceneNode* SetPosition(const math::point3d<float>& Position) override { return this; }
        virtual math::point3d<float> GetPosition() const override { return math::point3d<float>::zero(); }
        virtual SceneNode* SetWorldPosition(const math::point3d<float>& Position) override { return this; }
        virtual math::point3d<float> GetWorldPosition() override { return math::point3d<float>::zero(); }
        virtual SceneNode* SetRightDirection(const math::float3& right)  override { return this; }
        virtual const math::float3 GetRightDirection() override { return math::float3::unit_x(); }
        virtual SceneNode* SetUpDirection(const math::float3& up) override { return this; }
        virtual const math::float3 GetUpDirection() override { return math::float3::unit_y(); }
        virtual SceneNode* SetForwardDirection(const math::float3& up) override { return this; }
        virtual const math::float3 GetForwardDirection() override { return math::float3::unit_z(); }
        virtual SceneNode* SetLocalScale(const math::float3& scale) override { return this; }
        virtual const math::float3 GetLocalScale() const override { return math::float3::one(); }
        virtual void SetVisible(bool) override { }
        virtual bool IsVisible() const override { return true; }
        virtual void SetStatic(bool) override { }
        virtual bool IsStatic() const override { return true; }
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask, GE::RecursiveSetCameraVisibleMaskHint) override { }
        virtual GE::CameraVisibleMask GetCameraVisibleMask() const override { return GE::CameraVisibleMask(); }
        virtual unsigned int GetIndex() const override { return 0; }
        virtual math::point3d<float> WorldToLocal(const math::point3d<float>& pos) override { return pos; }
        virtual math::point3d<float> LocalToWorld(const math::point3d<float>& pos) override { return pos; }

    protected:
        SceneNode(Scene* scene, SceneNode* parent)
            : mScene(scene)
            , mParent(parent)
        {

        }

    private:
        engine::Scene* mScene = nullptr;
        SceneNode* mParent = nullptr;
        std::vector<SceneNode*> mChildren;
    };

    class InvalidateRootSceneNode final : public SceneNode
    {
        DefineRTTI;
    private:
        static math::float4x4 dummy;
        virtual SceneNode* GetPrevSiblingNode() override { return nullptr; }
        virtual SceneNode* GetNextSiblingNode() override { return nullptr; }
        virtual void MoveToFirst() override { }
        virtual void MoveToLast() override { }
        virtual void MovePrev() override { }
        virtual void MoveNext() override { }
        virtual bool AddComponent(GE::Component*) override { return false; }
        virtual bool AddComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool RemoveComponent(GE::Component*) override { return false; }
        virtual bool RemoveComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool HasComponent(GE::Component*) const override { return false; }
        virtual bool IsComponentAutoRelease(GE::Component*) const override { return false; }
        virtual GE::Component* GetComponentByIndex(unsigned int index) override { return nullptr; }
        virtual unsigned int GetComponentCount() const override { return 0; }
        virtual const math::float4x4& GetLocalMatrix() override { return dummy; }
        virtual const math::float4x4& GetWorldMatrix() override { return dummy; }
        virtual SceneNode* SetPosition(const math::point3d<float>& Position) override { return this; }
        virtual math::point3d<float> GetPosition() const override { return math::point3d<float>::zero(); }
        virtual SceneNode* SetWorldPosition(const math::point3d<float>& Position) override { return this; }
        virtual math::point3d<float> GetWorldPosition() override { return math::point3d<float>::zero(); }
        virtual SceneNode* SetRightDirection(const math::float3& right)  override { return this; }
        virtual const math::float3 GetRightDirection() override { return math::float3::unit_x(); }
        virtual SceneNode* SetUpDirection(const math::float3& up) override { return this; }
        virtual const math::float3 GetUpDirection() override { return math::float3::unit_y(); }
        virtual SceneNode* SetForwardDirection(const math::float3& up) override { return this; }
        virtual const math::float3 GetForwardDirection() override { return math::float3::unit_z(); }
        virtual SceneNode* SetLocalScale(const math::float3& scale) override { return this; }
        virtual const math::float3 GetLocalScale() const override{ return math::float3::one(); }
        virtual void SetVisible(bool) override { }
        virtual bool IsVisible() const override { return true; }
        virtual void SetStatic(bool) override { } 
        virtual bool IsStatic() const override { return true; }
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask, GE::RecursiveSetCameraVisibleMaskHint) override { }
        virtual GE::CameraVisibleMask GetCameraVisibleMask() const override { return GE::CameraVisibleMask(); }
        virtual unsigned int GetIndex() const override { return 0; }
        virtual math::point3d<float> WorldToLocal(const math::point3d<float>& pos) override { return pos; }
        virtual math::point3d<float> LocalToWorld(const math::point3d<float>& pos) override { return pos; }

    public:
        InvalidateRootSceneNode(Scene* scene);
    };
}
