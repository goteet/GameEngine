#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"
#include "Integrator.h"

namespace
{
    class SimpleScene : public Scene
    {
        virtual void CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects) override
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

            SceneSphere* mainSphere = new SceneSphere(); OutSceneObjects.push_back(mainSphere);
            mainSphere->SetRadius(20);
            mainSphere->SetTranslate(
                SceneCenterX,
                SceneBottom + 22,
                SceneCenterZ + 10);
            //mainSphere->Material = std::make_unique<Glossy>(1, 0.85, 0.5, 2.5);
            //mainSphere->Material = std::make_unique<GGX>(0.1);
            mainSphere->Material = std::make_unique<Lambertian>(F(0.5), F(0.5), F(0.5));

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
            wallLeft->Material = std::make_unique<Lambertian>(F(0.75), F(0.2), F(0.2));

            SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
            wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
            wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
            wallRight->SetExtends(SceneExtendZ, SceneExtendY);
            wallRight->Material = std::make_unique<Lambertian>(F(0.2), F(0.2), F(0.75));

            SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
            wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
            wallTop->SetExtends(SceneExtendZ, SceneExtendX);
            wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            wallTop->Material = std::make_unique<Lambertian>(F(0.75), F(0.75), F(0.75));

            SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
            wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
            wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
            wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
            wallBottom->Material = std::make_unique<Lambertian>(F(0.2), F(0.75), F(0.2));

            SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
            wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
            wallFar->SetExtends(SceneExtendX, SceneExtendY);
            wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
            wallFar->Material = std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6));

            SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
            LightDisk->SetDualFace(true);
            LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
            LightDisk->SetExtends(25, 25);
            LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            F Intensity = 4.5;
            LightDisk->Material = std::make_unique<PureLight_ForTest>(Intensity, Intensity, Intensity);
        }
    };

    class OrenNayerComparisionScene : public Scene
    {
        virtual void CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects) override
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

            SceneSphere* LambertianShere = new SceneSphere(); OutSceneObjects.push_back(LambertianShere);
            LambertianShere->SetRadius(20);
            LambertianShere->SetTranslate(
                SceneCenterX + 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            LambertianShere->Material = std::make_unique<Lambertian>(1, 1, 1);


            SceneSphere* orenNayerSphere = new SceneSphere(); OutSceneObjects.push_back(orenNayerSphere);
            orenNayerSphere->SetRadius(20);
            orenNayerSphere->SetTranslate(
                SceneCenterX - 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            orenNayerSphere->Material = std::make_unique<OrenNayer>(1, 1, 1, math::radian<F>(0.25));

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
            wallLeft->Material = std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6));

            SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
            wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
            wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
            wallRight->SetExtends(SceneExtendZ, SceneExtendY);
            wallRight->Material = std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6));

            SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
            wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
            wallTop->SetExtends(SceneExtendZ, SceneExtendX);
            wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            wallTop->Material = std::make_unique<Lambertian>(F(0.75), F(0.75), F(0.75));

            SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
            wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
            wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
            wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
            wallBottom->Material = std::make_unique<Lambertian>(F(0.2), F(0.2), F(0.75));

            SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
            wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
            wallFar->SetExtends(SceneExtendX, SceneExtendY);
            wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
            wallFar->Material = std::make_unique<Lambertian>(F(0.75), F(0.2), F(0.2));

            SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
            LightDisk->SetDualFace(true);
            LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
            LightDisk->SetExtends(25, 25);
            LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            F Intensity = 4.5;
            LightDisk->Material = std::make_unique<PureLight_ForTest>(Intensity, Intensity, Intensity);
        }
    };

}

math::point3d<F> SceneRect::SampleRandomPoint(F epsilon[3]) const
{
    F e1 = (F(2) * epsilon[1] - F(1)) * Rect.width();
    F e2 = (F(2) * epsilon[2] - F(1)) * Rect.height();

    math::vector3<F> Bitangent = math::cross(mWorldNormal, mWorldTagent);
    return math::point3d<F>(mWorldPosition + e1 * mWorldTagent + e2 * Bitangent);
}

F SceneRect::SamplePdf(const HitRecord& hr, const math::ray3d<F>& ray) const
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

LitRenderer::LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch)
    : mCanvasLinePitch(canvasLinePitch)
    , mSystemCanvasDataPtr(canvasDataPtr)
    , mFilm(canvasWidth, canvasHeight)
    , mClearColor(0, 0, 0)
    , mCamera(math::degree<F>(50))
    , mScene(std::make_unique<SimpleScene>())
{
    mScene->Create(F(canvasWidth) / F(canvasHeight));
    int sampleCount = canvasWidth * canvasHeight;
    for (int index = 0; index < MaxCameraRaySampleCount; index++)
    {
        mCameraRaySamples[index] = new Sample[sampleCount];
    }
}

LitRenderer::~LitRenderer()
{
    for (int index = 0; index < MaxCameraRaySampleCount; index++)
    {
        SafeDeleteArray(mCameraRaySamples[index]);
    }
}

void LitRenderer::InitialSceneTransforms()
{
    mScene->UpdateWorldTransform();
}

void LitRenderer::GenerateCameraRays()
{
    const F PixelSize = F(0.1);
    const F HalfPixelSize = PixelSize * F(0.5);
    F halfWidth = mFilm.CanvasWidth * F(0.5) * PixelSize;
    F halfHeight = mFilm.CanvasHeight * F(0.5) * PixelSize;
    //     <---> (half height)
    //  .  o----. (o=origin)
    //  |  |  /   Asumed canvas is at origin(0,0,0),
    //  |  | /    and camera is placed at neg-z-axis,
    //  .  |/
    // (z) .      tan(half_fov) = halfHeight / cameraZ.
    F cameraZ = halfHeight / mCamera.HalfVerticalFovTangent;
    mCamera.Position.z = -cameraZ;

    auto canvasPositionToRay = [&cameraZ](F x, F y) -> math::vector3<F> {
        //vector3<float>(x,y,0) - camera.position;
        //  x = x - 0;
        //  y = y - 0;
        //  z = 0 - camera.position.z
        return math::vector3<F>(x, y, cameraZ);
    };

    const F QuaterPixelSize = F(0.5) * HalfPixelSize;
    math::vector3<F>* canvasDataPtr = mFilm.GetBackbufferPtr();
    random<F> RandomGeneratorPickingPixel;
    for (int rowIndex = 0; rowIndex < mFilm.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mFilm.CanvasWidth;
        for (int colIndex = 0; colIndex < mFilm.CanvasWidth; colIndex++)
        {
            const F pixelCenterX = colIndex * PixelSize + HalfPixelSize - halfWidth;
            const F pixelCenterY = rowIndex * PixelSize + HalfPixelSize - halfHeight;

            for (int index = 0; index < MaxCameraRaySampleCount; ++index)
            {
                Sample& samples = mCameraRaySamples[index][colIndex + rowOffset];
                samples.PixelRow = rowIndex;
                samples.PixelCol = colIndex;

                F x = F(2) * RandomGeneratorPickingPixel.value() - F(1);
                F y = F(2) * RandomGeneratorPickingPixel.value() - F(1);
                samples.Ray.set_origin(mCamera.Position);
                samples.Ray.set_direction(canvasPositionToRay(pixelCenterX + x * HalfPixelSize, pixelCenterY + y * HalfPixelSize));
            }
        }
    }
}

void LitRenderer::QueryP1Records()
{
    for (int rowIndex = 0; rowIndex < mFilm.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mFilm.CanvasWidth;
        for (int colIndex = 0; colIndex < mFilm.CanvasWidth; colIndex++)
        {
            for (int index = 0; index < MaxCameraRaySampleCount; ++index)
            {
                Sample& samples = mCameraRaySamples[index][colIndex + rowOffset];
                samples.RecordP1 = mScene->DetectIntersecting(samples.Ray, nullptr, math::SMALL_NUM<F>);
            }
        }
    }
}

void LitRenderer::Initialize()
{
    InitialSceneTransforms();
    GenerateCameraRays();
    QueryP1Records();
}

void LitRenderer::GenerateImageProgressive()
{
    if (MaxSampleCount <= 0 || (mFilm.GetSampleCount() <= MaxSampleCount))
    {
        ResolveSamples();
        mNeedFlushBackbuffer = true;
    }
}

bool LitRenderer::NeedUpdate()
{
    if (mNeedFlushBackbuffer)
    {
        mFilm.FlushTo(mSystemCanvasDataPtr, mCanvasLinePitch);// LinearColorToGammaCorrectedCanvasDataBuffer();
        mNeedFlushBackbuffer = false;
        mNeedUpdateSystemWindowRect = true;
    }
    return mNeedUpdateSystemWindowRect;
}

void LitRenderer::ResolveSamples()
{
    const Sample* Samples = mCameraRaySamples[mCurrentCameraRayIndex];
    mCurrentCameraRayIndex = (mCurrentCameraRayIndex + 1) % MaxCameraRaySampleCount;
    math::vector3<F>* accumulatedBufferPtr = mFilm.GetBackbufferPtr();
    PathIntegrator integrator;
    for (int rowIndex = 0; rowIndex < mFilm.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mFilm.CanvasWidth;
        for (int colIndex = 0; colIndex < mFilm.CanvasWidth; colIndex++)
        {
            const Sample& sample = Samples[colIndex + rowOffset];
            math::vector3<F>& canvasPixel = accumulatedBufferPtr[colIndex + rowOffset];
            canvasPixel += integrator.EvaluateLi(*mScene, sample.Ray, sample.RecordP1);
        }
    }
    mFilm.IncreaseSampleCount();
}

SimpleBackCamera::SimpleBackCamera(math::degree<F> verticalFov)
    : HalfVerticalFov(DegreeClampHelper(verticalFov).value* F(0.5))
    , HalfVerticalFovTangent(math::tan(math::degree<F>(DegreeClampHelper(verticalFov).value* F(0.5))))
    , Position(0, 0, 0)
{ }


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

HitRecord Scene::DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject, F epsilon)
{
    HitRecord result;
    for (const SceneObject* obj : mSceneObjects)
    {
        if (excludeObject != obj)
        {
            HitRecord info = obj->IntersectWithRay(ray, epsilon);
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
        PureLight_ForTest* lightMaterial = dynamic_cast<PureLight_ForTest*>(objectPtr->Material.get());
        if (lightMaterial != nullptr)
        {
            mSceneLights.push_back(objectPtr);
        }
    }
}

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

HitRecord SceneSphere::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t0, t1;
    bool isOnSurface = true;
    math::intersection result = math::intersect_sphere(ray, mWorldCenter, mSphere.radius_sqr(), error, t0, t1);
    if (result == math::intersection::none)
    {
        return HitRecord();
    }
    else if (result == math::intersection::inside)
    {
        t0 = t1;
        isOnSurface = false;
    }

    math::point3d<F> intersectPosition = ray.calc_offset(t0);
    math::vector3<F> surfaceNormal = normalized(intersectPosition - mWorldCenter);
    return HitRecord(const_cast<SceneSphere*>(this), isOnSurface, (isOnSurface ? surfaceNormal : -surfaceNormal), t0);
}

void SceneRect::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = transform(Transform.TransformMatrix, Rect.position());
    mWorldNormal = transform(Transform.TransformMatrix, Rect.normal()); // no scale so there...
    mWorldTagent = transform(Transform.TransformMatrix, Rect.tangent());
}

HitRecord SceneRect::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t;
    bool isOnSurface = true;
    math::intersection result = math::intersect_rect(ray, mWorldPosition, mWorldNormal, mWorldTagent, Rect.extends(), mDualFace, error, t);
    if (result == math::intersection::none)
    {
        return HitRecord();
    }
    else
    {
        isOnSurface = math::dot(mWorldNormal, ray.direction()) < 0;
        return HitRecord(const_cast<SceneRect*>(this), isOnSurface, isOnSurface ? mWorldNormal : -mWorldNormal, t);
    }
}

void SceneDisk::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = transform(Transform.TransformMatrix, Disk.position());
    mWorldNormal = transform(Transform.TransformMatrix, Disk.normal()); // no scale so there...
}

HitRecord SceneDisk::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t;
    bool isOnSurface = true;
    math::intersection result = math::intersect_disk(ray, mWorldPosition, mWorldNormal, Disk.radius(), mDualFace, error, t);
    if (result == math::intersection::none)
    {
        return HitRecord();
    }
    else
    {
        isOnSurface = math::dot(mWorldNormal, ray.direction()) < 0;
        return HitRecord(const_cast<SceneDisk*>(this), isOnSurface, isOnSurface ? mWorldNormal : -mWorldNormal, t);
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

HitRecord SceneCube::IntersectWithRay(const math::ray3d<F>& ray, F error) const
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
        return HitRecord();
    }
    else if (result == math::intersection::inside)
    {
        n0 = -n1;
        t0 = t1;
        front = false;
    }

    return HitRecord(const_cast<SceneCube*>(this), front, n0, t0);
}


F Scene::SampleLightPdf(const math::ray3d<F>& ray)
{
    HitRecord result;
    result.Distance = std::numeric_limits<F>::max();
    for (SceneObject* light : mSceneLights)
    {
        HitRecord hr = light->IntersectWithRay(ray, math::SMALL_NUM<F>);
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
    lSphere->Material = std::make_unique<Lambertian>();

    SceneSphere* metalSphere = new SceneSphere(); OutSceneObjects.push_back(metalSphere);
    metalSphere->SetRadius(16);
    metalSphere->SetTranslate(
        SceneCenterX,
        SceneCenterY,
        SceneCenterZ + 10);
    metalSphere->Material = std::make_unique<Metal>(F(0.08));

    SceneSphere* dielectricSphereFloat = new SceneSphere(); OutSceneObjects.push_back(dielectricSphereFloat);
    dielectricSphereFloat->SetRadius(15);
    dielectricSphereFloat->SetTranslate(
        SceneCenterX + 40,
        SceneCenterY,
        SceneCenterZ + 10);
    dielectricSphereFloat->Material = std::make_unique<Dielectric>(F(1.5));

    SceneSphere* dielectricSphere = new SceneSphere(); OutSceneObjects.push_back(dielectricSphere);
    dielectricSphere->SetRadius(20);
    dielectricSphere->SetTranslate(
        SceneLeft + 30,
        SceneBottom + 20,
        SceneFar - 30);
    dielectricSphere->Material = std::make_unique<Dielectric>(F(1.4));

    SceneCube* lCube = new SceneCube(); OutSceneObjects.push_back(lCube);
    lCube->SetExtends(SmallObjectSize, BigObjectSize, SmallObjectSize);
    lCube->SetTranslate(
        SceneCenterX + 20 + BigObjectSize,
        SceneBottom + BigObjectSize,
        SceneCenterZ + 30);
    lCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(-30)));
    lCube->Material = std::make_unique<Lambertian>();

    SceneCube* rCube = new SceneCube(); OutSceneObjects.push_back(rCube);
    rCube->SetExtends(SmallObjectSize, SmallObjectSize, SmallObjectSize);
    rCube->SetTranslate(
        SceneCenterX + 15 + SmallObjectSize,
        SceneBottom + SmallObjectSize,
        SceneCenterZ + 5);
    rCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(60)));
    rCube->Material = std::make_unique<Lambertian>();

    SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
    wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
    wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
    wallLeft->Material = std::make_unique<Lambertian>(F(0.75), F(0.2), F(0.2));

    SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
    wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
    wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
    wallRight->SetExtends(SceneExtendZ, SceneExtendY);
    wallRight->Material = std::make_unique<Lambertian>(F(0.2), F(0.2), F(0.75));

    SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
    wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
    wallTop->SetExtends(SceneExtendZ, SceneExtendX);
    wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    wallTop->Material = std::make_unique<Lambertian>(F(0.75), F(0.75), F(0.75));

    SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
    wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
    wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
    wallBottom->Material = std::make_unique<Lambertian>(F(0.2), F(0.75), F(0.2));

    SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
    wallFar->SetExtends(SceneExtendX, SceneExtendY);
    wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
    wallFar->Material = std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6));

    SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
    LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
    LightDisk->SetExtends(25, 25);
    LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    F Intensity = 1;
    LightDisk->Material = std::make_unique<PureLight_ForTest>(Intensity, Intensity, Intensity);
}
