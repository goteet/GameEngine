#pragma once
#include <memory>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Matrix.h>
#include <Foundation/Math/Geometry.h>
#include "Material.h"

struct SceneObject;

struct SurfaceIntersection
{
    SurfaceIntersection() = default;
    SurfaceIntersection(SceneObject* o, bool f, math::vector3<F> n, F d)
        : Object(o), SurfaceNormal(n), Distance(d), IsOnSurface(f)
    {

    }

    operator bool() const { return Object != nullptr; }

    SceneObject* Object = nullptr;
    bool IsOnSurface = true;
    math::normalized_vector3<F> SurfaceNormal;
    F Distance = F(0);
};

struct Transform
{
    void UpdateWorldTransform();
    math::vector3<F> Translate = math::vector3<F>::zero();
    math::quaternion<F> Rotation = math::quaternion<F>::identity();
    math::matrix4x4<F> TransformMatrix = math::matrix4x4<F>::identity();
};

struct SceneObject
{
    virtual ~SceneObject() { }
    virtual void UpdateWorldTransform();
    virtual SurfaceIntersection IntersectWithRay(const math::ray3d<F>& ray, F error) const = 0;
    void SetTranslate(F x, F y, F z) { Transform.Translate.set(x, y, z); }
    void SetRotation(const math::quaternion<F>& q) { Transform.Rotation = q; }
    virtual math::point3d<F> SampleRandomPoint(F epsilon[3]) const { return math::vector3<F>::zero(); }
    virtual F SamplePdf(const SurfaceIntersection& hr, const math::ray3d<F>& ray) const { return F(0); }
    virtual bool IsDualface() const { return false; }
    Transform Transform;
    std::unique_ptr<IMaterial> Material;
};


struct SceneSphere : SceneObject
{
    SceneSphere() : mSphere(math::point3d<F>(), 1) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetRadius(F radius) { mSphere.set_radius(radius); }
private:
    math::sphere<F> mSphere;
    math::point3d<F> mWorldCenter;
};

struct SceneRect : SceneObject
{
    SceneRect() : Rect(math::point3d<F>(),
        math::normalized_vector3<F>::unit_x(),
        math::normalized_vector3<F>::unit_z(),
        math::vector2<F>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetExtends(F x, F y) { Rect.set_extends(x, y); }
    void SetDualFace(bool dual) { mDualFace = dual; }
    virtual math::point3d<F> SampleRandomPoint(F epsilon[3]) const override;
    virtual F SamplePdf(const SurfaceIntersection& hr, const math::ray3d<F>& ray) const override;
    virtual bool IsDualface() const override { return mDualFace; }
private:
    bool mDualFace = false;
    math::rect<F> Rect;
    math::point3d<F> mWorldPosition;
    math::normalized_vector3<F> mWorldNormal;
    math::normalized_vector3<F> mWorldTagent;
};

struct SceneDisk : SceneObject
{
    SceneDisk() : Disk(math::point3d<F>(), math::normalized_vector3<F>::unit_x(), F(1)) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetRadius(F r) { Disk.set_radius(r); }
    void SetDualFace(bool dual) { mDualFace = dual; }
    virtual bool IsDualface() const override { return mDualFace; }
private:
    bool mDualFace = false;
    math::disk<F> Disk;
    math::point3d<F> mWorldPosition;
    math::vector3<F> mWorldNormal;
};

struct SceneCube : SceneObject
{
    SceneCube() : Cube(math::point3d<F>(), math::vector3<F>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetExtends(F x, F y, F z) { Cube.set_extends(x, y, z); }
private:
    math::cube<F> Cube;
    math::point3d<F> mWorldPosition;
    math::vector3<F> mWorldAxisX;
    math::vector3<F> mWorldAxisY;
    math::vector3<F> mWorldAxisZ;
};

class Scene
{
public:
    virtual ~Scene();
    void UpdateWorldTransform();
    SurfaceIntersection DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject, F epsilon);
    void Create(F aspect);
    int GetLightCount() const { return (int)mSceneLights.size(); }
    SceneObject* GetLightSourceByIndex(int index) { return mSceneLights[index]; }
    F SampleLightPdf(const math::ray3d<F>& ray);

    SceneObject* UniformSampleLightSource(F u);
private:
    virtual void CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects);
    void FindAllLights();
    std::vector<SceneObject*> mSceneObjects;
    std::vector<SceneObject*> mSceneLights;
};
