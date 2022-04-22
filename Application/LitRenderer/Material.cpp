#include "Material.h"
#include "LitRenderer.h"


F PowerHeuristic(F pdfA, F pdfB)
{
    pdfA *= pdfA;
    pdfB *= pdfB;
    return pdfA / (pdfA + pdfB);
}

F BalanceHeuristic(F pdfA, F pdfB)
{
    return pdfA / (pdfA + pdfB);
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

F GeometryGGX(F roughness, F NdotH, F VdotH, F IdotH)
{
    F G1 = ShadowingGGX(roughness, VdotH, NdotH);
    F G2 = ShadowingGGX(roughness, IdotH, NdotH);
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
    outLightRay.f = f(N, Ray.direction(), Wi, IsOnSurface);
    return math::dot(Wi, N) > F(0);
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
    F pdfLambertian = math::saturate(NdotL) * math::InvPI<F>;
    return pdfLambertian;
}



OrenNayer::OrenNayer(F r, F g, F b, math::radian<F> sigma)
    : IMaterial(r, g, b)
    , Sigma(sigma)
    , SigmaSqr(math::square(sigma.value))
{
    A = F(1) - F(0.5) * SigmaSqr.value / (SigmaSqr.value + F(0.33));
    B = F(0.45) * SigmaSqr.value / (SigmaSqr.value + F(0.09));
}


math::vector3<F> OrenNayer::f(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi,
    bool IsOnSurface) const
{
    F cosWi = math::saturate(math::dot(N, Wi)); F sinWi = sqrt(F(1) - math::square(cosWi));
    F cosWo = math::saturate(math::dot(N, Wo)); F sinWo = sqrt(F(1) - math::square(cosWo));

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
    return Albedo * math::InvPI<F> *factor;
}

bool Metal::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    const math::nvector3<F> Wo = -Ray.direction();
    const math::vector3<F> FuzzyDirection = Fuzzy * GenerateUnitSphereVector();
    const math::nvector3<F> Wi = math::reflection(Wo, N) + FuzzyDirection;
    outLightRay.isSpecular = true;
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.f = math::vector3<F>::one();
    F NdotL = math::dot(N, Wi);
    return NdotL > F(0);
}

F Metal::pdf(const math::nvector3<F>& N, const math::nvector3<F>& Wo, const math::nvector3<F>& Wi) const
{
    return math::saturate(math::dot(Wi, N));
}

bool Dielectric::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    //TODO: totally wrong!
    const F AirRefractiveIndex = F(1.0003);

    const math::nvector3<F> Wo = -Ray.direction();

    F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const F RefractionRatio = eta1 / eta2;

    const F NdotO = math::dot(Wo, N);
    F CosTheta = math::saturate(NdotO);
    F SinThetaSqr = math::saturate(F(1) - CosTheta * CosTheta);
    F Det = F(1) - SinThetaSqr * RefractionRatio * RefractionRatio;
    bool RefractRay = Det >= F(0);

    bool ReflectRay = epsilon[0] < FresnelSchlick(CosTheta, eta1, eta2);
    math::nvector3<F> Wi;
    if (!RefractRay || ReflectRay)
    {
        Wi = math::reflection(Wo, N);
    }
    else
    {
        math::vector3<F> Rprep = RefractionRatio * (Wo + CosTheta * N);
        math::vector3<F> Rpall = N * sqrt(Det);
        Wi = Rprep - Rpall;
    }

    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.f = math::vector3<F>::one();
    return true;
}

bool PureLight_ForTest::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    return false;
}

math::vector3<F> PureLight_ForTest::Emitting() const
{
    return Emission;
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
        const F Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
        outLightRay.scattering.set_origin(P);
        outLightRay.scattering.set_direction(Wi);
        outLightRay.f = math::vector3<F>(Frehnel, Frehnel, Frehnel);
        outLightRay.isSpecular = true;
        result = math::dot(Wi, N) > F(0);
    }
    else
    {
        result = Lambertian::Scattering(epsilon, P, N, Ray, IsOnSurface, outLightRay);
        const math::nvector3<F>& Wi = outLightRay.scattering.direction();
        const F NdotL = math::dot(Wi, N);
        const F Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
        outLightRay.f *= F(1) - Frehnel;
    }
    return result;
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
    const F Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    math::vector3<F> s(Frehnel, Frehnel, Frehnel);
    math::vector3<F> d = Lambertian::f(N, Wo, Wi, IsOnSurface) * (F(1) - Frehnel);
    return DiffuseSamplingProbability * d + SpecularSamplingProbability * s;
}

F Glossy::pdf(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi) const
{
    math::nvector3<F> H = Wo + Wi;
    F DiracApproxmation = math::near_zero(N - H) ? F(1) : F(0);
    F pdfSpecular = DiracApproxmation * math::saturate(math::dot(N, Wi));
    F pdfDiffuse = Lambertian::pdf(N, Wo, Wi);
    return DiffuseSamplingProbability * pdfDiffuse + SpecularSamplingProbability * pdfSpecular;
}


math::nvector3<F> SampleGGXVNDF(
    const math::nvector3<F>& Wo,  // Input Wo: view direction
    F alpha_x, F alpha_y,                   // Input alpha_x, alpha_y: roughness parameters
    F e1, F e2                              // Input U1, U2: uniform random numbers
)
{
    // Section 3.2: transforming the view direction to the hemisphere configuration
    math::nvector3<F> H = Wo * math::vector3<F>(alpha_x, alpha_y, F(1));
    // Section 4.1: orthonormal basis (with special case if cross product is zero)
    F lensq = H.x * H.x + H.y * H.y;
    math::vector3<F> T1 = lensq > 0 ? math::vector3<F>(-H.y, H.x, 0) * math::inverse_sqrt(lensq) : math::vector3<F>(F(1), 0, 0);
    math::vector3<F> T2 = math::cross(H, T1);
    // Section 4.2: parameterization of the projected area
    F r = sqrt(e1);
    math::radian<F> phi(F(2.0) * math::PI<F> *e2);
    F t1 = r * cos(phi);
    F t2 = r * sin(phi);
    F s = 0.5 * (1.0 + H.z);
    F t1_sqr = math::square(t1);
    t2 = (F(1.0) - s) * sqrt(F(1.0) - t1_sqr) + s * t2;
    F t2_sqr = math::square(t2);
    // Section 4.3: reprojection onto hemisphere
    math::vector3<F> Nh = t1 * T1 + t2 * T2 + sqrt(math::max2(F(0), F(1) - t1_sqr - t2_sqr)) * H;
    // Section 3.4: transforming the normal back to the ellipsoid configuration
    // Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
    math::nvector3<F> Ne = math::vector3<F>(alpha_x * Nh.x, alpha_y * Nh.y, math::max2(F(0), Nh.z));
    return Ne;
}

bool Disney::Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    const math::nvector3<F> Wo = -Ray.direction();
    const F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;

    bool result = false;
    bool chooseReflectRay = epsilon[0] < F(0.5);
    if (chooseReflectRay)
    {
        UVW uvw(N);
        F alpha = math::square(Roughness);        
        math::nvector3<F> Wi = SampleGGXVNDF(uvw.world_to_local(Wo), alpha, alpha, epsilon[1], epsilon[2]);
        Wi = uvw.local_to_world(Wi);
        const F NdotL = math::dot(Wi, N);
        const F Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);

        outLightRay.scattering.set_origin(P);
        outLightRay.scattering.set_direction(Wi);
        outLightRay.f = math::vector3<F>(Frehnel, Frehnel, Frehnel);
        outLightRay.isSpecular = true;
        result = math::dot(Wi, N) > F(0);
    }
    else
    {
        result = Lambertian::Scattering(epsilon, P, N, Ray, IsOnSurface, outLightRay);
        const math::nvector3<F>& Wi = outLightRay.scattering.direction();
        const F NdotL = math::dot(Wi, N);
        const F Frehnel = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
        outLightRay.f *= F(1) - Frehnel;
    }
    return result;
}

math::vector3<F> Disney::f(
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
    const F Fr = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    const F D = DistributionGGX(Roughness, NdotH);
    const F G = GeometryGGX(Roughness, NdotH, math::dot(H, Wi), math::dot(H, Wo));
    F TermS = F(0.25) * Fr * D * G / (math::dot(N, Wo) * math::dot(N, Wi));
    math::vector3<F> s(TermS, TermS, TermS);
    math::vector3<F> d = Lambertian::f(N, Wo, Wi, IsOnSurface) * (F(1) - Fr);
    return (s + d) * F(0.5);
}

F Disney::pdf(
    const math::nvector3<F>& N,
    const math::nvector3<F>& Wo,
    const math::nvector3<F>& Wi) const
{
    math::nvector3<F> H = Wo + Wi;
    F DiracApproxmation = math::near_zero(N - H) ? F(1) : F(0);
    F pdfSpecular = DiracApproxmation / math::saturate(math::dot(N, Wi));
    F pdfDiffuse = Lambertian::pdf(N, Wo, Wi);
    return F(0.5) * (pdfDiffuse + pdfSpecular);
}
