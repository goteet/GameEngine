#include "Scene.h"
#include <Foundation/Base/MemoryHelper.h>

void Transform::UpdateWorldTransform()
{
    TransformMatrix = math::matrix4x4<Float>::tr(Translate, Rotation);
    TransformMatrix3x3 = math::convert_to_matrix3x3(TransformMatrix);
    TransformMatrix3x3T = math::transposed(TransformMatrix3x3);
    TransformMatrix3x3InverseT = math::inversed(TransformMatrix3x3T);
}

Direction Transform::InverseTransformNormal(const Direction& direction) const
{
    return math::transform(TransformMatrix3x3T, direction);
}

Direction Transform::TransformNormal(const Direction& direction) const
{
    return math::transform(TransformMatrix3x3InverseT, direction);
}

Direction Transform::TransformDirection(const Direction& direction) const
{
    return math::transform(TransformMatrix3x3, direction);
}

Point Transform::TransformPoint(const Point& Point) const
{
    return math::transform(TransformMatrix, Point);
}

void SceneObject::UpdateWorldTransform()
{
    WorldTransform.UpdateWorldTransform();
}

Direction SceneObject::WorldToLocalNormal(const Direction& direction) const
{
    return WorldTransform.InverseTransformNormal(direction);
}

Direction SceneObject::LocalToWorldNormal(const Direction& direction) const
{
    return WorldTransform.TransformNormal(direction);
}

void SceneSphere::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();
    mWorldCenter = WorldTransform.TransformPoint(mSphere.center());
}

Point SceneRect::SampleRandomPoint(Float epsilon[3]) const
{
    Float e1 = (Float(2) * epsilon[1] - Float(1)) * Rect.width();
    Float e2 = (Float(2) * epsilon[2] - Float(1)) * Rect.height();

    Direction Bitangent = math::cross(mWorldNormal, mWorldTangent);
    return Point(mWorldPosition + e1 * mWorldTangent + e2 * Bitangent);
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

    Direction localNormal = WorldToLocalNormal(surfaceNormal);
    //{dPx/dPhi, dPy/dPhi, dPz/dPhi}; 
    //it should be: { -sinTheta*sinPhi, sinTheta*cosPhi, 0 }
    // sinPhi = -y / (x*x + y*y)  --> -y
    // cosPhi =  x / (x*x + y*y)  -->  x
    //and after normalized, it can be:
    bool bIsPolaPoint = math::near_one_length(localNormal.y);
    Direction localTangent = bIsPolaPoint
        ? Direction(Float(1), Float(0), Float(0))
        : Direction(-localNormal.z, 0, localNormal.x);
    Direction surfaceTangent = LocalToWorldNormal(localTangent);
    return SurfaceIntersection(const_cast<SceneSphere*>(this), isOnSurface,
        (isOnSurface ? surfaceNormal : -surfaceNormal),
        (isOnSurface ? surfaceTangent : -surfaceTangent), t0);
}

void SceneRect::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = WorldTransform.TransformPoint(Rect.position());
    mWorldNormal = WorldTransform.TransformNormal(Rect.normal()); // no scale so there...
    mWorldTangent = WorldTransform.TransformDirection(Rect.tangent());
}

SurfaceIntersection SceneRect::IntersectWithRay(const Ray& ray, Float error) const
{
    Float t;
    bool isOnSurface = true;
    math::intersection result = math::intersect_rect(ray, mWorldPosition, mWorldNormal, mWorldTangent, Rect.extends(), mDualFace, error, t);
    if (result == math::intersection::none)
    {
        return SurfaceIntersection();
    }
    else
    {
        isOnSurface = math::dot(mWorldNormal, ray.direction()) < 0;
        return SurfaceIntersection(const_cast<SceneRect*>(this), isOnSurface,
            (isOnSurface ? mWorldNormal : -mWorldNormal),
            (isOnSurface ? mWorldTangent : -mWorldTangent), t);
    }
}

void SceneDisk::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = WorldTransform.TransformPoint(Disk.position());
    mWorldNormal = WorldTransform.TransformNormal(Disk.normal()); // no scale so there...
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
        Direction worldTangent = ray.calc_offset(t) - mWorldPosition;
        return SurfaceIntersection(const_cast<SceneDisk*>(this), isOnSurface,
            (isOnSurface ? mWorldNormal : -mWorldNormal),
            (isOnSurface ? worldTangent : -worldTangent), t);
    }
}

void SceneCube::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();
    mWorldPosition = WorldTransform.TransformPoint(Cube.center());
    mWorldAxisX = WorldTransform.TransformDirection(Cube.axis_x());
    mWorldAxisY = WorldTransform.TransformDirection(Cube.axis_y());
    mWorldAxisZ = WorldTransform.TransformDirection(Cube.axis_z());
}

SurfaceIntersection SceneCube::IntersectWithRay(const Ray& ray, Float error) const
{
    Float t0, t1;
    bool front = true;

    Direction n0, n1;
    Direction tangent0 = Direction::unit_x();
    Direction tangent1 = Direction::unit_x();
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
        tangent0 = tangent1;
        front = false;
    }

    return SurfaceIntersection(const_cast<SceneCube*>(this), front, n0, tangent0, t0);
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