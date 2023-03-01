#include "Material.h"
#include "LitRenderer.h"

template<int E> F Ws(F p1, F p2)
{
    return Ws<1>(math::power<E>(p1), math::power<E>(p2));
}

template<> F Ws<1>(F p1, F p2)
{
    return p1 / (p1 + p2);
}

F PowerHeuristic(F pdfA, F pdfB)
{
    return Ws<2>(pdfA, pdfB);
}

F BalanceHeuristic(F pdfA, F pdfB)
{
    return Ws<1>(pdfA, pdfB);
}

F FresnelSchlick(F CosTheta, F eta1, F eta2)
{
    F F0 = (eta1 - eta2) / (eta1 + eta2);
    F R0 = F0 * F0;
    F Base = F(1) - CosTheta;
    return R0 + (F(1) - R0) * math::power<5>(Base);
}


F DistributionGTR2(F roughness, F NdotH)
{
    F alpha = math::square(roughness);
    F X = NdotH > 0 ? F(1) : F(0);
    //      alpha^2 * X(n.m)           sqrt           alpha * X                    0.5 * alpha * X
    //--------------------------------- = -------------------------------- = ----------------------------
    // 4 * cos^4 * (alpha^2 + tan^2)^2      2 * (alpha^2 * cos^2 + sin^2)      cos^2 * (alpha^2 - 1) + 1
    //
    F cos = math::saturate(NdotH);
    F numerator = F(0.5) * alpha * X;
    F denominator = math::square(cos) * (math::square(alpha) - F(1)) + 1;
    return math::square(numerator / denominator);
}

F DistributionGTR1(F roughness, F HdotN)
{
    F alpha = math::square(roughness);
    F X = HdotN > 0 ? F(1) : F(0);
    //      alpha^2 * X(n.m)                       alpha^2 * X                  (0.5 * alpha * X)^2
    //--------------------------------- = ------------------------------- = ---------------------------
    // 4 * cos^2 * (alpha^2 + tan^2)       4 * (alpha^2 * cos^2 + sin^2)     cos^2 * (alpha^2 - 1) + 1
    //
    F cos = math::saturate(HdotN);
    F numerator = F(0.5) * alpha * X;
    F denominator = math::square(cos) * (math::square(alpha) - F(1)) + 1;
    return math::square(numerator) / denominator;
}

F DistributionGGX(F roughness, F NdotH)
{
    return DistributionGTR2(roughness, NdotH);
}

F DistributionBerry(F roughness, F NdotH)
{
    return DistributionGTR1(roughness, NdotH);
}

F ShadowingGGX(F roughness, F VdotH, F NdotH)
{
    F alpha = math::square(roughness);
    F alpha2 = math::square(alpha);
    F cos = math::saturate(VdotH);
    F cos2 = math::square(cos);
    F X = (NdotH / VdotH) > 0 ? F(1) : F(0);
    //          2 * X                                       2 * cos * X                                     2 * cos * X
    //--------------------------------- = ------------------------------------------- = ---------------------------------------------
    // 1 + sqrt(1 + AlphaG^2 * tan^2)      cos + sqrt(cos^2 + alpha^2 * (1 - cos^2))     cos + sqrt(cos^2 * (1 - alpha^2) + alpha^2)
    //
    F numerator = F(2) * cos * X;
    F denominator = cos + sqrt(cos2 * (F(1) - alpha2) + alpha2);
    return numerator / denominator;
}

F GeometryGGX(F roughness, F NdotH, F VdotH, F LdotH)
{
    F G1 = ShadowingGGX(roughness, VdotH, NdotH);
    F G2 = ShadowingGGX(roughness, LdotH, NdotH);
    return math::min2(G1, G2);
}



struct UVW
{
    math::nvector3<F> u, v, w;

    UVW(const math::nvector3<F>& normal) { from_normal(normal); }

    void from_normal(const math::nvector3<F>& normal)
    {
        w = normal;
        v = fabs(w.x) > F(0.95) ? math::nvector3<F>::unit_y() : math::nvector3<F>::unit_x();
        v = math::cross(w, v);
        u = math::cross(w, v);
    }

    math::nvector3<F> local_to_world(F x, F y, F z)
    {
        return x * u + y * v + z * w;
    }
    math::nvector3<F> local_to_world(const math::nvector3<F>& dir)
    {
        return local_to_world(dir.x, dir.y, dir.z);
    }

    math::nvector3<F> world_to_local(const math::nvector3<F>& dir)
    {
        math::vector3<F> x = math::projection(dir, u);
        math::vector3<F> y = math::projection(dir, v);
        math::vector3<F> z = math::projection(dir, w);
        return x + y + z;
    }
};

math::vector3<F> GenerateUnitSphereVector()
{
    static random<F> rand;
    while (true)
    {
        F x = rand(F(1));
        F y = rand(F(1));
        F z = rand(F(1));
        math::vector3<F> v = math::vector3<F>(x, y, z);
        if (math::magnitude_sqr(v) < F(1))
        {
            return v;
        }
    }
}

math::nvector3<F> GenerateHemisphereDirection(F epsilon[2], const math::nvector3<F>& normal)
{
    F rand_theta = epsilon[0];
    F rand_phi = epsilon[1];
    F cosTheta = F(1) - F(2) * rand_theta;
    F sinTheta = sqrt(F(1) - cosTheta * cosTheta);
    math::radian<F> phi(math::TWO_PI<F> *rand_phi);
    F cosPhi = math::cos(phi);
    F sinPhi = math::sin(phi);

    F x = sinTheta * cosPhi;
    F y = sinTheta * sinPhi;
    F z = cosTheta;

    UVW uvw(normal);
    return uvw.local_to_world(x, y, z);
}

math::nvector3<F> GenerateUniformHemisphereDirection(F epsilon[2], const math::nvector3<F>& normal)
{
    F rand_theta = epsilon[0];
    F rand_phi = epsilon[1];
    // pdf = 1/2PI
    // => cdf = 1/2PI*phi*(1-cos_theta)
    // => f_phi = 1/2PI*phi       --> phi(x) = 2*PI*x
    // => f_theta = 1-cos_theta   --> cos_theta(x) = 1-x = x'
    F cosTheta = rand_theta; //replace 1-e to e'
    F sinTheta = sqrt(F(1) - cosTheta * cosTheta);
    math::radian<F> phi(math::TWO_PI<F> *rand_phi);
    F cosPhi = math::cos(phi);
    F sinPhi = math::sin(phi);

    F x = sinTheta * cosPhi;
    F y = sinTheta * sinPhi;
    F z = cosTheta;

    return UVW(normal).local_to_world(x, y, z);
}


math::vector3<F> GenerateCosineWeightedHemisphereDirection(F epsilon[2], const math::nvector3<F>& normal)
{
    F rand_theta = epsilon[0];
    F rand_phi = epsilon[1];
    // pdf = cos(theta) / Pi.
    F cosTheta_sqr = rand_theta; //replace 1-e to e'
    F cosTheta = sqrt(cosTheta_sqr);
    F sinTheta = sqrt(F(1) - cosTheta_sqr);
    math::radian<F> phi(F(2) * math::PI<F> *rand_phi);
    F cosPhi = math::cos(phi);
    F sinPhi = math::sin(phi);

    F x = sinTheta * cosPhi;
    F y = sinTheta * sinPhi;
    F z = cosTheta;

    UVW uvw(normal);
    return uvw.local_to_world(x, y, z);
}



bool Lambertian::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    math::nvector3<F> Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.cosine = math::dot(Wi, N);
    outLightRay.f = Lambertian::f(N, Ray.direction(), Wi, IsOnSurface);
    return outLightRay.cosine >= F(0);
}

math::vector3<F> Lambertian::f(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi,
    bool IsOnSurface) const
{
    return Albedo * math::InvPI<F>;
}

F Lambertian::pdf(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi) const
{
    F NdotL = math::dot(N, Wi);
    return math::saturate(NdotL) * math::InvPI<F>;
}



OrenNayer::OrenNayer(F r, F g, F b, math::radian<F> sigma, F weight)
    : BSDF("Oren-Nayer", Material::BSDFMask::Diffuse, weight)
    , Albedo(r, g, b)
    , Sigma(sigma)
    , SigmaSquare(math::square(sigma.value))
{
    A = F(1) - F(0.5) * SigmaSquare.value / (SigmaSquare.value + F(0.33));
    B = F(0.45) * SigmaSquare.value / (SigmaSquare.value + F(0.09));
}

bool OrenNayer::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    math::nvector3<F> Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.cosine = math::dot(Wi, N);
    outLightRay.f = f(N, Ray.direction(), Wi, IsOnSurface);
    return outLightRay.cosine >= F(0);
}

math::vector3<F> OrenNayer::f(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi,
    bool IsOnSurface) const
{
    const F cosWi = math::saturate(math::dot(N, Wi)); const F sinWi = sqrt(F(1) - math::square(cosWi));
    const F cosWo = math::saturate(math::dot(N, Wo)); const F sinWo = sqrt(F(1) - math::square(cosWo));

    F sinAlpha, tanBeta;
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

    F maxWi_Wo = math::max2(F(0), cosWi * cosWo + sinWi * sinWo);
    F factor = A + B * maxWi_Wo * sinAlpha * tanBeta;
    return Albedo * math::InvPI<F> *factor * cosWi;
}

F OrenNayer::pdf(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi) const
{
    F NdotL = math::dot(N, Wi);
    return math::saturate(NdotL) * math::InvPI<F>;
}

bool IsReflectionDirection(const math::nvector3<F>& N, const math::nvector3<F>& Wo, const math::nvector3<F>& Wi)
{
    const math::nvector3<F> Wr = math::reflection(Wo, N);
    const math::nvector3<F> a = math::cross(N, Wo);
    const math::nvector3<F> b = math::cross(Wr, N);
    return math::almost_same(Wr, Wi, F(0.05)) && math::almost_same(a, b, F(0.01));
}

const F AirRefractiveIndex = F(1.0003);
bool Glossy::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    const F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;

    bool result = false;
    bool chooseReflectRay = epsilon[0] < SpecularSamplingProbability;
    if (chooseReflectRay)
    {
        const math::nvector3<F> Wo = -Ray.direction();
        const math::nvector3<F> Wi = math::reflection(Wo, N);
        const F NdotL = math::dot(Wi, N);
        const F Fi = FresnelSchlick(NdotL, eta1, eta2);
        outLightRay.specular = true;
        outLightRay.scattering.set_origin(P);
        outLightRay.scattering.set_direction(Wi);
        outLightRay.f = math::vector3<F>::one() * Fi;
        outLightRay.cosine = NdotL;
        result = NdotL > F(0);
    }
    else
    {
        const math::nvector3<F> Wo = -Ray.direction();
        math::nvector3<F> Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
        outLightRay.scattering.set_origin(P);
        outLightRay.scattering.set_direction(Wi);
        outLightRay.cosine = math::dot(Wi, N);
        outLightRay.f = f(N, Ray.direction(), Wi, IsOnSurface);

        result = outLightRay.cosine >= F(0);

        if (result)
        {
            const math::nvector3<F>& Wi = outLightRay.scattering.direction();
            const F NdotL = outLightRay.cosine;
            const F NdotV = math::dot(Wo, N);
            const F Fi = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
            const F Fr = FresnelSchlick(math::saturate(NdotV), eta2, eta1);
            outLightRay.f *= math::saturate(F(1) - Fi) * math::saturate((F(1) - Fr));
        }
    }
    return result;
}


bool Glossy::IsSpecular(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi) const
{
    return IsReflectionDirection(N, Wo, Wi);
}

math::vector3<F> Glossy::f(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi,
    bool IsOnSurface) const
{
    const F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const F NdotL = math::dot(Wi, N);
    const F NdotV = math::dot(Wo, N);

    F Fi = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    F Fr = FresnelSchlick(math::saturate(NdotV), eta2, eta1);
    F DiracApproxmation = IsSpecular(N, Wo, Wi) ? F(1) : F(0);
    math::vector3<F> s = math::vector3<F>::one() * (Fi * DiracApproxmation);
    math::vector3<F> d = f(N, Wo, Wi, IsOnSurface) * (F(1) - Fi) * (F(1) - Fr);
    return DiffuseSamplingProbability * d + SpecularSamplingProbability * s;
}

F Glossy::pdf(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi) const
{
    const math::nvector3<F> Wr = math::reflection(Wo, N);
    const math::nvector3<F> a = math::cross(N, Wo);
    const math::nvector3<F> b = math::cross(Wr, N);
    F DiracApproxmation = math::almost_same(Wr, Wi, F(0.1)) && math::almost_same(a, b, F(0.1)) ? F(1) : F(0);
    F pdfSpecular = DiracApproxmation;

    F NdotL = math::dot(N, Wi);
    F pdfDiffuse = math::saturate(NdotL) * math::InvPI<F>;
    return DiffuseSamplingProbability * pdfDiffuse + SpecularSamplingProbability * pdfSpecular;
}


math::nvector3<F> SampleGGXVNDF(
    const math::nvector3<F>& Wo,  // Input Wo: view direction
    F alpha_x, F alpha_y,                   // Input alpha_x, alpha_y: roughness parameters
    F e1, F e2                              // Input U1, U2: uniform random numbers
)
{
    // stretch view
    math::nvector3<F> V = Wo * math::vector3<F>(alpha_x, alpha_y, F(1));

    // orthonormal basis
    math::nvector3<F> T1 = V.z < F(0.99999)
        ? math::cross(V, math::nvector3<F>::unit_z())
        : math::nvector3<F>::unit_x();
    math::nvector3<F> T2 = math::cross(V, T1);

    // sample point with polar coordinates (r, phi)
    F a = F(1) / (F(1) + V.z);
    F r = sqrt(e1);
    math::radian<F> phi = (e2 < a)
        ? math::radian<F>(e2 / a * math::PI<F>)
        : math::radian<F>(math::PI<F> +(e2 - a) / (F(1) - a) * math::PI<F>);

    F t1 = r * cos(phi);
    F t2 = r * sin(phi) * ((e2 < a) ? F(1) : V.z);

    // compute normal
    math::vector3<F> N = T1 * t1 + T2 * t2 + V * sqrt(math::max2(F(0), F(1) - math::square(t1) - math::square(t2)));

    // unstretch
    math::nvector3<F> Wm = math::vector3<F>(alpha_x * N.x, alpha_y * N.y, math::max2(F(0), N.z));
    return Wm;
}

bool GGX::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    const math::nvector3<F> Wo = -Ray.direction();
    const F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;

    F alpha = math::square(Roughness);
    UVW uvw(N);
    math::nvector3<F> Wm = SampleGGXVNDF(uvw.world_to_local(-Wo), alpha, alpha, epsilon[1], epsilon[2]);
    math::nvector3<F> Wi = math::reflection(Wo, uvw.local_to_world(Wm));
    const F NdotL = math::dot(Wi, N);
    const F Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.f = math::vector3<F>::one() * Frehnel;
    outLightRay.cosine = NdotL;
    outLightRay.specular = true;
    return NdotL > 0;
}

math::vector3<F> GGX::f(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi,
    bool IsOnSurface) const
{
    const math::nvector3<F> H = Wo + Wi;
    const F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const F NdotL = math::dot(N, Wi);
    const F NdotH = math::dot(N, H);
    const F NdotV = math::dot(N, Wo);
    const F Fi = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    const F Fr = FresnelSchlick(math::saturate(NdotV), eta2, eta1);
    const F D = DistributionGGX(Roughness, NdotH);
    const F G = GeometryGGX(Roughness, NdotH, math::dot(H, Wi), math::dot(H, Wo));
    F brdf = F(0.25) * Fi * D * G / (NdotV);
    return NdotL > F(0) ? math::vector3<F>::one() * brdf : math::vector3<F>::zero();
}

F GGX::pdf(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi) const
{
    const math::nvector3<F> H = Wi + Wo;
    F pdfSpecular = math::power<5>(math::dot(H, N));
    return pdfSpecular;
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

const std::unique_ptr<BSDF>& Material::GetRandomBSDFComponent(F u) const
{
    uint32_t length = (uint32_t)mBSDFComponents.size();
    uint32_t index = math::min2<uint32_t>(math::floor2<uint32_t>(u * length), length - 1);
    return GetBSDFComponentByIndex(index);
}
