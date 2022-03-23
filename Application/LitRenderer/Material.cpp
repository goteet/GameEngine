#include "Material.h"
#include "LitRenderer.h"


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



bool Lambertian::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const
{
    math::normalized_vector3<F> Wi = GenerateCosineWeightedHemisphereDirection(epsilon + 1, N);
    outBrdf = Albedo / math::PI<F>;
    outScattering.set_origin(P);
    outScattering.set_direction(Wi);
    return math::dot(Wi, N) > F(0);
}

F Lambertian::pdf(const math::normalized_vector3<F>& N,
    const math::normalized_vector3<F>& Wo,
    const math::normalized_vector3<F>& Wi) const
{
    F cosTheta = math::dot(N, Wi);
    F pdfLambertian = math::max2(F(0), cosTheta) * math::InvPI<F>;
    return pdfLambertian;
}

math::normalized_vector3<F> Reflect(
    const math::normalized_vector3<F>& In,
    const math::normalized_vector3<F>& N
)
{
    return In - F(2) * math::dot(In, N) * N;
}

bool Metal::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const
{
    math::vector3<F> FuzzyDirection = Fuzzy * GenerateUnitSphereVector();
    math::normalized_vector3<F> reflectDirection = Reflect(Ray.direction(), N) + FuzzyDirection;
    outScattering.set_origin(P);
    outScattering.set_direction(reflectDirection);
    outBrdf = math::vector3<F>::one();
    F cosTheta = math::dot(N, reflectDirection);
    return cosTheta > F(0);
}

F Power5(F Base)
{
    F Ret = Base * Base;
    return Ret * Ret * Base;
}

F ReflectanceSchlick(F CosTheta, F eta1, F eta2)
{
    // Use Schlick's approximation for reflectance.
    F F0 = (eta1 - eta2) / (eta1 + eta2);
    F R0 = F0 * F0;
    F Base = F(1) - CosTheta;
    return R0 + (F(1) - R0) * Power5(Base);
}

bool Dielectric::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const
{
    const F AirRefractiveIndex = F(1.0003);

    const math::vector3<F>& InDirection = Ray.direction();

    F eta1 = IsFrontFace ? AirRefractiveIndex : RefractiveIndex;
    F eta2 = IsFrontFace ? RefractiveIndex : AirRefractiveIndex;
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

    outScattering.set_origin(P);
    outScattering.set_direction(ScatteredDirection);
    outBrdf = math::vector3<F>::one();
    return true;
}

bool PureLight_ForTest::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const
{
    return false;
}

math::vector3<F> PureLight_ForTest::Emitting() const
{
    return Emission;
}
bool Glossy::Scattering(F epsilon[3], const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const
{
    const F AirRefractiveIndex = F(1.0003);

    const math::vector3<F>& Wo = Ray.direction();
    const F eta1 = IsFrontFace ? AirRefractiveIndex : RefractiveIndex;
    const F eta2 = IsFrontFace ? RefractiveIndex : AirRefractiveIndex;
    const F IdotN = -math::dot(Wo, N);
    const F cosTheta = math::clamp(IdotN);
    const F Frehnel = ReflectanceSchlick(cosTheta, eta1, eta2);

    bool result = false;
    bool chooseReflectRay = epsilon[0] < SpecularSampleProbability;
    if (chooseReflectRay)
    {
        math::normalized_vector3<F> Wr = Reflect(Wo, N);
        outScattering.set_origin(P);
        outScattering.set_direction(Wr);
        outBrdf = math::vector3<F>(Frehnel, Frehnel, Frehnel);
        result = math::dot(Wr, N) > F(0);
    }
    else
    {
        result = Lambertian::Scattering(epsilon, P, N, Ray, IsFrontFace, outScattering, outBrdf);
        outBrdf *= F(1) - Frehnel;
    }
    return result;
}

F Glossy::pdf(const math::normalized_vector3<F>& N,
    const math::normalized_vector3<F>& Wo,
    const math::normalized_vector3<F>& Wi) const
{
    F pdfDiffuse = Lambertian::pdf(N, Wo, Wi);
    F pdfSpecular = math::max2(F(0), math::dot(Wo, N));
    return (F(1) - SpecularSampleProbability) * pdfDiffuse + SpecularSampleProbability * pdfSpecular;
}
