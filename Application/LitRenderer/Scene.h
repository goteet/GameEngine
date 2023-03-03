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
    SurfaceIntersection(SceneObject* o, bool f, Direction n, Float d)
        : Object(o), SurfaceNormal(n), Distance(d), IsOnSurface(f)
    {

    }

    operator bool() const { return Object != nullptr; }

    SceneObject* Object = nullptr;
    bool IsOnSurface = true;
    Direction SurfaceNormal;
    Float Distance = Float(0);
};

struct Transform
{
    void UpdateWorldTransform();
    math::vector3<Float> Translate = math::vector3<Float>::zero();
    math::quaternion<Float> Rotation = math::quaternion<Float>::identity();
    math::matrix4x4<Float> TransformMatrix = math::matrix4x4<Float>::identity();
};


struct LightSource
{
    LightSource(Float r, Float g, Float b) : Emission(r,g,b) { }

    const Spectrum& Le() const { return Emission; }

private:
    Spectrum Emission = Spectrum::one();
};

struct SceneObject
{
    virtual ~SceneObject() { }
    virtual void UpdateWorldTransform();
    virtual SurfaceIntersection IntersectWithRay(const Ray& ray, Float error) const = 0;
    void SetTranslate(Float x, Float y, Float z) { Transform.Translate.set(x, y, z); }
    void SetRotation(const math::quaternion<Float>& q) { Transform.Rotation = q; }
    virtual Point SampleRandomPoint(Float epsilon[3]) const { return Point::zero(); }
    virtual Float SamplePdf(const SurfaceIntersection& hr, const Ray& ray) const { return Float(0); }
    virtual bool IsDualface() const { return false; }
    Transform Transform;
    std::unique_ptr<Material> Material;
    std::unique_ptr<LightSource> LightSource = nullptr;
};


struct SceneSphere : SceneObject
{
    SceneSphere() : mSphere(Point(), 1) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const Ray& ray, Float error) const override;
    void SetRadius(Float radius) { mSphere.set_radius(radius); }
private:
    math::sphere<Float> mSphere;
    Point mWorldCenter;
};

struct SceneRect : SceneObject
{
    SceneRect() : Rect(Point(),
        Direction::unit_x(),
        Direction::unit_z(),
        math::vector2<Float>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const Ray& ray, Float error) const override;
    void SetExtends(Float x, Float y) { Rect.set_extends(x, y); }
    void SetDualFace(bool dual) { mDualFace = dual; }
    virtual Point SampleRandomPoint(Float epsilon[3]) const override;
    virtual Float SamplePdf(const SurfaceIntersection& hr, const Ray& ray) const override;
    virtual bool IsDualface() const override { return mDualFace; }
private:
    bool mDualFace = false;
    math::rect<Float> Rect;
    Point mWorldPosition;
    Direction mWorldNormal;
    Direction mWorldTagent;
};

struct SceneDisk : SceneObject
{
    SceneDisk() : Disk(Point(), Direction::unit_x(), Float(1)) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const Ray& ray, Float error) const override;
    void SetRadius(Float r) { Disk.set_radius(r); }
    void SetDualFace(bool dual) { mDualFace = dual; }
    virtual bool IsDualface() const override { return mDualFace; }
private:
    bool mDualFace = false;
    math::disk<Float> Disk;
    Point mWorldPosition;
    Direction mWorldNormal;
};

struct SceneCube : SceneObject
{
    SceneCube() : Cube(Point(), math::vector3<Float>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual SurfaceIntersection IntersectWithRay(const Ray& ray, Float error) const override;
    void SetExtends(Float x, Float y, Float z) { Cube.set_extends(x, y, z); }
private:
    math::cube<Float> Cube;
    Point mWorldPosition;
    Direction mWorldAxisX;
    Direction mWorldAxisY;
    Direction mWorldAxisZ;
};

class Scene
{
public:
    virtual ~Scene();
    void UpdateWorldTransform();
    SurfaceIntersection DetectIntersecting(const Ray& ray, const SceneObject* excludeObject, Float epsilon);
    void Create(Float aspect);
    int GetLightCount() const { return (int)mSceneLights.size(); }
    SceneObject* GetLightSourceByIndex(int index) { return mSceneLights[index]; }
    Float SampleLightPdf(const Ray& ray);

    SceneObject* UniformSampleLightSource(Float u);
private:
    virtual void CreateScene(Float aspect, std::vector<SceneObject*>& OutSceneObjects);
    void FindAllLights();
    std::vector<SceneObject*> mSceneObjects;
    std::vector<SceneObject*> mSceneLights;
};
