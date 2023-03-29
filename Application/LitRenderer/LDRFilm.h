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
    void FlushTo(const AccumulatedSpectrum& Spectrum, uint32_t Row, uint32_t Column, unsigned char* CanvasDataPtr, int linePitch);

private:
    AccumulatedSpectrum* mBackbuffer = nullptr;
};
