#pragma once

#include <Foundation/Math/Vector.h>
#include "Material.h"

class LDRFilm
{
public:
    LDRFilm(int width, int height);
    ~LDRFilm();

    Spectrum* GetBackbufferPtr() { return mBackbuffer; }
    const int CanvasWidth;
    const int CanvasHeight;
    void Clear();
    void FlushTo(unsigned char* outCanvasDataPtr, int linePitch);
    void IncreaseSampleCount() { mSampleCount++; }
    int GetSampleCount() const { return mSampleCount; }

private:
    Spectrum* mBackbuffer = nullptr;
    int mSampleCount = 0;
};
