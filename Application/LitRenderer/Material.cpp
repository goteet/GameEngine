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


struct UVW
{
    math::vector3<F> u, v, w;

    UVW(const math::vector3<F>& normal) { from_normal(normal); }

    void from_normal(const math::vector3<F>& normal)
    {
        w = normalized(normal);
        v = fabs(w.x) > F(0.95) ? math::normalized_vector3<F>::unit_y() : math::normalized_vector3<F>::unit_x();
        v = normalized(math::cross(w, v));
        u = normalized(math::cross(w, v));
    }

    math::vector3<F> local(F x, F y, F z)
    {
        return normalized(x * u + y * v + z * w);
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

math::normalized_vector3<F> GenerateHemisphereDirection(F epsilon[2], const math::vector3<F>& normal)
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
    return uvw.local(x, y, z);
}

math::normalized_vector3<F> GenerateUniformHemisphereDirection(F epsilon[2], const math::vector3<F>& normal)
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

    return UVW(normal).local(x, y, z);
}


math::vector3<F> GenerateCosineWeightedHemisphereDirection(F epsilon[2], const math::vector3<F>& normal)
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
    return uvw.local(x, y, z);
}



bool Lambertian::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    math::normalized_vector3<F> Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(Wi);
    outLightRay.f = f(N, Ray.direction(), Wi, IsOnSurface);
    return math::dot(Wi, N) > F(0);
}

math::vector3<F> Lambertian::f(
    const math::normalized_vector3<F>& N,
    const math::normalized_vector3<F>& Wo,
    const math::normalized_vector3<F>& Wi,
    bool IsOnSurface) const
{
    return Albedo * math::InvPI<F>;
}

F Lambertian::pdf(
    const math::normalized_vector3<F>& N,
    const math::normalized_vector3<F>& Wo,
    const math::normalized_vector3<F>& Wi) const
{
    F cosTheta = math::dot(N, Wi);
    F pdfLambertian = math::max2(F(0), cosTheta) * math::InvPI<F>;
    return pdfLambertian;
}



OrenNayer::OrenNayer(F r, F g, F b, math::radian<F> sigma)
    : IMaterial(r, g, b)
    , Sigma(sigma)
    , SigmaSqr(math::sqr(sigma.value))
{
    A = F(1) - F(0.5) * SigmaSqr.value / (SigmaSqr.value + F(0.33));
    B = F(0.45) * SigmaSqr.value / (SigmaSqr.value + F(0.09));
}


math::vector3<F> OrenNayer::f(
    const math::normalized_vector3<F>& N,
    const math::normalized_vector3<F>& Wo,
    const math::normalized_vector3<F>& Wi,
    bool IsOnSurface) const
{
    F cosWi = math::clamp(math::dot(N, Wi)); F sinWi = sqrt(F(1) - math::sqr(cosWi));
    F cosWo = math::clamp(math::dot(N, Wo)); F sinWo = sqrt(F(1) - math::sqr(cosWo));

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

math::normalized_vector3<F> Reflect(
    const math::normalized_vector3<F>& In,
    const math::normalized_vector3<F>& N)
{
    return In - F(2) * math::dot(In, N) * N;
}

bool Metal::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    math::vector3<F> FuzzyDirection = Fuzzy * GenerateUnitSphereVector();
    math::normalized_vector3<F> reflectDirection = Reflect(Ray.direction(), N) + FuzzyDirection;
    outLightRay.isSpecular = true;
    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(reflectDirection);
    outLightRay.f = math::vector3<F>::one();
    F cosTheta = math::dot(N, reflectDirection);
    return cosTheta > F(0);
}

F ReflectanceSchlick(F CosTheta, F eta1, F eta2)
{
    // Use Schlick's approximation for reflectance.
    F F0 = (eta1 - eta2) / (eta1 + eta2);
    F R0 = F0 * F0;
    F Base = F(1) - CosTheta;
    return R0 + (F(1) - R0) * math::power<5>(Base);
}

bool Dielectric::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    const F AirRefractiveIndex = F(1.0003);

    const math::vector3<F>& InDirection = Ray.direction();

    F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const F RefractionRatio = eta1 / eta2;

    const F IdotN = -math::dot(InDirection, N);
    F CosTheta = math::clamp(IdotN);
    F SinThetaSqr = math::clamp(F(1) - CosTheta * CosTheta);
    F Det = F(1) - SinThetaSqr * RefractionRatio * RefractionRatio;
    bool RefractRay = Det >= F(0);

    bool ReflectRay = epsilon[0] < ReflectanceSchlick(CosTheta, eta1, eta2);
    math::vector3<F> ScatteredDirection;
    if (!RefractRay || ReflectRay)
    {
        ScatteredDirection = Reflect(InDirection, N);
    }
    else
    {
        math::vector3<F> Rprep = RefractionRatio * (InDirection + CosTheta * N);
        math::vector3<F> Rpall = N * sqrt(Det);
        ScatteredDirection = Rprep - Rpall;
    }

    outLightRay.scattering.set_origin(P);
    outLightRay.scattering.set_direction(ScatteredDirection);
    outLightRay.f = math::vector3<F>::one();
    return true;
}

bool PureLight_ForTest::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{
    return false;
}

math::vector3<F> PureLight_ForTest::Emitting() const
{
    return Emission;
}
const F AirRefractiveIndex = F(1.0003);
bool Glossy::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const
{

    const math::vector3<F>& Wo = Ray.direction();
    const F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const F IdotN = -math::dot(Wo, N);
    const F cosTheta = math::clamp(IdotN);
    const F Frehnel = ReflectanceSchlick(cosTheta, eta1, eta2);

    bool result = false;
    bool chooseReflectRay = epsilon[0] < SpecularSampleProbability;
    if (chooseReflectRay)
    {
        math::normalized_vector3<F> Wr = Reflect(Wo, N);
        outLightRay.scattering.set_origin(P);
        outLightRay.scattering.set_direction(Wr);
        outLightRay.f = math::vector3<F>(Frehnel, Frehnel, Frehnel);
        outLightRay.isSpecular = true;
        result = math::dot(Wr, N) > F(0);
    }
    else
    {
        result = Lambertian::Scattering(epsilon, P, N, Ray, IsOnSurface, outLightRay);
        outLightRay.f *= F(1) - Frehnel;
    }
    return result;
}

math::vector3<F> Glossy::f(
    const math::normalized_vector3<F>& N,
    const math::normalized_vector3<F>& Wo,
    const math::normalized_vector3<F>& Wi,
    bool IsOnSurface) const
{
    const math::normalized_vector3<F> H = Wo + Wi;
    const F NdotH = math::dot(N, H);
    const F eta1 = IsOnSurface ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsOnSurface ? RefractiveIndex : AirRefractiveIndex;
    const F IdotN = -math::dot(Wo, N);
    const F cosTheta = math::clamp(IdotN);
    const F Frehnel = ReflectanceSchlick(cosTheta, eta1, eta2);

    const math::vector3<F> S(Frehnel, Frehnel, Frehnel);
    const math::vector3<F> D = Lambertian::f(N, Wo, Wi, IsOnSurface) * (F(1) - Frehnel);

    return math::near_zero(H - N) ? S : D;
}

F Glossy::pdf(
    const math::normalized_vector3<F>& N,
    const math::normalized_vector3<F>& Wo,
    const math::normalized_vector3<F>& Wi) const
{
    math::normalized_vector3<F> H = Wo + Wi;
    F DiracApproxmation = math::near_zero(N - H) ? F(1) : F(0);
    F pdfSpecular = DiracApproxmation / math::clamp(math::dot(N, Wi));
    F pdfDiffuse = Lambertian::pdf(N, Wo, Wi);
    return (F(1) - SpecularSampleProbability) * pdfDiffuse + SpecularSampleProbability * pdfSpecular;
}
