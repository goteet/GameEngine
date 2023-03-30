#include "Scene.h"
#include <Foundation/Base/MemoryHelper.h>

void Transform::UpdateWorldTransform()
{
    TransformMatrix = math::matrix4x4<Float>::tr(Translate, Rotation);
}

void SceneObject::UpdateWorldTransform()
{
    Transform.UpdateWorldTransform();
}

void SceneSphere::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();
    mWorldCenter = transform(Transform.TransformMatrix, mSphere.center());
}

Point SceneRect::SampleRandomPoint(Float epsilon[3]) const
{
    Float e1 = (Float(2) * epsilon[1] - Float(1)) * Rect.width();
    Float e2 = (Float(2) * epsilon[2] - Float(1)) * Rect.height();

    Direction Bitangent = math::cross(mWorldNormal, mWorldTagent);
    return Point(mWorldPosition + e1 * mWorldTagent + e2 * Bitangent);
}

Float SceneRect::SamplePdf(const SurfaceIntersection& hr, const Ray& ray) const
{
    if (hr.Object != this)
    {
        return Float(0);
    }

    //calculate pdf(w) = pdf(x') * dist_sqr / cos_theta'
    // pdf(x') = 1 / area = > pdf(w) = dist_sqr / (area * cos_theta')
    const Direction Wo = -ray.direction();
    const Direction& N = this->mWorldNormal;
    Float cosThetaPrime = math::dot(N, Wo);

    if (cosThetaPrime < -math::SMALL_NUM<Float> && IsDualface())
    {
        cosThetaPrime = -cosThetaPrime;
    }

    if (cosThetaPrime <= math::SMALL_NUM<Float>)
    {
        return Float(0);
    }

    Float area = Float(4) * Rect.width() * Rect.height();
    return math::square(hr.Distance) / (area * cosThetaPrime);

}

SurfaceIntersection SceneSphere::IntersectWithRay(const Ray& ray, Float error) const
{
    Float t0, t1;
    bool isOnSurface = true;
    math::intersection result = math::intersect_sphere(ray, mWorldCenter, mSphere.radius_sqr(), error, t0, t1);
    if (result == math::intersection::none)
    {
        return SurfaceIntersection();
    }
    else if (result == math::intersection::inside)
    {
        t0 = t1;
        isOnSurface = false;
    }

    Point intersectPosition = ray.calc_offset(t0);
    Direction surfaceNormal = intersectPosition - mWorldCenter;
    return SurfaceIntersection(const_cast<SceneSphere*>(this), isOnSurface, (isOnSurface ? surfaceNormal : -surfaceNormal), t0);
}

void SceneRect::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = transform(Transform.TransformMatrix, Rect.position());
    mWorldNormal = transform(Transform.TransformMatrix, Rect.normal()); // no scale so there...
    mWorldTagent = transform(Transform.TransformMatrix, Rect.tangent());
}

SurfaceIntersection SceneRect::IntersectWithRay(const Ray& ray, Float error) const
{
    Float t;
    bool isOnSurface = true;
    math::intersection result = math::intersect_rect(ray, mWorldPosition, mWorldNormal, mWorldTagent, Rect.extends(), mDualFace, error, t);
    if (result == math::intersection::none)
    {
        return SurfaceIntersection();
    }
    else
    {
        isOnSurface = math::dot(mWorldNormal, ray.direction()) < 0;
        return SurfaceIntersection(const_cast<SceneRect*>(this), isOnSurface, isOnSurface ? mWorldNormal : -mWorldNormal, t);
    }
}

void SceneDisk::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = transform(Transform.TransformMatrix, Disk.position());
    mWorldNormal = transform(Transform.TransformMatrix, Disk.normal()); // no scale so there...
}

SurfaceIntersection SceneDisk::IntersectWithRay(const Ray& ray, Float error) const
{
    Float t;
    bool isOnSurface = true;
    math::intersection result = math::intersect_disk(ray, mWorldPosition, mWorldNormal, Disk.radius(), mDualFace, error, t);
    if (result == math::intersection::none)
    {
        return SurfaceIntersection();
    }
    else
    {
        isOnSurface = math::dot(mWorldNormal, ray.direction()) < 0;
        return SurfaceIntersection(const_cast<SceneDisk*>(this), isOnSurface, isOnSurface ? mWorldNormal : -mWorldNormal, t);
    }
}

void SceneCube::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();
    mWorldPosition = transform(Transform.TransformMatrix, Cube.center());
    mWorldAxisX = transform(Transform.TransformMatrix, Cube.axis_x());
    mWorldAxisY = transform(Transform.TransformMatrix, Cube.axis_y());
    mWorldAxisZ = transform(Transform.TransformMatrix, Cube.axis_z());
}

SurfaceIntersection SceneCube::IntersectWithRay(const Ray& ray, Float error) const
{
    Float t0, t1;
    bool front = true;

    math::vector3<Float> n0, n1;
    math::intersection result = math::intersect_cube(ray, mWorldPosition,
        mWorldAxisX, mWorldAxisY, mWorldAxisZ,
        Cube.width(), Cube.height(), Cube.depth(),
        error, t0, t1, n0, n1);
    if (result == math::intersection::none)
    {
        return SurfaceIntersection();
    }
    else if (result == math::intersection::inside)
    {
        n0 = -n1;
        t0 = t1;
        front = false;
    }

    return SurfaceIntersection(const_cast<SceneCube*>(this), front, n0, t0);
}

Scene::~Scene()
{
    for (const SceneObject* obj : mSceneObjects)
    {
        SafeDelete(obj);
    }
    mSceneObjects.clear();
}

void Scene::UpdateWorldTransform()
{
    std::vector<Task> UpdateTasks;
    for (auto* object : mSceneObjects)
    {
        Task UpdateTask = Task::Start(ThreadName::Worker, [object](auto)
            {
                object->UpdateWorldTransform();
            });
        UpdateTasks.push_back(UpdateTask);
    }

    for (auto& Task : UpdateTasks)
    {
        Task.SpinWait();
    }
}

SurfaceIntersection Scene::DetectIntersecting(const Ray& ray, const SceneObject* excludeObject, Float epsilon)
{
    SurfaceIntersection result;
    for (const SceneObject* obj : mSceneObjects)
    {
        if (excludeObject != obj)
        {
            SurfaceIntersection info = obj->IntersectWithRay(ray, epsilon);
            if (info.Object != nullptr)
            {
                if (result.Object == nullptr || result.Distance > info.Distance)
                {
                    result = info;
                }
            }
        }
    }
    return result;
}

void Scene::Create(Float aspect)
{
    CreateScene(aspect, mSceneObjects);
    FindAllLights();
}

void Scene::FindAllLights()
{
    for (SceneObject* objectPtr : mSceneObjects)
    {
        if (objectPtr->LightSource != nullptr)
        {
            mSceneLights.push_back(objectPtr);
        }
    }
}

SceneObject* Scene::UniformSampleLightSource(Float u)
{
    if (mSceneLights.size() > 0)
    {
        uint32_t length = (uint32_t)mSceneLights.size();
        uint32_t index = math::min2<uint32_t>(math::floor2<uint32_t>(u * length), length - 1);
        return mSceneLights[index];
    }
    else
    {
        return nullptr;
    }
}

Float Scene::SampleLightPdf(const Ray& ray)
{
    SurfaceIntersection result;
    result.Distance = std::numeric_limits<Float>::max();
    for (SceneObject* light : mSceneLights)
    {
        SurfaceIntersection hr = light->IntersectWithRay(ray, math::SMALL_NUM<Float>);
        if (hr.Object == light && hr.Distance < result.Distance)
        {
            result = hr;
        }
    }

    return (result.Object != nullptr)
        ? result.Object->SamplePdf(result, ray) / Float(mSceneLights.size())
        : Float(0);
}

void Scene::CreateScene(Float aspect, std::vector<SceneObject*>& OutSceneObjects)
{

}
