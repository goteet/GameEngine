#include "Material.h"
#include "LitRenderer.h"

template<int E> Float Ws(Float p1, Float p2)
{
    return Ws<1>(math::power<E>(p1), math::power<E>(p2));
}

template<> Float Ws<1>(Float p1, Float p2)
{
    return p1 / (p1 + p2);
}

Float PowerHeuristic(Float pdfA, Float pdfB)
{
    return Ws<2>(pdfA, pdfB);
}

Float BalanceHeuristic(Float pdfA, Float pdfB)
{
    return Ws<1>(pdfA, pdfB);
}

Float FresnelSchlick(Float CosTheta, Float eta1, Float eta2)
{
    Float F0 = (eta1 - eta2) / (eta1 + eta2);
    Float R0 = F0 * F0;
    Float Base = Float(1) - CosTheta;
    return R0 + (Float(1) - R0) * math::power<5>(Base);
}


Float DistributionGTR2(Float roughness, Float NdotH)
{
    //F alpha = math::square(roughness);
    Float alpha2 = math::power<4>(roughness);
    //      alpha^2 * X(n.m)           sqrt           alpha * X                    0.5 * alpha * X
    //--------------------------------- = -------------------------------- = ----------------------------
    // 4 * cos^4 * (alpha^2 + tan^2)^2      2 * (alpha^2 * cos^2 + sin^2)      cos^2 * (alpha^2 - 1) + 1     //<----should replace 4 with PI.
    //
    // Note: Naty-Hoffman-2015 show that there is PI in the demoninator.
    // it should be:
    //                  a^2                                               a2
    //  Dtr = -------------------------- = InversePi * ----------------------------------------
    //         PI * ((n.h)^2(a^2-1)+1)^2                 (NdotH^2 * (a2 - 1) + 1)^2
    Float cos = math::saturate(NdotH); //<--hidden X(x) -> x>0?1:0; here
    Float denominator = math::square(cos) * (alpha2 - 1) + 1;
    return alpha2 * math::square(denominator) * math::InvPI<Float>;
}

//still error with GTR2, so I copy this from filament's description
Float DistributionGGX_FromFilament(Float roughness, Float NdotH)
{
    Float a = NdotH * roughness;
    Float cos = math::saturate(NdotH);
    Float numerator = roughness;
    Float denominator = Float(1) - math::square(cos) + math::square(a);
    return math::square(numerator / denominator) * math::InvPI<Float>;
}

Float DistributionGTR1(Float roughness, Float HdotN)
{
    Float alpha = math::square(roughness);
    Float X = HdotN > 0 ? Float(1) : Float(0);
    //      alpha^2 * X(n.m)                       alpha^2 * X                  (0.5 * alpha * X)^2
    //--------------------------------- = ------------------------------- = ---------------------------
    // 4 * cos^2 * (alpha^2 + tan^2)       4 * (alpha^2 * cos^2 + sin^2)     cos^2 * (alpha^2 - 1) + 1
    //
    Float cos = math::saturate(HdotN);
    Float numerator = Float(0.5) * alpha * X;
    Float denominator = math::square(cos) * (math::square(alpha) - Float(1)) + 1;
    return math::square(numerator) / denominator;
}

Float DistributionGGX(Float roughness, Float NdotH)
{
    return DistributionGTR2(roughness, NdotH);
}

Float DistributionBerry(Float roughness, Float NdotH)
{
    return DistributionGTR1(roughness, NdotH);
}

Float ShadowingGGX(Float roughness, Float VdotH, Float NdotH)
{
    Float alpha = math::square(roughness);
    Float alpha2 = math::square(alpha);
    Float cos = math::saturate(VdotH);
    Float cos2 = math::square(cos);
    Float X = (NdotH / VdotH) > 0 ? Float(1) : Float(0);
    //          2 * X                                       2 * cos * X                                     2 * cos * X
    //--------------------------------- = ------------------------------------------- = ---------------------------------------------
    // 1 + sqrt(1 + AlphaG^2 * tan^2)      cos + sqrt(cos^2 + alpha^2 * (1 - cos^2))     cos + sqrt(cos^2 * (1 - alpha^2) + alpha^2)
    //
    Float numerator = Float(2) * cos * X;
    Float denominator = cos + sqrt(cos2 * (Float(1) - alpha2) + alpha2);
    return numerator / denominator;
}

Float GeometryGGX(Float roughness, Float NdotH, Float VdotH, Float LdotH)
{
    Float G1 = ShadowingGGX(roughness, VdotH, NdotH);
    Float G2 = ShadowingGGX(roughness, LdotH, NdotH);
    return math::min2(G1, G2);
}



struct UVW
{
    Direction u, v, w;

    UVW(const Direction& normal) { from_normal(normal); }

    void from_normal(const Direction& normal)
    {
        w = normal;
        v = fabs(w.x) > Float(0.95) ? Direction::unit_y() : Direction::unit_x();
        v = math::cross(w, v);
        u = math::cross(w, v);
    }

    Direction local_to_world(Float x, Float y, Float z)
    {
        return x * u + y * v + z * w;
    }
    Direction local_to_world(const Direction& dir)
    {
        return local_to_world(dir.x, dir.y, dir.z);
    }

    Direction world_to_local(const Direction& dir)
    {
        Spectrum x = math::projection(dir, u);
        Spectrum y = math::projection(dir, v);
        Spectrum z = math::projection(dir, w);
        return x + y + z;
    }
};

Spectrum GenerateUnitSphereVector()
{
    static random<Float> rand;
    while (true)
    {
        Float x = rand(Float(1));
        Float y = rand(Float(1));
        Float z = rand(Float(1));
        Spectrum v = Spectrum(x, y, z);
        if (math::magnitude_sqr(v) < Float(1))
        {
            return v;
        }
    }
}

Direction GenerateHemisphereDirection(Float epsilon[2], const Direction& normal)
{
    Float rand_theta = epsilon[0];
    Float rand_phi = epsilon[1];
    Float cosTheta = Float(1) - Float(2) * rand_theta;
    Float sinTheta = sqrt(Float(1) - cosTheta * cosTheta);
    Radian phi(math::TWO_PI<Float> *rand_phi);
    Float cosPhi = math::cos(phi);
    Float sinPhi = math::sin(phi);

    Float x = sinTheta * cosPhi;
    Float y = sinTheta * sinPhi;
    Float z = cosTheta;

    UVW uvw(normal);
    return uvw.local_to_world(x, y, z);
}

Direction GenerateUniformHemisphereDirection(Float epsilon[2], const Direction& normal)
{
    Float rand_theta = epsilon[0];
    Float rand_phi = epsilon[1];
    // pdf = 1/2PI
    // => cdf = 1/2PI*phi*(1-cos_theta)
    // => f_phi = 1/2PI*phi       --> phi(x) = 2*PI*x
    // => f_theta = 1-cos_theta   --> cos_theta(x) = 1-x = x'
    Float cosTheta = rand_theta; //replace 1-e to e'
    Float sinTheta = sqrt(Float(1) - cosTheta * cosTheta);
    Radian phi(math::TWO_PI<Float> *rand_phi);
    Float cosPhi = math::cos(phi);
    Float sinPhi = math::sin(phi);

    Float x = sinTheta * cosPhi;
    Float y = sinTheta * sinPhi;
    Float z = cosTheta;

    return UVW(normal).local_to_world(x, y, z);
}


Spectrum GenerateCosineWeightedHemisphereDirection(Float epsilon[2], const Direction& normal)
{
    Float rand_theta = epsilon[0];
    Float rand_phi = epsilon[1];
    // pdf = cos(theta) / Pi.
    Float cosTheta_sqr = rand_theta; //replace 1-e to e'
    Float cosTheta = sqrt(cosTheta_sqr);
    Float sinTheta = sqrt(Float(1) - cosTheta_sqr);
    Radian phi(Float(2) * math::PI<Float> *rand_phi);
    Float cosPhi = math::cos(phi);
    Float sinPhi = math::sin(phi);

    Float x = sinTheta * cosPhi;
    Float y = sinTheta * sinPhi;
    Float z = cosTheta;

    UVW uvw(normal);
    return uvw.local_to_world(x, y, z);
}



bool Lambertian::Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    Direction Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.cosine = math::dot(Wi, N);
    outLightRay.f = Lambertian::f(N, Ray.direction(), Wi, IsOnSurface);
    return outLightRay.cosine >= Float(0);
}

Spectrum Lambertian::f(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi,
    bool IsOnSurface) const
{
    return Albedo * math::InvPI<Float>;
}

Float Lambertian::pdf(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi) const
{
    Float NdotL = math::dot(N, Wi);
    return math::saturate(NdotL) * math::InvPI<Float>;
}



OrenNayer::OrenNayer(Float r, Float g, Float b, Radian sigma, Float weight)
    : BSDF("Oren-Nayer", Material::BSDFMask::Diffuse, weight)
    , Albedo(r, g, b)
    , Sigma(sigma)
    , SigmaSquare(math::square(sigma.value))
{
    A = Float(1) - Float(0.5) * SigmaSquare.value / (SigmaSquare.value + Float(0.33));
    B = Float(0.45) * SigmaSquare.value / (SigmaSquare.value + Float(0.09));
}

bool OrenNayer::Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    Direction Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.cosine = math::dot(Wi, N);
    outLightRay.f = f(N, Ray.direction(), Wi, IsOnSurface);
    return outLightRay.cosine >= Float(0);
}

Spectrum OrenNayer::f(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi,
    bool IsOnSurface) const
{
    const Float cosWi = math::saturate(math::dot(N, Wi)); const Float sinWi = sqrt(Float(1) - math::square(cosWi));
    const Float cosWo = math::saturate(math::dot(N, Wo)); const Float sinWo = sqrt(Float(1) - math::square(cosWo));

    Float sinAlpha, tanBeta;
    bool IsWiGreater = cosWi < cosWo;
    if (IsWiGreater)
    {
        sinAlpha = sinWi;
        tanBeta = sinWo / cosWo;
    }
    else
    {
        sinAlpha = sinWo;
        tanBeta = sinWi / cosWi;
    }

    Float maxWi_Wo = math::max2(Float(0), cosWi * cosWo + sinWi * sinWo);
    Float factor = A + B * maxWi_Wo * sinAlpha * tanBeta;
    return Albedo * math::InvPI<Float> *factor * cosWi;
}

Float OrenNayer::pdf(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi) const
{
    Float NdotL = math::dot(N, Wi);
    return math::saturate(NdotL) * math::InvPI<Float>;
}

bool IsReflectionDirection(const Direction& N, const Direction& Wo, const Direction& Wi)
{
    const Direction Wr = math::reflection(Wo, N);
    const Direction a = math::cross(N, Wo);
    const Direction b = math::cross(Wr, N);
    return math::almost_same(Wr, Wi, Float(0.05)) && math::almost_same(a, b, Float(0.01));
}

const Float AirRefractiveIndex = Float(1.0003);
bool Glossy::Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    const Float eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const Float eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;

    bool result = false;
    bool chooseReflectRay = epsilon[0] < SpecularSamplingProbability;
    if (chooseReflectRay)
    {
        const Direction Wo = -Ray.direction();
        const Direction Wi = math::reflection(Wo, N);
        const Float NdotL = math::dot(Wi, N);
        const Float Fi = FresnelSchlick(NdotL, eta1, eta2);
        outLightRay.specular = true;
        outLightRay.scattering.set_origin(P);
        outLightRay.scattering.set_direction(Wi);
        outLightRay.f = Spectrum::one() * Fi;
        outLightRay.cosine = NdotL;
        result = NdotL > Float(0);
    }
    else
    {
        const Direction Wo = -Ray.direction();
        Direction Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
        outLightRay.scattering.set_origin(P);
        outLightRay.scattering.set_direction(Wi);
        outLightRay.cosine = math::dot(Wi, N);
        outLightRay.f = f(N, Ray.direction(), Wi, IsOnSurface);

        result = outLightRay.cosine >= Float(0);

        if (result)
        {
            const Direction& Wi = outLightRay.scattering.direction();
            const Float NdotL = outLightRay.cosine;
            const Float NdotV = math::dot(Wo, N);
            const Float Fi = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
            const Float Fr = FresnelSchlick(math::saturate(NdotV), eta2, eta1);
            outLightRay.f *= math::saturate(Float(1) - Fi) * math::saturate((Float(1) - Fr));
        }
    }
    return result;
}


bool Glossy::IsSpecular(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi) const
{
    return IsReflectionDirection(N, Wo, Wi);
}

Spectrum Glossy::f(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi,
    bool IsOnSurface) const
{
    const Float eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const Float eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const Float NdotL = math::dot(Wi, N);
    const Float NdotV = math::dot(Wo, N);

    Float Fi = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    Float Fr = FresnelSchlick(math::saturate(NdotV), eta2, eta1);
    Float DiracApproxmation = IsSpecular(N, Wo, Wi) ? Float(1) : Float(0);
    Spectrum s = Spectrum::one() * (Fi * DiracApproxmation);
    Spectrum d = f(N, Wo, Wi, IsOnSurface) * (Float(1) - Fi) * (Float(1) - Fr);
    return DiffuseSamplingProbability * d + SpecularSamplingProbability * s;
}

Float Glossy::pdf(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi) const
{
    const Direction Wr = math::reflection(Wo, N);
    const Direction a = math::cross(N, Wo);
    const Direction b = math::cross(Wr, N);
    Float DiracApproxmation = math::almost_same(Wr, Wi, Float(0.1)) && math::almost_same(a, b, Float(0.1)) ? Float(1) : Float(0);
    Float pdfSpecular = DiracApproxmation;

    Float NdotL = math::dot(N, Wi);
    Float pdfDiffuse = math::saturate(NdotL) * math::InvPI<Float>;
    return DiffuseSamplingProbability * pdfDiffuse + SpecularSamplingProbability * pdfSpecular;
}


Direction SampleGGXVNDF(
    const Direction& Wo,  // Input Wo: view direction
    Float alpha_x, Float alpha_y,                   // Input alpha_x, alpha_y: roughness parameters
    Float e1, Float e2                              // Input U1, U2: uniform random numbers
)
{
    // stretch view
    Direction V = Wo * Spectrum(alpha_x, alpha_y, Float(1));

    // orthonormal basis
    Direction T1 = V.z < Float(0.99999)
        ? math::cross(V, Direction::unit_z())
        : Direction::unit_x();
    Direction T2 = math::cross(V, T1);

    // sample point with polar coordinates (r, phi)
    Float a = Float(1) / (Float(1) + V.z);
    Float r = sqrt(e1);
    Radian phi = (e2 < a)
        ? Radian(e2 / a * math::PI<Float>)
        : Radian(math::PI<Float> +(e2 - a) / (Float(1) - a) * math::PI<Float>);

    Float t1 = r * cos(phi);
    Float t2 = r * sin(phi) * ((e2 < a) ? Float(1) : V.z);

    // compute normal
    Spectrum N = T1 * t1 + T2 * t2 + V * sqrt(math::max2(Float(0), Float(1) - math::square(t1) - math::square(t2)));

    // unstretch
    Direction Wm = Spectrum(alpha_x * N.x, alpha_y * N.y, math::max2(Float(0), N.z));
    return Wm;
}

bool GGX::Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    const Direction Wo = -Ray.direction();
    const Float eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const Float eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;

    Float alpha = math::square(Roughness);
    UVW uvw(N);
    Direction Wm = SampleGGXVNDF(uvw.world_to_local(-Wo), alpha, alpha, epsilon[1], epsilon[2]);
    Direction H = uvw.local_to_world(Wm);
    Direction Wi = math::reflection(Wo, H);
    const Float NdotL = math::dot(Wi, N);
    const Float NdotH = math::dot(N, H);
    const Float HdotV = math::dot(H, Wo);
    const Float Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    //const F D = DistributionGGX(Roughness, NdotH);
    const Float G = GeometryGGX(Roughness, NdotH, math::dot(H, Wi), math::dot(H, Wo));
    //const F BRDF = F(0.25) * Frehnel * D * G / NdotV;
    const Float BRDF = Frehnel * G / HdotV;
    outLightRay.f = Spectrum::one() * BRDF;
    outLightRay.cosine = NdotL;
    outLightRay.specular = true;
    return NdotL > 0;
}

Spectrum GGX::f(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi,
    bool IsOnSurface) const
{
    const Direction H = Wo + Wi;
    const Float eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const Float eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const Float NdotL = math::dot(N, Wi);
    const Float NdotH = math::dot(N, H);
    const Float NdotV = math::dot(N, Wo);
    const Float HdotV = math::dot(H, Wo);
    const Float Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    const Float D = DistributionGGX(Roughness, NdotH);
    const Float G = GeometryGGX(Roughness, NdotH, math::dot(H, Wi), math::dot(H, Wo));
    const Float brdf = Frehnel * G / HdotV;
    return NdotL > Float(0) ? Spectrum::one() * brdf : Spectrum::zero();
}

Float GGX::pdf(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi) const
{
#if true
    //copy from UE4.
    const Direction H = Wi + Wo;
    const Float NdotH = math::dot(N, H);
    const Float VdotH = math::dot(Wo, H);
    const Float D = DistributionGGX(Roughness, NdotH);
    return NdotH / VdotH;
    //return D * NdotH * 0.25 / VdotH;
#else
	const Direction H = Wi + Wo;
    F pdfSpecular = math::power<5>(math::dot(H, N));
    return pdfSpecular;
#endif
}

void Material::AddBSDFComponent(std::unique_ptr<BSDF> component)
{
    mBSDFMask |= component->BSDFMask;
    mBSDFComponents.emplace_back(std::move(component));
}

const std::unique_ptr<BSDF>& Material::GetBSDFComponentByMask(uint32_t mask) const
{
    static std::unique_ptr<BSDF> dummy = nullptr;
    for (const auto& comp : mBSDFComponents)
    {
        if (comp->BSDFMask & mask)
        {
            return comp;
        }
    }
    return dummy;
}

const std::unique_ptr<BSDF>& Material::GetRandomBSDFComponent(Float u) const
{
    uint32_t length = (uint32_t)mBSDFComponents.size();
    uint32_t index = math::min2<uint32_t>(math::floor2<uint32_t>(u * length), length - 1);
    return GetBSDFComponentByIndex(index);
}
