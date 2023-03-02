#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"
#include "Integrator.h"

namespace
{
    const bool DEBUG = false;
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

            const F roughness = F(0.01);
            const F IoR = F(1.5);
            if (DEBUG)
            {
                mainSphere->Material = MakeGGXMaterialForDebug(roughness, IoR);
            }
            else
            {
                F Ks = F(0.5);
                F Kd = F(1) - Ks;
                mainSphere->Material = MakePlasticMaterial(Kd, F(0.5), F(0.5), F(0.5), Ks, roughness, IoR);
            }

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
            wallLeft->Material = MakeMatteMaterial(F(0.75), F(0.2), F(0.2));

            SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
            wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
            wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
            wallRight->SetExtends(SceneExtendZ, SceneExtendY);
            wallRight->Material = MakeMatteMaterial(F(0.2), F(0.2), F(0.75));

            SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
            wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
            wallTop->SetExtends(SceneExtendZ, SceneExtendX);
            wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            wallTop->Material = MakeMatteMaterial(F(0.75), F(0.75), F(0.75));

            SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
            wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
            wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
            wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
            wallBottom->Material = MakeMatteMaterial(F(0.2), F(0.75), F(0.2));

            SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
            wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
            wallFar->SetExtends(SceneExtendX, SceneExtendY);
            wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
            wallFar->Material = MakeMatteMaterial(F(0.6), F(0.6), F(0.6));

            SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
            LightDisk->SetDualFace(true);
            LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
            LightDisk->SetExtends(25, 25);
            LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            F Intensity = 4.5;

            LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
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
            LambertianShere->Material = MakeMatteMaterial(1, 1, 1);


            SceneSphere* orenNayerSphere = new SceneSphere(); OutSceneObjects.push_back(orenNayerSphere);
            orenNayerSphere->SetRadius(20);
            orenNayerSphere->SetTranslate(
                SceneCenterX - 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            orenNayerSphere->Material = MakeMatteMaterial(1, 1, 1, math::radian<F>(0.25));

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
            wallLeft->Material = MakeMatteMaterial(F(0.6), F(0.6), F(0.6));

            SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
            wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
            wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
            wallRight->SetExtends(SceneExtendZ, SceneExtendY);
            wallRight->Material = MakeMatteMaterial(F(0.6), F(0.6), F(0.6));

            SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
            wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
            wallTop->SetExtends(SceneExtendZ, SceneExtendX);
            wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            wallTop->Material = MakeMatteMaterial(F(0.75), F(0.75), F(0.75));

            SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
            wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
            wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
            wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
            wallBottom->Material = MakeMatteMaterial(F(0.2), F(0.2), F(0.75));

            SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
            wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
            wallFar->SetExtends(SceneExtendX, SceneExtendY);
            wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
            wallFar->Material = MakeMatteMaterial(F(0.75), F(0.2), F(0.2));

            SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
            LightDisk->SetDualFace(true);
            LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
            LightDisk->SetExtends(25, 25);
            LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
            F Intensity = 4.5;
            LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
        }
    };

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

    auto CanvasPositionToRay = [&cameraZ](F x, F y) -> math::vector3<F> {
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
                samples.Ray.set_direction(CanvasPositionToRay(pixelCenterX + x * HalfPixelSize, pixelCenterY + y * HalfPixelSize));
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

    PathIntegrator pathIntegrator;
    DebugIntegrator debugIntegrator;
    Integrator& integrator = DEBUG ? (Integrator&)debugIntegrator : (Integrator&)pathIntegrator;

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
