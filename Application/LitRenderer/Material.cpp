#include <tuple>
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

Float FresnelSchlick(Float CosTheta, Float R0)
{
    return R0 + (Float(1) - R0) * math::power<5>(Float(1) - CosTheta);
}

Float DistributionGTR2(Float AlphaSquare, Float NdotH)
{
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
    Float denominator = math::square(cos) * (AlphaSquare - 1) + 1;
    return AlphaSquare * math::InvPI<Float> / math::square(denominator);
}

//still error with GTR2, so I copy this from filament's description
Float DistributionGGX_FromFilament(Float roughness, Float NdotH)
{
    Float alpha = NdotH * roughness;
    Float cos = math::saturate(NdotH);
    Float numerator = roughness;
    Float denominator = Float(1) - math::square(cos) + math::square(alpha);
    return math::square(numerator / denominator) * math::InvPI<Float>;
}

Float DistributionGTR1(Float roughness, Float HdotN)
{
    Float alpha = math::square(roughness);
    //Float X = HdotN > 0 ? Float(1) : Float(0);
    //      alpha^2 * X(n.m)                       alpha^2 * X                  (0.5 * alpha * X)^2
    //--------------------------------- = ------------------------------- = ---------------------------
    // 4 * cos^2 * (alpha^2 + tan^2)       4 * (alpha^2 * cos^2 + sin^2)     cos^2 * (alpha^2 - 1) + 1
    //
    // Note that X is already hide in saturate() operation.
    Float cos = math::saturate(HdotN);
    Float numerator = Float(0.5) * alpha;// *X;
    Float denominator = math::square(cos) * (math::square(alpha) - Float(1)) + 1;
    return math::square(numerator) / denominator;
}

Float DistributionBerry(Float roughness, Float NdotH)
{
    return DistributionGTR1(roughness, NdotH);
}

Float ShadowingGGX(Float AlphaSquare, Float HdotV, Float NdotH)
{
    Float cos = math::saturate(HdotV);
    Float cos2 = math::square(cos);
    Float X = (NdotH / HdotV) > 0 ? Float(1) : Float(0);
    //          2 * X                                       2 * cos * X                                     2 * cos * X
    //--------------------------------- = ------------------------------------------- = ---------------------------------------------
    // 1 + sqrt(1 + AlphaG^2 * tan^2)      cos + sqrt(cos^2 + alpha^2 * (1 - cos^2))     cos + sqrt(cos^2 * (1 - alpha^2) + alpha^2)
    //
    Float numerator = Float(2) * cos * X;
    Float denominator = cos + sqrt(cos2 * (Float(1) - AlphaSquare) + AlphaSquare);
    return numerator / denominator;
}

Float GeometryGGX(Float roughness, Float NdotH, Float HdotV, Float HdotL)
{
    Float AlphaSquare = math::power<4>(roughness);
    Float G1 = ShadowingGGX(AlphaSquare, HdotV, NdotH);
    Float G2 = ShadowingGGX(AlphaSquare, HdotL, NdotH);
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

Direction GenerateHemisphereDirection(const Float uTheta, const Float uPhi, const Direction& normal)
{
    Float cosTheta = Float(1) - Float(2) * uTheta;
    Float sinTheta = sqrt(Float(1) - cosTheta * cosTheta);
    Radian phi(math::TWO_PI<Float> *uPhi);
    Float cosPhi = math::cos(phi);
    Float sinPhi = math::sin(phi);

    Float x = sinTheta * cosPhi;
    Float y = sinTheta * sinPhi;
    Float z = cosTheta;

    UVW uvw(normal);
    return uvw.local_to_world(x, y, z);
}

Direction GenerateUniformHemisphereDirection(const Float uTheta, const Float uPhi, const Direction& normal)
{
    // pdf = 1/2PI
    // => cdf = 1/2PI*phi*(1-cos_theta)
    // => f_phi = 1/2PI*phi       --> phi(x) = 2*PI*x
    // => f_theta = 1-cos_theta   --> cos_theta(x) = 1-x = x'
    Float cosTheta = uTheta; //replace 1-e to e'
    Float sinTheta = sqrt(Float(1) - cosTheta * cosTheta);
    Radian phi(math::TWO_PI<Float> *uPhi);
    Float cosPhi = math::cos(phi);
    Float sinPhi = math::sin(phi);

    Float x = sinTheta * cosPhi;
    Float y = sinTheta * sinPhi;
    Float z = cosTheta;

    return UVW(normal).local_to_world(x, y, z);
}

Direction SampleGGXVNDF(
    const Direction& Wo,  // Input Wo: view direction
    Float AlphaX, Float AlphaY,                   // Input alpha_x, alpha_y: roughness parameters
    Float u1, Float u2                              // Input U1, U2: uniform random numbers
)
{
    // stretch view
    Direction V = Wo * math::vector3<Float>(AlphaX, AlphaY, Float(1));

    // orthonormal basis
    Direction T1 = V.z < Float(0.99999)
        ? math::cross(V, Direction::unit_z())
        : Direction::unit_x();
    Direction T2 = math::cross(V, T1);

    // sample point with polar coordinates (r, phi)
    Float a = Float(1) / (Float(1) + V.z);
    Float r = sqrt(u1);
    Radian phi = (u2 < a)
        ? Radian(u2 / a * math::PI<Float>)
        : Radian(math::PI<Float> +(u2 - a) / (Float(1) - a) * math::PI<Float>);

    Float t1 = r * cos(phi);
    Float t2 = r * sin(phi) * ((u2 < a) ? Float(1) : V.z);

    // compute normal
    Spectrum N = T1 * t1 + T2 * t2 + V * sqrt(math::max2(Float(0), Float(1) - math::square(t1) - math::square(t2)));

    // unstretch
    // note this will do the normalize() operation.
    Direction Wm = math::vector3<Float>(AlphaX * N.x, AlphaY * N.y, math::max2(Float(0), N.z));
    return Wm;
}

Spectrum GenerateCosineWeightedHemisphereDirection(const Float uTheta, const Float uPhi, const Direction& normal)
{
    // pdf = cos(theta) / Pi.
    Float cosTheta_sqr = uTheta; //replace 1-e to e'
    Float cosTheta = sqrt(cosTheta_sqr);
    Float sinTheta = sqrt(Float(1) - cosTheta_sqr);
    Radian phi(Float(2) * math::PI<Float> *uPhi);
    Float cosPhi = math::cos(phi);
    Float sinPhi = math::sin(phi);

    Float x = sinTheta * cosPhi;
    Float y = sinTheta * sinPhi;
    Float z = cosTheta;

    UVW uvw(normal);
    return uvw.local_to_world(x, y, z);
}

DistributionGGX::DistributionGGX(Float roughness)
    : Roughness(roughness)
    , Alpha(math::power<2>(roughness))
    , AlphaSquare(math::power<4>(roughness))
{
}

Direction DistributionGGX::SampleWh(const Direction& Wo, Float u1, Float u2) const
{
    return SampleGGXVNDF(Wo, Alpha, Alpha, u1, u2);
}

Float DistributionGGX::D(Float NdotH) const
{
    return DistributionGTR2(AlphaSquare, NdotH);
}

Float DistributionGGX::G(Float NdotH, Float HdotV, Float HdotL) const
{
    Float G1 = ShadowingGGX(AlphaSquare, HdotV, NdotH);
    Float G2 = ShadowingGGX(AlphaSquare, HdotL, NdotH);
    return math::min2(G1, G2);
}

Float DistributionGGX::pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const
{
    return Float();
}



bool Lambertian::SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const
{
    //  f * cos(theta)     rho * cos(theta)        Pi
    // ---------------- = ----------------- * ------------ = rho
    //      pdf                 Pi             cos(theta)  
    oBSDFSample.Wi = GenerateCosineWeightedHemisphereDirection(u[1], u[2], N);
    oBSDFSample.F = Albedo;
    oBSDFSample.SampleMask = BSDFMask::Diffuse;
    const Float NdotL = math::dot(oBSDFSample.Wi, N);
    return NdotL >= Float(0);
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



OrenNayer::OrenNayer(const Spectrum& albedo, Radian sigma, Float weight)
    : BSDF("Oren-Nayer", BSDFMask::Diffuse, weight)
    , Albedo(albedo)
    , Sigma(sigma)
    , SigmaSquare(math::square(sigma.value))
{
    A = Float(1) - Float(0.5) * SigmaSquare.value / (SigmaSquare.value + Float(0.33));
    B = Float(0.45) * SigmaSquare.value / (SigmaSquare.value + Float(0.09));
}

std::tuple<Float, Float> CalculateFactorAndCosineWi(const Direction& N, const Direction& Wi, const Direction& Wo, const Float A, const Float B)
{
    const Float CosineWi = math::dot(N, Wi);
    const Float CosineWo = math::dot(N, Wo);
    const Float SineWi = sqrt(Float(1) - math::saturate(math::square(CosineWi)));
    const Float SineWo = sqrt(Float(1) - math::saturate(math::square(CosineWo)));

    Float SineAlpha, TangentBeta;
    bool bIsWiGreater = CosineWi < CosineWo;
    if (bIsWiGreater)
    {
        SineAlpha = SineWi;
        TangentBeta = SineWo / CosineWo;
    }
    else
    {
        SineAlpha = SineWo;
        TangentBeta = SineWi / CosineWi;
    }

    Float maxWi_Wo = math::max2(Float(0), CosineWi * CosineWo + SineWi * SineWo);
    Float factor = A + B * maxWi_Wo * SineAlpha * TangentBeta;
    return { factor, CosineWi };
}

bool OrenNayer::SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const
{
    //  f * cos(theta)                           rho                     Pi
    // ---------------- = factor * cosine(Wi) * ----- * cos(theta) * ------------ = factor * cosine(Wi) * rho
    //      pdf                                   Pi                  cos(theta) 
    // *  Float NdotL = math::dot(N, Wi);

    const Direction Wi = GenerateCosineWeightedHemisphereDirection(u[1], u[2], N);
    const Direction Wo = -Ray.direction();
    Float factor, CosineWi;
    std::tie(factor, CosineWi) = CalculateFactorAndCosineWi(N, Wi, Wo, A, B);
    oBSDFSample.Wi = Wi;
    oBSDFSample.F = (factor * CosineWi) * Albedo;
    oBSDFSample.SampleMask = BSDFMask::Diffuse;
    return CosineWi >= Float(0);
}

Spectrum OrenNayer::f(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi,
    bool IsOnSurface) const
{
    Float factor, CosineWi;
    std::tie(factor, CosineWi) = CalculateFactorAndCosineWi(N, Wi, Wo, A, B);
    return (factor * CosineWi * math::InvPI<Float>) * Albedo;
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

bool TorranceSparrow::SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const
{
    // f * cos(theta)        D * F * G                      4 * HdotV     F * G * HdotV
    //--------------- = ------------------- * cos(theta) * ----------- = ---------------
    //     pdf           4 * NdotV * NdotL                  D * NdotH     NdotV * NdotH
    //
    // Note that cos(theta) = NdotL
    //           Distribution::Pdf  = D * cos(theta),
    //           Distribution::Pdf' = D * G1 * HdotV / NdotV, this will be special due to sampling visible area.
    const Direction Wo = -Ray.direction();
    const Float eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const Float eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;

    UVW uvw(N);
    const Direction Ho = Distribution->SampleWh(uvw.world_to_local(-Wo), u[1], u[2]);
    const Direction H = uvw.local_to_world(Ho);
    const Direction Wi = math::reflection(Wo, H);
    const Float NdotL = math::dot(Wi, N);
    const Float NdotH = math::dot(N, H);

    //const F D = DistributionGGX(Roughness, NdotH);
    const Float F = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    const Float G = Distribution->G(NdotH, math::dot(H, Wi), math::dot(H, Wo));
    oBSDFSample.Wi = Wi;
    oBSDFSample.F = (f(N, Wo, Wi, IsOnSurface) * NdotL / pdf(N, Wo, Wi)) * Spectrum::one();
    oBSDFSample.SampleMask = (Distribution->IsNearMirrorReflection()
        ? BSDFMask::Specular | BSDFMask::Reflection
        : BSDFMask::Specular);
    return NdotL > 0;
}

Spectrum TorranceSparrow::f(
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
    const Float F = FresnelSchlick(math::saturate(NdotL), eta1, eta2);
    const Float D = Distribution->D(NdotH);
    const Float G = Distribution->G(NdotH, math::dot(H, Wi), math::dot(H, Wo));
    const Float BRDF = Float(0.25) * D * F * G / (NdotV * NdotL);
    return NdotL > Float(0) ? Spectrum::one() * BRDF : Spectrum::zero();
}

Float TorranceSparrow::pdf(
    const Direction& N,
    const Direction& Wo,
    const Direction& Wi) const
{
    //copy from Pbrt, same with UE4.
    // D * NdotH
    //-----------
    // 4 * HdotV
    const Direction H = Wo + Wi;
    const Float NdotH = math::dot(N, H);
    const Float NdotL = math::dot(N, Wi);
    const Float HdotV = math::dot(H, Wo);
    const Float D = Distribution->D(NdotH);
    return D * NdotH * Float(0.25) / HdotV;
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

bool AshikhminAndShirley::SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const
{
    const Direction Wo = -Ray.direction();
    Direction Wi;
    if (u[0] < Float(0.5))
    {
        //sample diffuse
        Wi = GenerateCosineWeightedHemisphereDirection(u[1], u[2], N);
        oBSDFSample.SampleMask = BSDFMask::Diffuse;
    }
    else
    {
        //sample specular
        UVW uvw(N);
        const Direction Ho = Distribution->SampleWh(uvw.world_to_local(-Wo), u[1], u[2]);
        const Direction H = uvw.local_to_world(Ho);
        Wi = math::reflection(Wo, H);
        oBSDFSample.SampleMask = (Distribution->IsNearMirrorReflection()
            ? BSDFMask::Specular | BSDFMask::Reflection
            : BSDFMask::Specular);
    }

    const Float NdotL = math::dot(Wi, N);
    oBSDFSample.Wi = Wi;
    oBSDFSample.F = f(N, Wo, Wi, IsOnSurface) * NdotL / pdf(N, Wo, Wi);
    return NdotL >= Float(0);
}

Spectrum AshikhminAndShirley::f(const Direction& N, const Direction& Wo, const Direction& Wi, bool IsOnSurface) const
{
    const Direction H = Wo + Wi;
    if (math::near_zero(H))
    {
        return Spectrum::zero();
    }
    const Float HdotL = math::dot(H, Wi);
    const Float NdotL = math::dot(N, Wi);
    const Float NdotV = math::dot(N, Wo);
    const Float NdotH = math::dot(N, H);
    const Float D = Distribution->D(NdotH);
    const Float F = FresnelSchlick(HdotL, Rs);

    // 28*Rd 
    //------- * (1-Rs) *(1 - pow5(1 - 0.5 * NdotL)) * (1 - pow5(1 - 0.5 * NdotV))
    // 23*Pi
    Spectrum Diffse = (Float(28) * Rd * math::InvPI<Float> / Float(23))
        * (Float(1) - Rs)
        * (Float(1) - math::power<5>(Float(1) - Float(0.5) * NdotL))
        * (Float(1) - math::power<5>(Float(1) - Float(0.5) * NdotV))
        * Spectrum::one();

    //         D * F
    //-------------------------------
    // 4 * HdotL * max(NdotL, NdotV)
    Spectrum Specular = (D * F * Float(0.25)) / (HdotL * math::max2(NdotL, NdotV)) * Spectrum::one();


    return Diffse + Specular;
}

Float AshikhminAndShirley::pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const
{
    const Direction H = Wo + Wi;
    if (math::near_zero(H))
    {
        return Float(0);
    }

    Float NdotH = math::dot(N, H);
    Float NdotL = math::dot(N, Wi);
    Float HdotV = math::dot(H, Wo);
    Float D = Distribution->D(NdotH);
    //copy from Pbrt, same with UE4.
    // D * NdotH
    //-----------
    // 4 * HdotV
    Float SpecularPdf = D * NdotH * Float(0.25) / HdotV;
    Float DiffusePdf = math::saturate(NdotL) * math::InvPI<Float>;

    return Float(0.5) * (SpecularPdf + DiffusePdf);
}
