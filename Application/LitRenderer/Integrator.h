#pragma once
#include "LitRenderer.h"

struct Integrator
{
    virtual ~Integrator() { };
    virtual math::vector3<F> EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const SurfaceIntersection& recordP1) = 0;
};

class PathIntegrator : public Integrator
{
    random<F> TerminateSampler;
    random<F> mUniformSamplers[3];
public:
    virtual math::vector3<F> EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const SurfaceIntersection& recordP1) override;
};


class DebugIntegrator : public Integrator
{
    random<F> TerminateSampler;
    random<F> mUniformSamplers[3];
public:
    virtual math::vector3<F> EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const SurfaceIntersection& recordP1) override;

};
