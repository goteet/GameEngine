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


struct IMaterial
{
    math::vector3<F> Albedo = math::vector3<F>::one();

    IMaterial() = default;
    IMaterial(F r, F g, F b) : Albedo(r, g, b) { }
    virtual ~IMaterial() { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const = 0;
    virtual math::vector3<F> Emitting() const { return math::vector3<F>::zero(); }
    virtual F pdf(const math::normalized_vector3<F>& N, const math::normalized_vector3<F>& Wo, const math::normalized_vector3<F>& Wi) const { return F(1); }
};

struct Lambertian : public virtual IMaterial
{

    Lambertian() = default;
    Lambertian(F r, F g, F b) : IMaterial(r, g, b) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const override;
    virtual F pdf(const math::normalized_vector3<F>& N,
        const math::normalized_vector3<F>& Wo,
        const math::normalized_vector3<F>& Wi) const override;
};

struct Metal : public IMaterial
{
    F Fuzzy = F(0);

    Metal() = default;
    Metal(F f) : Fuzzy(f) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const override;
};

struct Glossy : public Lambertian
{
    mutable random<F> RandGenerator;
    F RefractiveIndex = F(2.5);
    F SpecularSampleProbability = F(0.5);

    Glossy() = default;
    Glossy(F r, F g, F b, F ior = F(2.5)) : IMaterial(r, g, b), RefractiveIndex(ior) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const override;
    virtual F pdf(const math::normalized_vector3<F>& N,
        const math::normalized_vector3<F>& Wo,
        const math::normalized_vector3<F>& Wi) const override;
};

struct Dielectric : public IMaterial
{
    F RefractiveIndex = F(1.0003);

    Dielectric() = default;
    Dielectric(F ior) : RefractiveIndex(ior) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const override;
};

struct PureLight_ForTest : IMaterial
{
    math::vector3<F> Emission = math::vector3<F>::one();

    PureLight_ForTest() = default;
    PureLight_ForTest(F x, F y, F z) : Emission(x, y, z) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf) const override;
    virtual math::vector3<F> Emitting() const override;
};
