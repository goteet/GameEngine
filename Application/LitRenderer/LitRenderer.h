#pragma once
#include <memory>
#include <vector>
#include "PreInclude.h"
#include "LDRFilm.h"
#include "Material.h"
#include "Scene.h"


class SimpleBackCamera
{
    struct DegreeClampHelper
    {
        DegreeClampHelper(Degree degree)
            : value(math::clamp(degree.value, Float(1), Float(179))) { }
        const Float value;
    };
public:
    SimpleBackCamera(Degree verticalFov);
    Point Position;
    const Radian HalfVerticalFov;
    const Float HalfVerticalFovTangent;
};

class LitRenderer
{
public:
    LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch);
    ~LitRenderer();

    void Initialize();
    bool GenerateImageProgressive();
    bool NeedUpdate();

private:
    void InitialSceneTransforms();
    void GenerateCameraRays();
    void QueryP1Records();
    void ResolveSamples();

    static const int MaxCameraRaySampleCount = 8;
    static const int MaxLightRaySampleCount = -1;
    static const int MaxSampleCount = MaxCameraRaySampleCount * MaxLightRaySampleCount;

    struct Sample
    {
        Ray Ray;
        SurfaceIntersection RecordP1;
        int PixelRow = 0, PixelCol = 0;
    };

    const int mCanvasLinePitch;
    unsigned char* mSystemCanvasDataPtr;
    LDRFilm mFilm;
    SimpleBackCamera mCamera;
    std::unique_ptr<Scene> mScene;
    Sample* mCameraRaySamples[MaxCameraRaySampleCount];
    int  mCurrentCameraRayIndex = 0;

    Task ResolveSampleTask;
};
