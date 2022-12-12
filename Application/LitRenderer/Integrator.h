#pragma once
#include "LitRenderer.h"

struct Integrator
{
    virtual ~Integrator() { };
    virtual math::vector3<F> EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const HitRecord& recordP1) = 0;
};

class PathIntegrator : public Integrator
{
    random<F> TerminateSampler;
    random<F> mEpsilonSamplers[3];
public:
    virtual math::vector3<F> EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const HitRecord& recordP1) override;
};
