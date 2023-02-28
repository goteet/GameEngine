#include "Scene.h"
#include <Foundation/Base/MemoryHelper.h>

void Transform::UpdateWorldTransform()
{
    TransformMatrix = math::matrix4x4<F>::tr(Translate, Rotation);
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

math::point3d<F> SceneRect::SampleRandomPoint(F epsilon[3]) const
{
    F e1 = (F(2) * epsilon[1] - F(1)) * Rect.width();
    F e2 = (F(2) * epsilon[2] - F(1)) * Rect.height();

    math::vector3<F> Bitangent = math::cross(mWorldNormal, mWorldTagent);
    return math::point3d<F>(mWorldPosition + e1 * mWorldTagent + e2 * Bitangent);
}

F SceneRect::SamplePdf(const SurfaceIntersection& hr, const math::ray3d<F>& ray) const
{
    if (hr.Object != this)
    {
        return F(0);
    }

    //calculate pdf(w) = pdf(x') * dist_sqr / cos_theta'
    // pdf(x') = 1 / area = > pdf(w) = dist_sqr / (area * cos_theta')
    const math::nvector3<F> Wo = -ray.direction();
    const math::nvector3<F>& N = this->mWorldNormal;
    F cosThetaPrime = math::dot(N, Wo);

    if (cosThetaPrime < -math::SMALL_NUM<F> && IsDualface())
    {
        cosThetaPrime = -cosThetaPrime;
    }

    if (cosThetaPrime <= math::SMALL_NUM<F>)
    {
        return F(0);
    }

    F area = F(4) * Rect.width() * Rect.height();
    return math::square(hr.Distance) / (area * cosThetaPrime);

}

SurfaceIntersection SceneSphere::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t0, t1;
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

    math::point3d<F> intersectPosition = ray.calc_offset(t0);
    math::vector3<F> surfaceNormal = normalized(intersectPosition - mWorldCenter);
    return SurfaceIntersection(const_cast<SceneSphere*>(this), isOnSurface, (isOnSurface ? surfaceNormal : -surfaceNormal), t0);
}

void SceneRect::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = transform(Transform.TransformMatrix, Rect.position());
    mWorldNormal = transform(Transform.TransformMatrix, Rect.normal()); // no scale so there...
    mWorldTagent = transform(Transform.TransformMatrix, Rect.tangent());
}

SurfaceIntersection SceneRect::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t;
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

SurfaceIntersection SceneDisk::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t;
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

SurfaceIntersection SceneCube::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t0, t1;
    bool front = true;

    math::vector3<F> n0, n1;
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
    for (auto* object : mSceneObjects)
    {
        object->UpdateWorldTransform();
    }
}

SurfaceIntersection Scene::DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject, F epsilon)
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

void Scene::Create(F aspect)
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

SceneObject* Scene::UniformSampleLightSource(F u)
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

F Scene::SampleLightPdf(const math::ray3d<F>& ray)
{
    SurfaceIntersection result;
    result.Distance = std::numeric_limits<F>::max();
    for (SceneObject* light : mSceneLights)
    {
        SurfaceIntersection hr = light->IntersectWithRay(ray, math::SMALL_NUM<F>);
        if (hr.Object == light && hr.Distance < result.Distance)
        {
            result = hr;
        }
    }

    return (result.Object != nullptr)
        ? result.Object->SamplePdf(result, ray) / F(mSceneLights.size())
        : F(0);
}

void Scene::CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects)
{
    const F SceneSize = 60;
    const F SceneNear = 1;
    const F SceneFar = SceneSize * F(2.5);
    const F SceneBottom = -SceneSize;
    const F SceneTop = SceneSize;
    const F SceneLeft = -SceneSize * aspect;
    const F SceneRight = SceneSize * aspect;

    const F SceneCenterX = (SceneLeft + SceneRight) * F(0.5);
    const F SceneCenterY = (SceneBottom + SceneTop) * F(0.5);
    const F SceneCenterZ = (SceneNear + SceneFar) * F(0.5);
    const F SceneExtendX = (SceneRight - SceneLeft) * F(0.5);
    const F SceneExtendY = (SceneTop - SceneBottom) * F(0.5);
    const F SceneExtendZ = (SceneFar - SceneNear) * F(0.5);

    const F SceneDistance = 5;
    const F SmallObjectSize = 8;
    const F BigObjectSize = SmallObjectSize * F(1.75);

    SceneSphere* lSphere = new SceneSphere(); OutSceneObjects.push_back(lSphere);
    lSphere->SetRadius(SmallObjectSize);
    lSphere->SetTranslate(
        SceneCenterX - 20,
        SceneBottom + SmallObjectSize,
        SceneCenterZ - 5);
    lSphere->Material.AddBSDFComponent(std::make_unique<Lambertian>());

    SceneSphere* metalSphere = new SceneSphere(); OutSceneObjects.push_back(metalSphere);
    metalSphere->SetRadius(16);
    metalSphere->SetTranslate(
        SceneCenterX,
        SceneCenterY,
        SceneCenterZ + 10);
    metalSphere->Material.AddBSDFComponent(std::make_unique<Metal>(F(0.08)));

    SceneSphere* dielectricSphereFloat = new SceneSphere(); OutSceneObjects.push_back(dielectricSphereFloat);
    dielectricSphereFloat->SetRadius(15);
    dielectricSphereFloat->SetTranslate(
        SceneCenterX + 40,
        SceneCenterY,
        SceneCenterZ + 10);
    dielectricSphereFloat->Material.AddBSDFComponent(std::make_unique<Dielectric>(F(1.5)));

    SceneSphere* dielectricSphere = new SceneSphere(); OutSceneObjects.push_back(dielectricSphere);
    dielectricSphere->SetRadius(20);
    dielectricSphere->SetTranslate(
        SceneLeft + 30,
        SceneBottom + 20,
        SceneFar - 30);
    dielectricSphere->Material.AddBSDFComponent(std::make_unique<Dielectric>(F(1.4)));

    SceneCube* lCube = new SceneCube(); OutSceneObjects.push_back(lCube);
    lCube->SetExtends(SmallObjectSize, BigObjectSize, SmallObjectSize);
    lCube->SetTranslate(
        SceneCenterX + 20 + BigObjectSize,
        SceneBottom + BigObjectSize,
        SceneCenterZ + 30);
    lCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(-30)));
    lCube->Material.AddBSDFComponent(std::make_unique<Lambertian>());

    SceneCube* rCube = new SceneCube(); OutSceneObjects.push_back(rCube);
    rCube->SetExtends(SmallObjectSize, SmallObjectSize, SmallObjectSize);
    rCube->SetTranslate(
        SceneCenterX + 15 + SmallObjectSize,
        SceneBottom + SmallObjectSize,
        SceneCenterZ + 5);
    rCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(60)));
    rCube->Material.AddBSDFComponent(std::make_unique<Lambertian>());

    SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
    wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
    wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
    wallLeft->Material.AddBSDFComponent(std::make_unique<Lambertian>(F(0.75), F(0.2), F(0.2)));

    SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
    wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
    wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
    wallRight->SetExtends(SceneExtendZ, SceneExtendY);
    wallRight->Material.AddBSDFComponent(std::make_unique<Lambertian>(F(0.2), F(0.2), F(0.75)));

    SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
    wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
    wallTop->SetExtends(SceneExtendZ, SceneExtendX);
    wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    wallTop->Material.AddBSDFComponent(std::make_unique<Lambertian>(F(0.75), F(0.75), F(0.75)));

    SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
    wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
    wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
    wallBottom->Material.AddBSDFComponent(std::make_unique<Lambertian>(F(0.2), F(0.75), F(0.2)));

    SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
    wallFar->SetExtends(SceneExtendX, SceneExtendY);
    wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
    wallFar->Material.AddBSDFComponent(std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6)));

    SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
    LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
    LightDisk->SetExtends(25, 25);
    LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    F Intensity = 1;
    LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
}
