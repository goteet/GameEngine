#include "Integrator.h"

math::vector3<F> PathIntegrator::EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const math::vector3<F> BackgroundColor = math::vector3<F>::zero();
        return BackgroundColor;
    }

    const unsigned int MaxBounces = 10;
    float rrContinueProbability = 1.0f;
    auto CheckRussiaRoulette = [&](float prob) { return TerminateSampler.value() > prob; };

    math::vector3<F> Lo = math::vector3<F>::zero();
    math::vector3<F> beta = math::vector3<F>::one();
    math::ray3d<F> ray = cameraRay;

    // First hit, on p_1
    SurfaceIntersection hitRecord = recordP1;
    for (int bounce = 0; hitRecord && !math::near_zero(beta) && bounce < MaxBounces; ++bounce)
    {
        const SceneObject& surface = *hitRecord.Object;
        const Material& material = surface.Material;
        const math::nvector3<F>& N = hitRecord.SurfaceNormal;
        const math::nvector3<F> W_o = -ray.direction();
        const bool isLightSource = surface.LightSource != nullptr;
        F biasedDistance = math::max2<F>(hitRecord.Distance, F(0));
        math::point3d<F> P_i = ray.calc_offset(biasedDistance);
        F u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };


        if (isLightSource)
        {
            if (bounce == 0)
            {
                math::vector3<F> Le = surface.LightSource->Le();
                Lo += beta * Le;
            }
        }


        if (material.IsValid())
        {
            const BSDF& bsdf = *material.GetRandomBSDFComponent(u[0]);

            //Sampling Light Source
            SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
            if (!isLightSource && lightSource != nullptr && lightSource != hitRecord.Object)
            {
                math::point3d<F> P_i_1 = lightSource->SampleRandomPoint(u);
                math::ray3d<F> lightRay(P_i, P_i_1);
                const math::nvector3<F>& W_i = lightRay.direction();

                SurfaceIntersection lightSI = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<F>);
                bool bIsVisible = lightSI.Object == lightSource;
                if (bIsVisible)
                {
                    math::nvector3<F> N_light = lightSI.SurfaceNormal;
                    F cosThetaPrime = math::dot(N_light, -W_i);
                    bIsVisible = cosThetaPrime > math::SMALL_NUM<F> || (cosThetaPrime < -math::SMALL_NUM<F> && lightSource->IsDualface());

                    if (bIsVisible)
                    {
                        F pdf_light = lightSource->SamplePdf(lightSI, lightRay);
                        F pdf_bsdf = bsdf.pdf(N, W_o, W_i);
                        F weight = PowerHeuristic(pdf_light, pdf_bsdf);
                        math::vector3<F> f = bsdf.f(N, W_o, W_i, true);

                        const math::vector3<F>& Le = lightSource->LightSource->Le();
                        Lo += weight * beta * f * Le / pdf_light;
                    }
                }
            }

            //Sampling BSDF
            {
                LightRay scatterLight;
                if (!bsdf.Scattering(u, P_i, N, ray, hitRecord.IsOnSurface, scatterLight))
                {
                    break;
                }

                ray = scatterLight.scattering;
                const math::nvector3<F>& W_i = ray.direction();
                F pdf_light = scene.SampleLightPdf(scatterLight.scattering);
                F pdf_bsdf = bsdf.pdf(N, W_o, W_i);
                F weight = PowerHeuristic(pdf_bsdf, pdf_light);
                beta *= weight * scatterLight.f * math::saturate(scatterLight.cosine) / pdf_bsdf;
            }
        }

        if (bounce > 3)
        {
            rrContinueProbability *= 0.95f;
            if (CheckRussiaRoulette(rrContinueProbability))
            {
                break;
            }
            beta /= rrContinueProbability;
        }

        // Find next path p_i+1
        hitRecord = scene.DetectIntersecting(ray, nullptr, math::SMALL_NUM<F>);
    }

    return Lo;
}
