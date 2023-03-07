#include "Integrator.h"

Spectrum PathIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum BackgroundColor = Spectrum::zero();
        return BackgroundColor;
    }

    const unsigned int MaxBounces = 10;
    float rrContinueProbability = 1.0f;
    auto CheckRussiaRoulette = [&](float prob) { return TerminateSampler.value() > prob; };

    Spectrum Lo = Spectrum::zero();
    Spectrum beta = Spectrum::one();
    Ray ray = cameraRay;

    // First hit, on p_1
    SurfaceIntersection hitRecord = recordP1;
    bool bIsReflectionTrace = false;
    for (int bounce = 0; hitRecord && !math::near_zero(beta) && bounce < MaxBounces; ++bounce)
    {
        const SceneObject& surface = *hitRecord.Object;
        const std::unique_ptr<Material>& material = surface.Material;
        const Direction& N = hitRecord.SurfaceNormal;
        const Direction W_o = -ray.direction();
        const bool bIsLightSource = surface.LightSource != nullptr;
        Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
        Point P_i = ray.calc_offset(biasedDistance);


        if (bIsLightSource)
        {
            if (bounce == 0 || bIsReflectionTrace)
            {
                Spectrum Le = surface.LightSource->Le();
                Lo += beta * Le;
            }
            break;
        }

        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
        bIsReflectionTrace = (bsdf.BSDFMask & Material::BSDFMask::Reflection) != 0;

        //Sampling Light Source
        if (!bIsReflectionTrace)
        {
            SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
            if (lightSource != nullptr && lightSource != hitRecord.Object)
            {
                Point P_i_1 = lightSource->SampleRandomPoint(u);
                Ray lightRay(P_i, P_i_1);
                const Direction& W_i = lightRay.direction();

                SurfaceIntersection lightSI = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<Float>);
                bool bIsVisible = lightSI.Object == lightSource;
                if (bIsVisible)
                {
                    Direction N_light = lightSI.SurfaceNormal;
                    Float cosThetaPrime = math::dot(N_light, -W_i);
                    bIsVisible = cosThetaPrime > math::SMALL_NUM<Float> || (cosThetaPrime < -math::SMALL_NUM<Float> && lightSource->IsDualface());

                    if (bIsVisible)
                    {
                        Float pdf_light = lightSource->SamplePdf(lightSI, lightRay);
                        //F pdf_bsdf = bsdf.pdf(N, W_o, W_i);
                        Float pdf_bsdf = material->SamplePdf(N, W_o, W_i);
                        Float weight = PowerHeuristic(pdf_light, pdf_bsdf);
                        //Spectrum f = bsdf.f(N, W_o, W_i, true);

                        Spectrum f = material->SampleF(N, W_o, W_i, true);

                        const Spectrum& Le = lightSource->LightSource->Le();
                        Lo += weight * beta * f * Le / pdf_light;
                    }
                }
            }
        }

        //Sampling BSDF
        {
            BSDFSample scatterLight;
            if (!bsdf.Scattering(u, P_i, N, ray, hitRecord.IsOnSurface, scatterLight))
            {
                break;
            }

            ray = scatterLight.scattering;
            const Direction& W_i = ray.direction();
            Float pdf_light = scene.SampleLightPdf(scatterLight.scattering);
            Float pdf_bsdf = bsdf.pdf(N, W_o, W_i);
            Float weight = PowerHeuristic(pdf_bsdf, pdf_light);
            beta *= weight * bsdf.Weight * scatterLight.f * math::saturate(scatterLight.cosine) / pdf_bsdf;
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
        hitRecord = scene.DetectIntersecting(ray, nullptr, math::SMALL_NUM<Float>);
    }

    return Lo;
}

Spectrum DebugIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum BackgroundColor = Spectrum::zero();
        return BackgroundColor;
    }

    const unsigned int MaxBounces = 10;
    float rrContinueProbability = 1.0f;
    auto CheckRussiaRoulette = [&](float prob) { return TerminateSampler.value() > prob; };


    const SurfaceIntersection& hitRecord = recordP1;
    const Ray& ray = cameraRay;

    const SceneObject& surface = *hitRecord.Object;
    const std::unique_ptr<Material>& material = surface.Material;
    const Direction& N = hitRecord.SurfaceNormal;
    const Direction W_o = -ray.direction();

    const bool bIsLightSource = surface.LightSource != nullptr;
    if (bIsLightSource)
    {
        Spectrum Le = surface.LightSource->Le();
        return Le;
    }
    else //Sampling BSDF
    {
        Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
        Point P_i = ray.calc_offset(biasedDistance);
        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        if (material)
        {
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            BSDFSample scatterLight;
            if (bsdf.Scattering(u, P_i, N, ray, hitRecord.IsOnSurface, scatterLight))
            {
                const Ray& scatteringRay = scatterLight.scattering;
                const Direction& W_i = scatteringRay.direction();
                return W_i;

                const Direction Half = (W_i + W_o);
                return Half * 0.5 + 0.5;
                return Spectrum(1, 0, 0) * (math::dot(N, Half));
            }
        }
    }

    return Spectrum::zero();
}
