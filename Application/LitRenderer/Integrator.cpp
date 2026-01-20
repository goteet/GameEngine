#include <assert.h>
#include "Integrator.h"

Spectrum PathIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum BackgroundColor = Spectrum::zero();
        return BackgroundColor;
    }

    // Seen light source directly;
    if (recordP1.Object->LightSource != nullptr)
    {
        return recordP1.Object->LightSource->Le();
    }

    const unsigned int MaxBounces = 10;
    float rrContinueProbability = 1.0f;
    auto CheckRussiaRoulette = [&](float prob) { return TerminateSampler.value() > prob; };

    Spectrum Lo = Spectrum::zero();
    Spectrum beta = Spectrum::one();
    Ray viewRay = cameraRay;

    // First hit, on P1
    SurfaceIntersection hitRecord = recordP1;

    struct MISRecord
    {
        bool IsValid(LightSource* light) const
        {
            //Note:
            // Mitsuba will sample random lightsource
            // while PBR-v3 will sample the same light.
            // I have no idea which is better.
            // 
            //return Weight_bsdf > Float(0) && light == Light;
            return Weight_BSDF > Float(0);
        }

        Float Weight_BSDF = Float(0);
        LightSource* Light = nullptr;
        bool IsMirrorReflection = false;
    } lastMISRecord;

    for (int bounce = 0; hitRecord && bounce < MaxBounces && !math::near_zero(beta); ++bounce)
    {
        const SceneObject& surface = *hitRecord.Object;
        if (surface.LightSource != nullptr)
        {
            //Note:
            // 
            // Mitsuba will sample random lightsource
            // while PBR-v3 will sample the same light.
            // I have no idea which is better.
            // 
            if (lastMISRecord.IsValid(surface.LightSource.get()))
            {
                Spectrum Le = surface.LightSource->Le();
                Lo += beta * Le * lastMISRecord.Weight_BSDF;
            }
            break;
        }

        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        const Direction& T = hitRecord.SurfaceTangent;
        const Direction& N = hitRecord.SurfaceNormal;
        const UVW uvw(N, T);

        //Multiple Importance Sampling
        {
            const std::unique_ptr<Material>& material = surface.Material;
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
            const Point Pi = viewRay.calc_offset(biasedDistance);
            const Direction Wo = uvw.world_2_local(-viewRay.direction());

            lastMISRecord.IsMirrorReflection = (bsdf.BSDFMask& BSDFMask::MirrorMask) != 0;
            lastMISRecord.Light = nullptr;

            //Sampling Direct Illumination
            if (!lastMISRecord.IsMirrorReflection)
            {
                SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
                if (lightSource != nullptr && lightSource != hitRecord.Object)
                {
                    const Point Pi_1 = lightSource->SampleRandomPoint(u);
                    const Ray lightRay(Pi, Pi_1);

                    SurfaceIntersection recordPi_1 = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<Float>);
                    if (recordPi_1.Object == lightSource)
                    {
                        const Direction& N_light = recordPi_1.SurfaceNormal;
                        const Direction& Wi = uvw.world_2_local(lightRay.direction());
                        const Direction Wi_light = -lightRay.direction();

                        Float NdotV_light = math::dot(N_light, Wi_light);
                        const Float NdotL = CosTheta(Wi);
                        if (NdotV_light > Float(0) && NdotL > Float(0))
                        {
                            const Float pdf_light = scene.SampleLightPdf(lightRay);
                            assert(pdf_light > Float(0));
                            const Float pdf_bsdf = material->SamplePdf(Wo, Wi);
                            const Float weight_mis = PowerHeuristic(pdf_light, pdf_bsdf);
                            const Spectrum f = material->SampleF(Wo, Wi);
                            const Spectrum& Le = lightSource->LightSource->Le();
                            //                      f * cos(Wi)
                            // beta * mis_weight * -------------
                            //                       pdf_light
                            Lo += (weight_mis * NdotL / pdf_light) * (beta * f * Le);
                            lastMISRecord.Light = lightSource->LightSource.get();
                        }
                    }
                }
            }

            //Sampling BSDF
            {
                const Direction Wi = bsdf.SampleWi(u, Wo);
                const Float NdotL = CosTheta(Wi);
                if (NdotL <= Float(0))
                {
                    break;
                }

                viewRay.set_origin(Pi);
                viewRay.set_direction(uvw.local_2_world(Wi));

                const Float pdf_light = scene.SampleLightPdf(viewRay);
                const Float pdf_bsdf = material->SamplePdf(Wo, Wi);
                lastMISRecord.Weight_BSDF = (lastMISRecord.IsMirrorReflection) ? Float(1) : PowerHeuristic(pdf_bsdf, pdf_light);
                const Spectrum f = material->SampleF(Wo, Wi);
                beta *= (NdotL / pdf_bsdf) * f;
            }
        }

        // Termination Check
        if (bounce > 3)
        {
            rrContinueProbability *= 0.95f;
            if (CheckRussiaRoulette(rrContinueProbability))
            {
                break;
            }
            beta /= rrContinueProbability;
        }

        // Find next path ends with Pi+1
        hitRecord = scene.DetectIntersecting(viewRay, nullptr, math::SMALL_NUM<Float>);
    }

    return Lo;
}

Spectrum DebugIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum kBackgroundColor = Spectrum::zero();
        return kBackgroundColor;
    }

    const SurfaceIntersection& hitRecord = recordP1;
    const Ray& viewRay = cameraRay;
    const SceneObject& surface = *hitRecord.Object;

    if (surface.LightSource != nullptr)
    {
        Spectrum Le = surface.LightSource->Le();
        return Le;
    }
    else //Sampling BSDF
    {
        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        const Direction& T = hitRecord.SurfaceTangent;
        const Direction& N = hitRecord.SurfaceNormal;
        const UVW uvw(N, T);

        const std::unique_ptr<Material>& material = surface.Material;
        if (material)
        {
            const Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
            const Point P_i = viewRay.calc_offset(biasedDistance);
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const Direction Wo = uvw.world_2_local(-viewRay.direction());

            const Direction Wi = bsdf.SampleWi(u, Wo);
            const Float NdotL = math::dot(N, Wi);

            return Spectrum(-NdotL);
            return Wi * 0.5 + 0.5;

            if (NdotL > 0)
            {
                //auto f = material->SampleF(N, Wo, Wi);
                //auto pdf = material->SamplePdf(N, Wo, Wi);
                //auto r = f * NdotL / pdf;
                //return r;
                return cameraRay.direction() * 0.5 + 0.5;

                const Direction Half = (Wi + Wo);
                return Half * 0.5 + 0.5;
                return Spectrum(1, 0, 0) * (math::dot(N, Half));
            }
        }
    }

    return Spectrum::zero();
}

Spectrum MISDebugIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    const SurfaceIntersection& hitRecord = recordP1;
    if (hitRecord)
    {
        const Ray& viewRay = cameraRay;

        const SceneObject& surface = *hitRecord.Object;
        if (surface.LightSource != nullptr)
        {
            Spectrum Le = surface.LightSource->Le();
            return Le;
        }
        else
        {
            Spectrum Weights = Spectrum::zero();

            const Float biasedDistance = math::saturate(hitRecord.Distance);
            const Point P_i = viewRay.calc_offset(biasedDistance);
            Float u[3] =
            {
                mUniformSamplers[0].value(),
                mUniformSamplers[1].value(),
                mUniformSamplers[2].value()
            };

            const Direction& T = hitRecord.SurfaceTangent;
            const Direction& N = hitRecord.SurfaceNormal;
            const UVW uvw(N, T);
            const Direction Wo = uvw.world_2_local(-viewRay.direction());

            const std::unique_ptr<Material>& material = surface.Material;
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const bool bIsMirrorReflection = (bsdf.BSDFMask & BSDFMask::MirrorMask) != 0;

            if (!bIsMirrorReflection)
            {
                SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
                if (lightSource != nullptr)
                {
                    Point P_i_1 = lightSource->SampleRandomPoint(u);
                    Ray lightRay(P_i, P_i_1);
                    const Direction Wi_light = -lightRay.direction();
                    const Direction Wi = uvw.world_2_local(lightRay.direction());

                    SurfaceIntersection recordPi_1 = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<Float>);
                    bool bIsVisible = recordPi_1.Object == lightSource;
                    if (bIsVisible)
                    {
                        Direction N_light = recordPi_1.SurfaceNormal;
                        Float cosThetaPrime = math::dot(N_light, Wi_light);
                        bIsVisible = cosThetaPrime > math::SMALL_NUM<Float> || (cosThetaPrime < -math::SMALL_NUM<Float> && lightSource->IsDualface());

                        if (bIsVisible)
                        {
                            Float pdf_light = scene.SampleLightPdf(lightRay);
                            assert(pdf_light > Float(0));
                            Float pdf_bsdf = material->SamplePdf(Wo, Wi);
                            Float weight = PowerHeuristic(pdf_light, pdf_bsdf);
                            Weights.y = math::saturate(weight);
                        }
                    }
                }
            }

            {
                const Direction Wi = bsdf.SampleWi(u, Wo);
                const Float NdotL = math::dot(N, Wi);
                if (NdotL > Float(0))
                {
                    Float weight = Float(1);
                    if (bIsMirrorReflection)
                    {
                        Float pdf_light = scene.SampleLightPdf(Ray{ P_i, uvw.local_2_world(Wi) });
                        if (pdf_light > Float(0))
                        {
                            Float pdf_bsdf = material->SamplePdf(Wo, Wi);
                            weight = PowerHeuristic(pdf_bsdf, pdf_light);
                        }
                    }
                    Weights.x = math::saturate(weight);
                }
            }
            return Weights;
        }
    }

    return Spectrum::zero();
}

Direction Refraction(const Direction& V, const Direction& N, Float eta)
{
    Float cos1 = dot(V, N);
    Float cos2sqr = Float(1.0) - eta * eta * (Float(1.0) - cos1 * cos1);
    if (cos2sqr < Float(0.0))
        return math::reflection(V, N);
    return (eta * cos1 - sqrt(cos2sqr)) * N - eta * V;
}

Spectrum WhittedIntegrator::Evaluate(Scene& scene, const Ray& sampleRay, const SurfaceIntersection& si, int depth)
{
    const int MAX_DEPTH = 5;
    if (depth == MAX_DEPTH)
    {
        return Spectrum(Float(1), Float(0), Float(0));
    }
    else if (!si)
    {
        return Spectrum(Float(0), Float(0), Float(0));
    }

    const Direction& N = si.SurfaceNormal;
    const Direction V = -sampleRay.direction();
    Point shadingPoint = sampleRay.calc_offset(si.Distance);
    Point Ndelta = 0.001 *  N;

    if (si.Object->LightSource != nullptr)
    {
        return si.Object->LightSource->Le();
    }

    switch (si.Object->Material->Type) {
    case Material::Transparency:
    {
        Float n1 = Float(1.0); Float n2 = Float(1.5);// r.geometry.material.IoR;
        Float eta = si.IsOnSurface ? n1 / n2 : n2 / n1;
        Ray tRay{ shadingPoint - Ndelta, Refraction(V, N, eta) };
        Ray rRay{ shadingPoint + Ndelta, math::reflection(V, N) };

        SurfaceIntersection tsi = scene.DetectIntersecting(tRay, nullptr, math::SMALL_NUM<Float>);
        SurfaceIntersection rsi = scene.DetectIntersecting(rRay, nullptr, math::SMALL_NUM<Float>);

        Spectrum T = Evaluate(scene, tRay, tsi, depth + 1);
        Spectrum R = Evaluate(scene, rRay, rsi, depth + 1);

        
        Float Kr = Float(0.25);
        Float Kt = Float(1.0) - Kr;
        return Kt * T + Kr * R;
    }
    default:
    case Material::Opaque:
    {
        Spectrum Diffuse = Spectrum(0);
        int numLights = scene.GetLightCount();
        for (int lightIdx = 0; lightIdx < numLights; lightIdx++)
        {
            SceneObject* lightObject = scene.GetLightSourceByIndex(lightIdx);
            Direction L = (lightObject->WorldTransform.Translate - shadingPoint);

            Ray shadowRay{ shadingPoint + Ndelta, L };
            SurfaceIntersection ssi = scene.DetectIntersecting(shadowRay, nullptr, math::SMALL_NUM<Float>);
            Float shadow = ssi && ssi.Object->LightSource == nullptr ? Float(1.0) : Float(0.0);
            Spectrum Kd = si.Object->Material->GetAlbedo();
            Float NdotL = math::dot(N, L);
            Diffuse += Kd * (NdotL * (1.0f - shadow));
        }
        return Diffuse;
    }
    }
}

Spectrum WhittedIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    return Evaluate(scene, cameraRay, recordP1, 0);
}

Direction RandomDirection()
{
    Radian theta = Degree(random<Float>::value() * 360);
    Radian phi   = Degree(random<Float>::range(90));
    Float cosTheta = cos(theta);
    Float sinTheta = sin(theta);
    Float cosPhi   = cos(phi);
    Float sinPhi   = sin(phi);
    return { cosTheta * sinPhi, sinTheta * sinPhi, cosPhi };
}


Spectrum DistributionIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    const int N = 100;
    Spectrum L = Spectrum(0);
    Float InvN = Float(1) / N;
    for (int n = 0; n < N; n++) {
        L += Evaluate(scene, cameraRay, recordP1, 0) * InvN;
    }
    return L;
}

Spectrum DistributionIntegrator::Evaluate(Scene& scene, const Ray& sampleRay, const SurfaceIntersection& si, int depth)
{
    const int MAX_DEPTH = 5;
    if (depth == MAX_DEPTH)
    {
        return Spectrum(Float(1), Float(0), Float(0));
    }
    else if (!si)
    {
        return Spectrum(Float(0), Float(0), Float(0));
    }

    const Direction& N = si.SurfaceNormal;
    const Direction V = -sampleRay.direction();
    Point shadingPoint = sampleRay.calc_offset(si.Distance);
    Point Ndelta = 0.001 * N;

    if (si.Object->LightSource != nullptr)
    {
        return si.Object->LightSource->Le();
    }

    Float Kr = Float(1.0);
    Float Ks = Float(0.15);

    Spectrum L = Spectrum(0);
    if(si.Object->Material->Type == Material::Transparency)
    {
        Kr = Float(0.05);
        Ks = Float(0.85);

        Float Kt = Float(1.0) - Kr;
        Float n1 = Float(1.0); Float n2 = Float(1.5);
        Float eta = si.IsOnSurface ? n1 / n2 : n2 / n1;
        Direction tdir = Refraction(V, N, eta) + 0.1 * RandomDirection();
        Ray tRay{ shadingPoint - Ndelta, tdir };
    
        SurfaceIntersection tsi = scene.DetectIntersecting(tRay, nullptr, math::SMALL_NUM<Float>);
        Spectrum T = Evaluate(scene, tRay, tsi, depth + 1);
        L += Kt * T;
    }

    //Reflection
    {
        Direction rdir = math::reflection(V, N) + 0.1 * RandomDirection();
        Ray rRay{ shadingPoint + Ndelta, rdir };
        SurfaceIntersection rsi = scene.DetectIntersecting(rRay, nullptr, math::SMALL_NUM<Float>);
        Float NdotR = dot(rdir, N);
        Spectrum R = Evaluate(scene, rRay, rsi, depth + 1) * NdotR;
        
        Spectrum D = Spectrum(0);
        int numLights = scene.GetLightCount();
        for (int lightIdx = 0; lightIdx < numLights; lightIdx++)
        {
            SceneObject* lightObject = scene.GetLightSourceByIndex(lightIdx);
            Float u[3] = { random<Float>::value(),random<Float>::value(),random<Float>::value() };
            Point lightSourceSamplingPoint = lightObject->SampleRandomPoint(u);
            Direction ldir = (lightSourceSamplingPoint - shadingPoint);
            Ray shadowRay{ shadingPoint + Ndelta, ldir };
            SurfaceIntersection ssi = scene.DetectIntersecting(shadowRay, nullptr, math::SMALL_NUM<Float>);
            Float shadow = ssi && ssi.Object->LightSource == nullptr ? Float(1.0) : Float(0.0);
            Spectrum Albedo = si.Object->Material->GetAlbedo();
            Float NdotL = math::dot(N, ldir);
            D += Albedo * (NdotL * (1.0f - shadow));
        }


        Float Kd = Float(1) - Ks;
        Ks *= Kr;
        Kd *= Kr;
        L += Ks * R + Kd * D;
    }
    return L;
}



Spectrum SimplePathIntegrator::Evaluate(Scene& scene, const Ray& sampleRay, const SurfaceIntersection& si, int depth)
{
    const int MAX_DEPTH = 5;
    if (depth == MAX_DEPTH)
    {
        return Spectrum(Float(0), Float(0), Float(0));
    }
    else if (!si)
    {
        return Spectrum(Float(0), Float(0), Float(0));
    }

    const Direction& N = si.SurfaceNormal;
    const Direction V = -sampleRay.direction();
    Point shadingPoint = sampleRay.calc_offset(si.Distance);
    Point Ndelta = 0.001 * N;

    if (si.Object->LightSource != nullptr)
    {
        return si.Object->LightSource->Le();
    }

    Float Kr = Float(1.0);
    Float Ks = Float(0.15);
    Float Kt = Float(1.0) - Kr;

    const bool bIsTransparent = si.Object->Material->Type == Material::Transparency;
    int ELight   = 0;
    int EReflect = 1;
    int ETransmit= 2;
    int rayType  = ELight;

    if (bIsTransparent)
    {
        Kr = Float(0.05);
        Ks = Float(0.85);
        Kt = Float(1.0) - Kr;
        rayType = static_cast<int>(random<Float>::value() * 3);
    }
    else
    {
        static_cast<int>(random<Float>::value() * 2);
    }

    if (rayType >= ETransmit)
    {
        Float n1 = Float(1.0); Float n2 = Float(1.5);
        Float eta = si.IsOnSurface ? n1 / n2 : n2 / n1;
        Direction tdir = Refraction(V, N, eta) + 0.1 * RandomDirection();
        Ray tRay{ shadingPoint - Ndelta, tdir };

        SurfaceIntersection tsi = scene.DetectIntersecting(tRay, nullptr, math::SMALL_NUM<Float>);
        Spectrum T = Evaluate(scene, tRay, tsi, depth + 1);
        return Kt * T;
    }
    else
    {
        Direction rdir = math::reflection(V, N) + 0.1 * RandomDirection();
        Ray rRay{ shadingPoint + Ndelta, rdir };

        if (rayType == ELight)
        {
            Float Kd = Float(1) - Ks;

            Spectrum D = Spectrum(0);
            int numLights = scene.GetLightCount();
            int randomLightIndex = static_cast<int>(random<Float>::value() * numLights);

            //for (int lightIdx = randomLightIndex; lightIdx < numLights; lightIdx++)
            int lightIdx = randomLightIndex;
            {
                SceneObject* lightObject = scene.GetLightSourceByIndex(lightIdx);
                Float u[3] = { random<Float>::value(),random<Float>::value(),random<Float>::value() };
                Point lightSourceSamplingPoint = lightObject->SampleRandomPoint(u);
                Direction ldir = (lightSourceSamplingPoint - shadingPoint);
                Ray shadowRay{ shadingPoint + Ndelta, ldir };
                SurfaceIntersection ssi = scene.DetectIntersecting(shadowRay, nullptr, math::SMALL_NUM<Float>);
                Float shadow = ssi && ssi.Object->LightSource == nullptr ? Float(1.0) : Float(0.0);
                Spectrum Albedo = si.Object->Material->GetAlbedo();
                Float NdotL = math::dot(N, ldir);
                D += Albedo * (NdotL * (1.0f - shadow));
            }
            return Kd * D;
        }
        else
        {
            SurfaceIntersection rsi = scene.DetectIntersecting(rRay, nullptr, math::SMALL_NUM<Float>);
            Float NdotR = dot(rdir, N);
            Spectrum R = Evaluate(scene, rRay, rsi, depth + 1) * NdotR;
            return Kr * R;
        }
    }
}

Direction RandomUniformDirection()
{
    Float u1 = random<Float>::value();
    Float u2 = random<Float>::value();

    Float costheta = u1;
    Float sintheta = sqrt(1 - u1 * u1);
    Radian phi = Radian(math::PI<Float> * u2 * Float(2));
    Float cosphi = math::cos(phi);
    Float sinphi = math::sin(phi);

    return { sintheta * cosphi, sintheta * sinphi, costheta };
}

Direction RandomCosineWeightedDirection()
{
    Float u1 = random<Float>::value();
    Float u2 = random<Float>::value();

    Float costheta = sqrt(u1);
    Float sintheta = sqrt(1 - u1);
    Radian phi = Radian(math::PI<Float> * u2 * Float(2));
    Float cosphi = math::cos(phi);
    Float sinphi = math::sin(phi);

    return { sintheta * cosphi, sintheta * sinphi, costheta };
}

Spectrum SimpleDiffuseIntegrator::Evaluate(Scene& scene, const Ray& sampleRay, const SurfaceIntersection& si, int depth)
{
    const int MAX_DEPTH = 5;
    if (depth == MAX_DEPTH)
    {
        return Spectrum(Float(0), Float(0), Float(0));
    }
    else if (!si)
    {
        return Spectrum(Float(0), Float(0), Float(0));
    }

    const bool bUseConsineWeighted = true;

    const Direction& N = si.SurfaceNormal;
    const Direction& T = si.SurfaceTangent;
    const Direction V = -sampleRay.direction();
    Point shadingPoint = sampleRay.calc_offset(si.Distance);
    Point Ndelta = 0.001 * N;

    if (si.Object->LightSource != nullptr)
    {
        return si.Object->LightSource->Le();
    }

    Spectrum D = Spectrum(0);
    int numLights = scene.GetLightCount();
    int randomLightIndex = static_cast<int>(random<Float>::value() * numLights);

    const UVW uvw(N, T);
    Direction dr = bUseConsineWeighted
        ? RandomCosineWeightedDirection()
        : RandomUniformDirection();
    dr = uvw.local_2_world(dr);


    Ray rRay{ shadingPoint + Ndelta, dr };

    SurfaceIntersection rsi = scene.DetectIntersecting(rRay, nullptr, math::SMALL_NUM<Float>);
    Float NdotR = dot(dr, N);
    Spectrum R = Evaluate(scene, rRay, rsi, depth + 1);
    Spectrum Albedo = si.Object->Material->GetAlbedo();

    Float weight = bUseConsineWeighted
        ? (Float(1) * math::PI<Float>)
        : (Float(2) * math::PI<Float> * NdotR);
    return weight * Albedo * R;
}
