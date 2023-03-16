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
    const Float SceneSize = 60;
    const Float SceneNear = 1;
    const Float SceneFar = SceneSize * Float(2.5);
    const Float SceneBottom = -SceneSize;
    const Float SceneTop = SceneSize;
    const Float SceneLeft = -SceneSize * aspect;
    const Float SceneRight = SceneSize * aspect;

    const Float SceneCenterX = (SceneLeft + SceneRight) * Float(0.5);
    const Float SceneCenterY = (SceneBottom + SceneTop) * Float(0.5);
    const Float SceneCenterZ = (SceneNear + SceneFar) * Float(0.5);
    const Float SceneExtendX = (SceneRight - SceneLeft) * Float(0.5);
    const Float SceneExtendY = (SceneTop - SceneBottom) * Float(0.5);
    const Float SceneExtendZ = (SceneFar - SceneNear) * Float(0.5);

    const Float SceneDistance = 5;
    const Float SmallObjectSize = 8;
    const Float BigObjectSize = SmallObjectSize * Float(1.75);

    SceneSphere* lSphere = new SceneSphere(); OutSceneObjects.push_back(lSphere);
    lSphere->SetRadius(SmallObjectSize);
    lSphere->SetTranslate(
        SceneCenterX - 20,
        SceneBottom + SmallObjectSize,
        SceneCenterZ - 5);
    lSphere->Material = MakeMatteMaterial();

    SceneSphere* metalSphere = new SceneSphere(); OutSceneObjects.push_back(metalSphere);
    metalSphere->SetRadius(16);
    metalSphere->SetTranslate(
        SceneCenterX,
        SceneCenterY,
        SceneCenterZ + 10);
    metalSphere->Material = MakeMatteMaterial();

    SceneSphere* dielectricSphereFloat = new SceneSphere(); OutSceneObjects.push_back(dielectricSphereFloat);
    dielectricSphereFloat->SetRadius(15);
    dielectricSphereFloat->SetTranslate(
        SceneCenterX + 40,
        SceneCenterY,
        SceneCenterZ + 10);
    dielectricSphereFloat->Material = MakeMatteMaterial();

    SceneSphere* dielectricSphere = new SceneSphere(); OutSceneObjects.push_back(dielectricSphere);
    dielectricSphere->SetRadius(20);
    dielectricSphere->SetTranslate(
        SceneLeft + 30,
        SceneBottom + 20,
        SceneFar - 30);
    dielectricSphere->Material = MakeMatteMaterial();

    SceneCube* lCube = new SceneCube(); OutSceneObjects.push_back(lCube);
    lCube->SetExtends(SmallObjectSize, BigObjectSize, SmallObjectSize);
    lCube->SetTranslate(
        SceneCenterX + 20 + BigObjectSize,
        SceneBottom + BigObjectSize,
        SceneCenterZ + 30);
    lCube->SetRotation(math::make_rotation_y_axis<Float>(-30_degd));
    lCube->Material = MakeMatteMaterial();

    SceneCube* rCube = new SceneCube(); OutSceneObjects.push_back(rCube);
    rCube->SetExtends(SmallObjectSize, SmallObjectSize, SmallObjectSize);
    rCube->SetTranslate(
        SceneCenterX + 15 + SmallObjectSize,
        SceneBottom + SmallObjectSize,
        SceneCenterZ + 5);
    rCube->SetRotation(math::make_rotation_y_axis<Float>(60_degd));
    rCube->Material = MakeMatteMaterial();

    Spectrum Red(Float(0.75), Float(0.2), Float(0.2));
    Spectrum Green(Float(0.2), Float(0.75), Float(0.2));
    Spectrum Blue(Float(0.2), Float(0.2), Float(0.75));
    Spectrum Gray(Float(0.75));
    Spectrum DarkGray(Float(0.6));

    SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
    wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
    wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
    wallLeft->Material = MakeMatteMaterial(Red);

    SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
    wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
    wallRight->SetRotation(math::make_rotation_y_axis<Float>(180_degd));
    wallRight->SetExtends(SceneExtendZ, SceneExtendY);
    wallRight->Material = MakeMatteMaterial(Blue);

    SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
    wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
    wallTop->SetExtends(SceneExtendZ, SceneExtendX);
    wallTop->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
    wallTop->Material = MakeMatteMaterial(Gray);

    SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
    wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
    wallBottom->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
    wallBottom->Material = MakeMatteMaterial(Green);

    SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
    wallFar->SetExtends(SceneExtendX, SceneExtendY);
    wallFar->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
    wallFar->Material = MakeMatteMaterial(DarkGray);

    SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
    LightDisk->SetTranslate(SceneCenterX, SceneTop - Float(0.01), SceneCenterZ + Float(10));
    LightDisk->SetExtends(25, 25);
    LightDisk->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
    Float Intensity = 1;
    LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
}
