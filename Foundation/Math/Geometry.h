#pragma once
#include "Vector.h"

namespace math
{
    enum normalize_hint { norm };
    template<typename value_type, EDim> struct point;
    template<typename value_type, EDim, typename> struct ray;

    template<typename value_type>
    struct point<value_type, EDim::_2> : public vector_t<value_type, EDim::_2>
    {
        constexpr point() = default;
        constexpr point(const point&) = default;
        constexpr point(const vector_t<value_type, EDim::_2>& r) : vector_t<value_type, EDim::_2>(r) { };
        constexpr point(const vector_t<value_type, EDim::_3>& r) : vector_t<value_type, EDim::_3>(r.x, r.y) { };
        constexpr point(value_type x, value_type y) : vector_t<value_type, EDim::_2>(x, y) { }
        constexpr operator vector_t<value_type, EDim::_3>() const { return vector_t<value_type, EDim::_3>(*this, value_type(1)); }

        static constexpr point origin() { return point(value_type(0), value_type(0)); }
    };

    template<typename value_type>
    struct point<value_type, EDim::_3> : public vector_t<value_type, EDim::_3>
    {
        constexpr point() = default;
        constexpr point(const point&) = default;
        constexpr point(const vector_t<value_type, EDim::_3>& r) : vector_t<value_type, EDim::_3>(r) { };
        constexpr point(const vector_t<value_type, EDim::_4>& r) : vector_t<value_type, EDim::_3>(r.x, r.y, r.z) { };
        constexpr point(value_type x, value_type y, value_type z) : vector_t<value_type, EDim::_3>(x, y, z) { }
        constexpr point(const vector_t<value_type, EDim::_2>& r, scaler_t<value_type> z) : vector_t<value_type, EDim::_3>(r, z) { };
        constexpr operator vector_t<value_type, EDim::_4>() const { return vector_t<value_type, EDim::_4>(*this, value_type(1)); }

        static constexpr point origin() { return point_t(value_type(0), value_type(0), value_type(0)); }
    };

    template<typename value_type, EDim dimension, typename = std::enable_if<dimension == EDim::_2 || dimension == EDim::_3>::type>
    struct ray
    {
        ray() = default;
        ray(const point<value_type, dimension>& o, const vector_t<value_type, dimension>& d) : _origin(o), _direction(normalized(d)) { }
        ray(const vector_t<value_type, (EDim)(dimension + 1)>& o, const vector_t<value_type, dimension>& d) : _origin(o), _direction(normalized(d)) { }
        ray(const point<value_type, dimension>& o, const vector_t<value_type, dimension>& d, normalize_hint) : _origin(o), _direction(d) { }
        ray(const vector_t<value_type, (EDim)(dimension + 1)>& o, const vector_t<value_type, dimension>& d, normalize_hint) : _origin(o), _direction(d) { }
        constexpr const point<value_type, dimension>& origin() const { return _origin; }
        constexpr const vector_t<value_type, dimension>& direction() const { return _direction; }
        constexpr void set_origin(const point<value_type, dimension>& o) { _origin = o; }
        constexpr void set_direction(const vector_t<value_type, dimension>& dir) { _direction = normalized(dir); }
        constexpr void set_direction(const vector_t<value_type, dimension>& dir, normalize_hint) { _direction = dir; }
        constexpr vector_t<value_type, dimension> calc_offset(scaler_t<value_type> length) const { return _origin + _direction * length; }

    private:
        point<value_type, dimension> _origin;
        vector_t<value_type, dimension> _direction;
    };

    template<typename value_type>
    struct sphere
    {
        sphere(const point< value_type, EDim::_3>& c, value_type r) : _center(c) { set_radius(r); };
        void set_center(const point<value_type, EDim::_3>& c) { _center = c; }
        void set_radius(value_type r) { if (r <= value_type(0)) r = EPSILON<value_type>; _radius = r; _radius_sqr = r * r; }
        constexpr const point<value_type, EDim::_3>& center() const { return _center; }
        constexpr value_type radius() const { return _radius; }
        constexpr value_type radius_sqr() const { return _radius_sqr; }
    private:
        point<value_type, EDim::_3> _center;
        value_type _radius, _radius_sqr;
    };


    enum class intersection : unsigned char
    {
        none, same,
        inside, contain,
        intersect, tangent
    };

    template<typename value_type>
    intersection intersect(const ray<value_type, EDim::_3>& ray, const sphere<value_type>& sphere,
        value_type& t0, value_type& t1)
    {
        //dot(dir, dir)*t^2 + 2*dot(dir, Or-Os)*t + dot(Or-Os, Or-Os) - r^2 = 0
        //a = dot(Dr,Dr)
        //b = 2*dot(Dr, Or-Os)
        //c = dot(Or-Os,Or-Os) - r^2
        //t = (-b +- sqrt(b^2 - 4ac)) / 2a
        vector_t<value_type, EDim::_3> Or_Os = ray.origin() - sphere.center();
        const vector_t<value_type, EDim::_3> Dr = ray.direction();
        value_type a = dot(Dr, Dr);
        value_type b = 2 * dot(Dr, Or_Os);
        value_type c = dot(Or_Os, Or_Os) - sphere.radius_sqr();
        value_type det = b * b - 4 * a*c;
        if (det > value_type(0))
        {
            det = sqrt(det);
            value_type inv2a = value_type(0.5) / a;
            t0 = (-b - det) * inv2a;
            t1 = (-b + det) * inv2a;
            return (t0 < 0)
                ? intersection::inside
                : intersection::intersect;
        }
        else  if (det < value_type(0))
        {
            return intersection::none;
        }
        else
        {
            t0 = t1 = -b * value_type(0.5) / a;
            return intersection::tangent;
        }
    }

    template<typename value_type> using point2d = point<value_type, EDim::_2>;
    template<typename value_type> using point3d = point<value_type, EDim::_3>;
    template<typename value_type> using ray2d = ray<value_type, EDim::_2>;
    template<typename value_type> using ray3d = ray<value_type, EDim::_3>;
    using point2df = point2d<float>;
    using point3df = point3d<float>;
    using point2dd = point2d<double>;
    using point3dd = point3d<double>;
    using ray2df = ray2d<float>;
    using ray3df = ray3d<float>;
    using ray2dd = ray2d<double>;
    using ray3dd = ray3d<double>;
}