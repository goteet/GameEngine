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
        virtual void CreateScene(Float aspect, std::vector<SceneObject*>& OutSceneObjects) override
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

            SceneSphere* mainSphere = new SceneSphere(); OutSceneObjects.push_back(mainSphere);
            mainSphere->SetRadius(20);
            mainSphere->SetTranslate(
                SceneCenterX,
                SceneBottom + 40,
                SceneCenterZ + 10);
            
            const Float roughness = Float(0.01);
            const Float IoR = Float(10);
            if (DEBUG)
            {
                mainSphere->Material = MakeGGXMaterialForDebug(roughness, IoR);
            }
            else
            {
                Float Ks = Float(0.5);
                Float Kd = Float(1) - Ks;
                mainSphere->Material = MakePlasticMaterial(Kd, Spectrum(Float(0.5), Float(0.5), Float(0.5)), Ks, roughness, IoR);
                //mainSphere->Material = MakeGGXMaterialForDebug(roughness, IoR);
            }

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);

            Spectrum Red(Float(0.75), Float(0.2), Float(0.2));
            Spectrum Green(Float(0.2), Float(0.75), Float(0.2));
            Spectrum Blue(Float(0.2), Float(0.2), Float(0.75));
            Spectrum Gray(Float(0.75), Float(0.75), Float(0.75));
            Spectrum DarkGray(Float(0.6), Float(0.6), Float(0.6));

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
            LightDisk->SetDualFace(true);
            LightDisk->SetTranslate(SceneCenterX, SceneTop - Float(0.01), SceneCenterZ + Float(10));
            LightDisk->SetExtends(25, 25);
            LightDisk->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
            Float Intensity = 4.5;

            LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
        }
    };

    class OrenNayerComparisionScene : public Scene
    {
        virtual void CreateScene(Float aspect, std::vector<SceneObject*>& OutSceneObjects) override
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

            SceneSphere* LambertianShere = new SceneSphere(); OutSceneObjects.push_back(LambertianShere);
            LambertianShere->SetRadius(20);
            LambertianShere->SetTranslate(
                SceneCenterX + 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            LambertianShere->Material = MakeMatteMaterial();


            SceneSphere* orenNayerSphere = new SceneSphere(); OutSceneObjects.push_back(orenNayerSphere);
            orenNayerSphere->SetRadius(20);
            orenNayerSphere->SetTranslate(
                SceneCenterX - 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            orenNayerSphere->Material = MakeMatteMaterial(Spectrum::one(), 0.25_radd);

            Spectrum Red(Float(0.75), Float(0.2), Float(0.2));
            Spectrum Green(Float(0.2), Float(0.75), Float(0.2));
            Spectrum Blue(Float(0.2), Float(0.2), Float(0.75));
            Spectrum Gray(Float(0.75), Float(0.75), Float(0.75));
            Spectrum DarkGray(Float(0.6), Float(0.6), Float(0.6));

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
            wallLeft->Material = MakeMatteMaterial(DarkGray);

            SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
            wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
            wallRight->SetRotation(math::make_rotation_y_axis<Float>(180_degd));
            wallRight->SetExtends(SceneExtendZ, SceneExtendY);
            wallRight->Material = MakeMatteMaterial(DarkGray);

            SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
            wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
            wallTop->SetExtends(SceneExtendZ, SceneExtendX);
            wallTop->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
            wallTop->Material = MakeMatteMaterial(Gray);

            SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
            wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
            wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
            wallBottom->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
            wallBottom->Material = MakeMatteMaterial(Blue);

            SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
            wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
            wallFar->SetExtends(SceneExtendX, SceneExtendY);
            wallFar->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
            wallFar->Material = MakeMatteMaterial(Red);

            SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
            LightDisk->SetDualFace(true);
            LightDisk->SetTranslate(SceneCenterX, SceneTop - Float(0.01), SceneCenterZ + Float(10));
            LightDisk->SetExtends(25, 25);
            LightDisk->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
            Float Intensity = 4.5;
            LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
        }
    };

}

LitRenderer::LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch)
    : mCanvasLinePitch(canvasLinePitch)
    , mSystemCanvasDataPtr(canvasDataPtr)
    , mFilm(canvasWidth, canvasHeight)
    , mClearColor(0, 0, 0)
    , mCamera(50_degd)
    , mScene(std::make_unique<SimpleScene>())
{
    mScene->Create(Float(canvasWidth) / Float(canvasHeight));
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
    const Float PixelSize = Float(0.1);
    const Float HalfPixelSize = PixelSize * Float(0.5);
    Float halfWidth = mFilm.CanvasWidth * Float(0.5) * PixelSize;
    Float halfHeight = mFilm.CanvasHeight * Float(0.5) * PixelSize;
    //     <---> (half height)
    //  .  o----. (o=origin)
    //  |  |  /   Asumed canvas is at origin(0,0,0),
    //  |  | /    and camera is placed at neg-z-axis,
    //  .  |/
    // (z) .      tan(half_fov) = halfHeight / cameraZ.
    Float cameraZ = halfHeight / mCamera.HalfVerticalFovTangent;
    mCamera.Position.z = -cameraZ;

    auto CanvasPositionToRay = [&cameraZ](Float x, Float y) -> math::vector3<Float> {
        //vector3<float>(x,y,0) - camera.position;
        //  x = x - 0;
        //  y = y - 0;
        //  z = 0 - camera.position.z
        return math::vector3<Float>(x, y, cameraZ);
    };

    const Float QuaterPixelSize = Float(0.5) * HalfPixelSize;
    Spectrum* canvasDataPtr = mFilm.GetBackbufferPtr();
    random<Float> RandomGeneratorPickingPixel;
    for (int rowIndex = 0; rowIndex < mFilm.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mFilm.CanvasWidth;
        for (int colIndex = 0; colIndex < mFilm.CanvasWidth; colIndex++)
        {
            const Float pixelCenterX = colIndex * PixelSize + HalfPixelSize - halfWidth;
            const Float pixelCenterY = rowIndex * PixelSize + HalfPixelSize - halfHeight;

            for (int index = 0; index < MaxCameraRaySampleCount; ++index)
            {
                Sample& samples = mCameraRaySamples[index][colIndex + rowOffset];
                samples.PixelRow = rowIndex;
                samples.PixelCol = colIndex;

                Float x = Float(2) * RandomGeneratorPickingPixel.value() - Float(1);
                Float y = Float(2) * RandomGeneratorPickingPixel.value() - Float(1);
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
                samples.RecordP1 = mScene->DetectIntersecting(samples.Ray, nullptr, math::SMALL_NUM<Float>);
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
    Spectrum* accumulatedBufferPtr = mFilm.GetBackbufferPtr();

    PathIntegrator pathIntegrator;
    DebugIntegrator debugIntegrator;
    Integrator& integrator = DEBUG ? (Integrator&)debugIntegrator : (Integrator&)pathIntegrator;

    for (int rowIndex = 0; rowIndex < mFilm.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mFilm.CanvasWidth;
        for (int colIndex = 0; colIndex < mFilm.CanvasWidth; colIndex++)
        {
            const Sample& sample = Samples[colIndex + rowOffset];
            Spectrum& canvasPixel = accumulatedBufferPtr[colIndex + rowOffset];
            canvasPixel += integrator.EvaluateLi(*mScene, sample.Ray, sample.RecordP1);
        }
    }
    mFilm.IncreaseSampleCount();
}

SimpleBackCamera::SimpleBackCamera(Degree verticalFov)
    : HalfVerticalFov(DegreeClampHelper(verticalFov).value* Float(0.5))
    , HalfVerticalFovTangent(math::tan(Degree(DegreeClampHelper(verticalFov).value* Float(0.5))))
    , Position(0, 0, 0)
{ }
