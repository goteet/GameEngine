#pragma once
#include <vector>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>

using F = double;
class RenderCanvas
{
public:
    RenderCanvas(unsigned char *canvasDataPtr, int width, int height, int linePitch);
    ~RenderCanvas();

    bool NeedUpdate();

    math::vector3<F>* GetBackbufferPtr() { return mBackbuffer; }
    bool NeedFlushBackbuffer = true;
    const int CanvasWidth;
    const int CanvasHeight;
    const int CanvasLinePitch;

private:
    void FlushLinearColorToGammaCorrectedCanvasData();

    bool mNeedUpdateWindowRect = false;
    unsigned char *mOutCanvasDataPtr;
    math::vector3<F>* mBackbuffer = nullptr;
};

class SimpleBackCamera
{
    struct DegreeClampHelper
    {
        DegreeClampHelper(math::degree<F> degree)
            : value(math::clamp(degree.value, 1.0f, 179.0f)) { }
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
    LitRenderer(unsigned char * canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch);
    ~LitRenderer();

    void GenerateImage();
    bool NeedUpdate();

private:
    void GenerateSamples();
    void ResolveSamples();

    struct Sample
    {
        math::ray3d<F> ray;
        int pixelRow, pixelCol;
    };

    const int mSampleArrayCount;
    RenderCanvas mCanvas;
    SimpleBackCamera mCamera;

    std::vector<Sample>* mImageSamples = nullptr;
};