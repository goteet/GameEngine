#pragma once
#include <Foundation/Math/Vector.h>

class RenderCanvas
{
public:
    RenderCanvas(unsigned char *canvasDataPtr, int width, int height, int linePitch);

    ~RenderCanvas();

    bool NeedUpdate();

    math::float3* GetBackbufferPtr() { return mBackbuffer; }

private:
    void FlushLinearColorToGammaCorrectedCanvasData();
    bool mNeedFlushBackbuffer = true;
    bool mNeedWindowUpdate = false;

    unsigned char *mOutCanvasDataPtr;
    const int mCanvasWidth;
    const int mCanvasHeight;
    const int mCanvasLinePitch;

    math::float3* mBackbuffer = nullptr;
};