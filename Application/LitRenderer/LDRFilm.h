#pragma once

#include <Foundation/Math/Vector.h>
#include "Material.h"

struct AccumulatedSpectrum
{
    Spectrum Value = Spectrum::zero();
    uint32_t Count = 0;
};
class LDRFilm
{
public:
    LDRFilm(int width, int height);
    ~LDRFilm();

    AccumulatedSpectrum* GetBackbufferPtr() { return mBackbuffer; }
    const int CanvasWidth;
    const int CanvasHeight;
    void Clear();
    void FlushTo(unsigned char* outCanvasDataPtr, int linePitch, Task& Task);

private:
    AccumulatedSpectrum* mBackbuffer = nullptr;
};
