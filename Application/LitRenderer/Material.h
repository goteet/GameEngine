#pragma once

#include <random>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Geometry.h>

using F = double;
class Scene;

template<typename T>
struct random
{
    random() : generator(randomDevice()), distribution(F(0), F(1)) { }
    T operator()() { return _value(); }
    T operator()(T range_value) { return _range(range_value); }
    static random<T>& instance() { static random<T> instance; return instance; }
    static T value() { return instance()(); }
    static T range(T range_value) { return instance()(range_value); }

private:
    T _value() { return distribution(generator); }
    T _range(T range_value) { return _value() * T(2) * range_value - range_value; }

    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<F> distribution;
};


F BalanceHeuristic(F pdfA, F pdfB);
F PowerHeuristic(F pdfA, F pdfB);

struct LightRay
{
    bool isSpecular = false;
    math::ray3d<F> scattering;
    math::vector3<F> f = math::vector3<F>::zero();
};

struct IMaterial
{
    math::vector3<F> Albedo = math::vector3<F>::one();

    IMaterial() = default;
    IMaterial(F r, F g, F b) : Albedo(r, g, b) { }
    virtual ~IMaterial() { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const = 0;
    virtual math::vector3<F> Emitting() const
    {
        return math::vector3<F>::zero();
    }
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const
    {
        return math::vector3<F>::zero();
    }
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const
    {
        return F(1);
    }
};

struct Lambertian : public virtual IMaterial
{
    Lambertian() = default;
    Lambertian(F r, F g, F b) : IMaterial(r, g, b) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const override;
};

struct OrenNayer : public virtual Lambertian
{
    math::radian<F> Sigma = math::radian<F>(0);
    math::radian<F> SigmaSqr = math::radian<F>(0);
    F A = F(1);
    F B = F(0);
    OrenNayer() = default;
    OrenNayer(F r, F g, F b, math::radian<F> sigma = math::radian<F>(0));
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
};

struct Metal : public IMaterial
{
    F Fuzzy = F(0);

    Metal() = default;
    Metal(F f) : Fuzzy(f) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
};

struct Glossy : public Lambertian
{
    F RefractiveIndex = F(2.5);
    F SpecularSamplingProbability = F(0.5);
    F DiffuseSamplingProbability = F(1) - SpecularSamplingProbability;

    Glossy() = default;
    Glossy(F r, F g, F b, F ior = F(1.5)) : IMaterial(r, g, b), RefractiveIndex(ior) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const override;
};

struct Disney : public Lambertian
{
    F Roughness = F(0.5);
    F RefractiveIndex = F(2.5);

    Disney() = default;
    Disney(F r, F g, F b, F rf, F ior = F(1.5)) : IMaterial(r, g, b), Roughness(rf), RefractiveIndex(ior) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const override;
};

struct Dielectric : public IMaterial
{
    F RefractiveIndex = F(1.0003);

    Dielectric() = default;
    Dielectric(F ior) : RefractiveIndex(ior) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
};

struct PureLight_ForTest : IMaterial
{
    math::vector3<F> Emission = math::vector3<F>::one();

    PureLight_ForTest() = default;
    PureLight_ForTest(F x, F y, F z) : Emission(x, y, z) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> Emitting() const override;
};
