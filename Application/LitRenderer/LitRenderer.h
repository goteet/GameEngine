#pragma once
#include <memory>
#include <vector>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Matrix.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>
#include "LDRFilm.h"
#include "Material.h"
#include "Scene.h"


class SimpleBackCamera
{
    struct DegreeClampHelper
    {
        DegreeClampHelper(math::degree<F> degree)
            : value(math::clamp(degree.value, F(1), F(179))) { }
        const F value;
    };
public:
    SimpleBackCamera(math::degree<F> verticalFov);
    math::point3d<F> Position;
    const math::radian<F> HalfVerticalFov;
    const F HalfVerticalFovTangent;
};

class LitRenderer
{
public:
    LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch);
    ~LitRenderer();

    void Initialize();
    void GenerateImageProgressive();
    bool NeedUpdate();
    void ClearUpdate() { mNeedUpdateSystemWindowRect = false; }

private:
    void InitialSceneTransforms();
    void GenerateCameraRays();
    void QueryP1Records();
    void ResolveSamples();

    static const int MaxCameraRaySampleCount = 16;
    static const int MaxLightRaySampleCount = 256;
    static const int MaxSampleCount = MaxCameraRaySampleCount* MaxLightRaySampleCount;

    struct Sample
    {
        math::ray3d<F> Ray;
        SurfaceIntersection RecordP1;
        int PixelRow, PixelCol;
    };

    const int mCanvasLinePitch;
    unsigned char* mSystemCanvasDataPtr;


    math::vector3<F> mClearColor;
    LDRFilm mFilm;
    SimpleBackCamera mCamera;
    std::unique_ptr<Scene> mScene;
    Sample* mCameraRaySamples[MaxCameraRaySampleCount];
    int  mCurrentCameraRayIndex = 0;
    bool mNeedFlushBackbuffer = true;
    bool mNeedUpdateSystemWindowRect = false;
};
