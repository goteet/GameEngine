#pragma once
#include "Vector.h"

namespace math
{
    enum normalize_hint { norm };
    enum dual_face_hint { dual_face };
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
        ray(const point<value_type, dimension>& o, const vector_t<value_type, dimension>& d) : _origin(o), _direction(normalized(d)) { _inv_direction = inversed(_direction); }
        ray(const vector_t<value_type, (EDim)(dimension + 1)>& o, const vector_t<value_type, dimension>& d) : _origin(o), _direction(normalized(d)) { _inv_direction = inversed(_direction); }
        ray(const point<value_type, dimension>& o, const vector_t<value_type, dimension>& d, normalize_hint) : _origin(o), _direction(d) { _inv_direction = inversed(_direction); }
        ray(const vector_t<value_type, (EDim)(dimension + 1)>& o, const vector_t<value_type, dimension>& d, normalize_hint) : _origin(o), _direction(d) { _inv_direction = inverse(_direction); }
        constexpr const point<value_type, dimension>& origin() const { return _origin; }
        constexpr point<value_type, dimension>& origin() { return _origin; }
        constexpr const vector_t<value_type, dimension>& direction() const { return _direction; }
        constexpr const vector_t<value_type, dimension>& inv_direction() const { return _inv_direction; }
        constexpr void set_origin(const point<value_type, dimension>& o) { _origin = o; }
        constexpr void set_direction(const vector_t<value_type, dimension>& dir) { _direction = normalized(dir); _inv_direction = inversed(_direction); }
        constexpr void set_direction(const vector_t<value_type, dimension>& dir, normalize_hint) { _direction = dir; _inv_direction = inversed(_direction); }
        constexpr point<value_type, dimension> calc_offset(scaler_t<value_type> length) const { return _origin + _direction * length; }

    private:
        point<value_type, dimension> _origin;
        vector_t<value_type, dimension> _direction, _inv_direction;
    };

    template<typename value_type>
    struct sphere
    {
        sphere(const point< value_type, EDim::_3>& c, value_type r) : _center(c) { set_radius(r); };
        void set_center(const point<value_type, EDim::_3>& c) { _center = c; }
        void set_radius(value_type r) { if (r <= value_type(0)) r = EPSILON<value_type>; _radius = r; _radius_sqr = r * r; }
        constexpr const point<value_type, EDim::_3>& center() const { return _center; }
        constexpr point<value_type, EDim::_3>& center() { return _center; }
        constexpr value_type radius() const { return _radius; }
        constexpr value_type radius_sqr() const { return _radius_sqr; }
    private:
        point<value_type, EDim::_3> _center;
        value_type _radius, _radius_sqr;
    };

    template<typename value_type>
    struct plane
    {
        plane(const point<value_type, EDim::_3>& p, const vector_t<value_type, EDim::_3>& n) :_position(p), _normal(normalized(n)) {}
        plane(const point<value_type, EDim::_3>& p, const vector_t<value_type, EDim::_3>& n, normalize_hint) :_position(p), _normal(n) {}
        void set_position(const point<value_type, EDim::_3>& p) { _position = p; }
        void set_normal(const vector_t<value_type, EDim::_3>& n) { _normal = normalized(n); }
        void set_normal(const vector_t<value_type, EDim::_3>& n, normalize_hint) { _normal = n; }
        constexpr const point<value_type, EDim::_3>& position() const { return _position; }
        constexpr point<value_type, EDim::_3>& position() { return _position; }
        constexpr const vector_t<value_type, EDim::_3>& normal() const { return _normal; }
    private:
        point<value_type, EDim::_3> _position;
        vector_t<value_type, EDim::_3> _normal;
    };

    template<typename value_type>
    struct disk : private plane<value_type>
    {
        //n,t
        disk(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n,
            scaler_t<value_type> r) : plane<value_type>(p, n) { set_radius(r); }

        //norm n
        disk(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n, normalize_hint,
            scaler_t<value_type> r) : plane<value_type>(p, n, norm) { set_radius(r); }

        void set_position(const point<value_type, EDim::_3>& p) { plane<value_type>::set_position(p); }
        void set_normal(const vector_t<value_type, EDim::_3>& n) { plane<value_type>::set_normal(n); }
        void set_normal(const vector_t<value_type, EDim::_3>& n, normalize_hint) { plane<value_type>::set_normal(n, normalize_hint::norm); }
        void set_radius(scaler_t<value_type> r) { _radius = math::max2(r, value_type(0)); }
        constexpr const point<value_type, EDim::_3>& position() const { return plane<value_type>::position(); }
        constexpr point<value_type, EDim::_3>& position() { return plane<value_type>::position(); }
        constexpr const vector_t<value_type, EDim::_3>& normal() const { return plane<value_type>::normal(); }
        value_type radius() const { return _radius; }
    private:
        value_type _radius;
    };

    template<typename value_type>
    struct rect : private plane<value_type>
    {
        //n,t,e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n,
            const vector_t<value_type, EDim::_3>& t,
            const vector_t<value_type, EDim::_2>& e) : plane<value_type>(p, n) { set_tangent(t); set_extends(e); }

        //norm n, t, e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n, normalize_hint,
            const vector_t<value_type, EDim::_3>& t,
            const vector_t<value_type, EDim::_2>& e) : plane<value_type>(p, n, norm) { set_tangent(t); set_extends(e); }

        //n, norm t, e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n,
            const vector_t<value_type, EDim::_3>& t, normalize_hint,
            const vector_t<value_type, EDim::_2>& e) : plane<value_type>(p, n), _tangent(t), _extends(e) { }

        //n, t, norm e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n,
            const vector_t<value_type, EDim::_3>& t,
            const vector_t<value_type, EDim::_2>& e, normalize_hint) : plane<value_type>(p, n), _extends(e) { set_tangent(t); }

        //norm n, norm t, e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n, normalize_hint,
            const vector_t<value_type, EDim::_3>& t, normalize_hint,
            const vector_t<value_type, EDim::_2>& e) : plane<value_type>(p, n, norm), _tangent(t) { set_extends(e); }

        //norm n, t, norm e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n, normalize_hint,
            const vector_t<value_type, EDim::_3>& t,
            const vector_t<value_type, EDim::_2>& e, normalize_hint) : plane<value_type>(p, n, norm), _extends(e) { set_tangent(t); }

        //n, norm t, norm e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n,
            const vector_t<value_type, EDim::_3>& t, normalize_hint,
            const vector_t<value_type, EDim::_2>& e, normalize_hint) : plane<value_type>(p, n), _tangent(t), _extends(e) { }

        //norm n, norm t, norm e
        rect(const point<value_type, EDim::_3>& p,
            const vector_t<value_type, EDim::_3>& n, normalize_hint,
            const vector_t<value_type, EDim::_3>& t, normalize_hint,
            const vector_t<value_type, EDim::_2>& e, normalize_hint) : plane<value_type>(p, n, norm), _tangent(t), _extends(e) { }

        void set_position(const point<value_type, EDim::_3>& p) { plane<value_type>::set_position(p); }
        void set_normal(const vector_t<value_type, EDim::_3>& n) { plane<value_type>::set_normal(n); }
        void set_normal(const vector_t<value_type, EDim::_3>& n, normalize_hint) { plane<value_type>::set_normal(n, normalize_hint::norm); }
        void set_tangent(const vector_t<value_type, EDim::_2>& t) { _tangent = normalized(_tangent - dot(_tangent, _normal) * _normal); }
        void set_tangent(const vector_t<value_type, EDim::_2>& t, normalize_hint) { _tangent = t; }
        void set_width(value_type w) { _extends.x = max2(w, value_type(0)); }
        void set_height(value_type h) { _extends.y = max2(h, value_type(0)); }
        void set_extends(value_type w, value_type h) { set_width(w); set_height(h); }
        void set_extends(const vector_t<value_type, EDim::_2>& e) { set_width(e.x); set_height(e.y); }
        void set_extends(const vector_t<value_type, EDim::_2>& e, normalize_hint) { _extends = e; }
        constexpr const point<value_type, EDim::_3>& position() const { return plane<value_type>::position(); }
        constexpr point<value_type, EDim::_3>& position() { return plane<value_type>::position(); }
        constexpr const vector_t<value_type, EDim::_3>& normal() const { return plane<value_type>::normal(); }
        constexpr const vector_t<value_type, EDim::_3>& tangent() const { return _tangent; }
        const vector_t<value_type, EDim::_2>& extends() const { return _extends; }
        value_type width() const { return _extends.x; }
        value_type height() const { return _extends.y; }
    private:
        vector_t<value_type, EDim::_3> _tangent;
        vector_t<value_type, EDim::_2> _extends;
    };

    template<typename value_type>
    class cube
    {
    public:
        cube(const point<value_type, EDim::_3>& c, const vector_t<value_type, EDim::_3>& e) :_center(c) { set_extends(e); }
        cube(const point<value_type, EDim::_3>& c, const vector_t<value_type, EDim::_3>& e, normalize_hint) : _center(c), _extends(e) {  }
        void set_width(value_type w) { _extends.x = max2(w, value_type(0)); }
        void set_height(value_type h) { _extends.y = max2(h, value_type(0)); }
        void set_depth(value_type d) { _extends.z = max2(d, value_type(0)); }
        void set_extends(value_type w, value_type h, value_type d) { set_width(w); set_height(h); set_depth(d); }
        void set_extends(const vector_t<value_type, EDim::_3>& e) { set_width(e.x); set_height(e.y); set_depth(e.z); }
        void set_extends(const vector_t<value_type, EDim::_3>& e, normalize_hint) { _extends = e; }
        const point<value_type, EDim::_3>& center() const { return _center; }
        point<value_type, EDim::_3>& center() { return _center; }
        const vector_t<value_type, EDim::_3>& extends() const { return _extends; }
        vector_t<value_type, EDim::_3> axis_x() const { return vector_t<value_type, EDim::_3>::unit_x(); }
        vector_t<value_type, EDim::_3> axis_y() const { return vector_t<value_type, EDim::_3>::unit_y(); }
        vector_t<value_type, EDim::_3> axis_z() const { return vector_t<value_type, EDim::_3>::unit_z(); }
        value_type width() const { return _extends.x; }
        value_type height() const { return _extends.y; }
        value_type depth() const { return _extends.z; }
    private:
        point<value_type, EDim::_3> _center;
        vector_t<value_type, EDim::_3> _extends;
    };

    template<typename value_type>
    constexpr point<value_type, EDim::_3> transform(
        const matrix_t<value_type, EDim::_4, EDim::_4>& l,
        const point<value_type, EDim::_3>& r)
    {
        vector_t<value_type, EDim::_4> rhs4(r, value_type(1));
        return point<value_type, EDim::_3>(
            dot(l.rows[0], rhs4),
            dot(l.rows[1], rhs4),
            dot(l.rows[2], rhs4));
    }


    enum class intersection : unsigned char
    {
        none, same,
        inside, contain,
        intersect, tangent
    };

    template<typename value_type>
    intersection intersect_plane(const ray<value_type, EDim::_3>& ray,
        const point<value_type, EDim::_3>& pos, const vector_t<value_type, EDim::_3>& normal, bool dualface,
        value_type error, value_type& t)
    {
        value_type Dr_dot_N = dot(ray.direction(), normal);

        if (Dr_dot_N < value_type(0) || (Dr_dot_N > value_type(0) && dualface))
        {
            //t = -dot(N, Pp) / dot(Dr, N);
            t = dot(pos - ray.origin(), normal) / Dr_dot_N;
            if (t > error)
            {
                return intersection::intersect;
            }
            else
            {
                return intersection::none;
            }
        }
        return intersection::none;
    }

    template<typename value_type>
    intersection intersect_disk(const ray<value_type, EDim::_3>& ray,
        const point<value_type, EDim::_3>& pos,
        const vector_t<value_type, EDim::_3>& normal,
        scaler_t<value_type> radius, bool dualface,
        value_type error, value_type& t)
    {
        intersection r = intersect_plane(ray, pos, normal, dualface, error, t);
        if (r != intersection::none)
        {
            point<value_type, EDim::_3> intersection = ray.calc_offset(t);
            vector_t<value_type, EDim::_3> offset = (intersection - pos);
            value_type r = dot(offset, offset);
            value_type radius_sqr = radius * radius;
            if (r > radius_sqr)
            {
                intersection::none;
            }
            else if (r < radius_sqr)
            {
                return intersection::inside;
            }
            else
            {
                return intersection::tangent;
            }
        }

        return intersection::none;
    }

    template<typename value_type>
    intersection intersect_rect(const ray<value_type, EDim::_3>& ray,
        const point<value_type, EDim::_3>& pos,
        const vector_t<value_type, EDim::_3>& normal,
        const vector_t<value_type, EDim::_3>& tangent,
        const vector_t<value_type, EDim::_2>& extends, bool dualface,
        value_type error, value_type& t)
    {
        intersection r = intersect_plane(ray, pos, normal, dualface, error, t);
        if (r != intersection::none)
        {
            point<value_type, EDim::_3> intersection = ray.calc_offset(t);
            vector_t<value_type, EDim::_3> offset = (intersection - pos);
            vector3<value_type> projection = dot(offset, tangent) * tangent;
            value_type x = magnitude(projection);
            value_type y = magnitude(offset - projection);
            if (x > extends.x || y > extends.y)
            {
                intersection::none;
            }
            else if (x < extends.x && y < extends.y)
            {
                return intersection::inside;
            }
            else
            {
                return intersection::tangent;
            }
        }

        return intersection::none;
    }

    template<typename value_type>
    intersection intersect(const ray<value_type, EDim::_3>& ray,
        const plane<value_type>& plane, dual_face_hint,
        value_type error, value_type& t)
    {
        return intersect_plane(ray, plane.position(), plane.normal(), true, error, t);
    }

    template<typename value_type>
    intersection intersect(const ray<value_type, EDim::_3>& ray,
        const plane<value_type>& plane, value_type error, value_type& t)
    {
        return intersect_plane(ray, plane.position(), plane.normal(), false, error, t);
    }


    template<typename value_type>
    intersection intersect_sphere(const ray<value_type, EDim::_3>& ray,
        const point<value_type, EDim::_3>& center, value_type radius_sqr,
        value_type error, value_type& t0, value_type& t1)
    {
        //dot(dir, dir)*t^2 + 2*dot(dir, Or-Os)*t + dot(Or-Os, Or-Os) - r^2 = 0
        //a = dot(Dr,Dr)
        //b = 2*dot(Dr, Or-Os)
        //c = dot(Or-Os,Or-Os) - r^2
        //t = (-b +- sqrt(b^2 - 4ac)) / 2a
        vector_t<value_type, EDim::_3> Or_Os = ray.origin() - center;
        const vector_t<value_type, EDim::_3> Dr = ray.direction();
        value_type a = dot(Dr, Dr);
        value_type b = 2 * dot(Dr, Or_Os);
        value_type c = dot(Or_Os, Or_Os) - radius_sqr;
        value_type det = b * b - 4 * a*c;
        if (det > value_type(0))
        {
            det = sqrt(det);
            value_type inv2a = value_type(0.5) / a;
            t0 = (-b - det) * inv2a;
            t1 = (-b + det) * inv2a;
            if (t1 < error)
            {
                return intersection::none;
            }

            if (t1 < t0) { std::swap(t0, t1); }
            return (t0 < error)
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

            return t0 > error ? intersection::tangent : intersection::none;
        }
    }

    template<typename value_type>
    intersection intersect(const ray<value_type, EDim::_3>& ray, const sphere<value_type>& sphere,
        value_type error, value_type& t0, value_type& t1)
    {
        return intersect_sphere(ray, sphere.center(), sphere.radius_sqr(), error, t0, t1);
    }

    template<typename value_type>
    intersection intersect_cube(const ray<value_type, EDim::_3>& ray,
        const point<value_type, EDim::_3>& position,
        const vector_t<value_type, EDim::_3>& axis_x,
        const vector_t<value_type, EDim::_3>& axis_y,
        const vector_t<value_type, EDim::_3>& axis_z,
        value_type extend_x, value_type extend_y, value_type extend_z,
        value_type error,
        value_type& t0, value_type& t1,
        vector_t<value_type, EDim::_3>& n0,
        vector_t<value_type, EDim::_3>& n1)
    {
        int parallelMask = 0;
        bool found = false;

        vector_t<value_type, EDim::_3> Dr_dot_Axis;
        vector_t<value_type, EDim::_3> OC_dot_Axis;
        vector_t<value_type, EDim::_3> P_Or = position - ray.origin();
        vector_t<value_type, EDim::_3> axis[3] = { axis_x, axis_y, axis_z };
        value_type extend[3] = { extend_x, extend_y, extend_z };

        for (int i = 0; i < 3; ++i)
        {
            Dr_dot_Axis.v[i] = dot(ray.direction(), axis[i]);
            OC_dot_Axis.v[i] = dot(P_Or, axis[i]);

            if (fabs(Dr_dot_Axis[i]) < EPSILON<value_type>)
            {
                parallelMask |= 1 << i;
            }
            else
            {
                value_type sign = Dr_dot_Axis[i] > value_type(0) ? 1 : -1;
                value_type extend_i = sign * extend[i];
                value_type invDA = value_type(1) / Dr_dot_Axis[i];

                if (!found)
                {
                    t0 = (OC_dot_Axis[i] - extend_i) * invDA;
                    t1 = (OC_dot_Axis[i] + extend_i) * invDA;
                    n0 = -axis[i] * sign;
                    n1 = axis[i] * sign;
                    found = true;
                }
                else
                {
                    value_type s = (OC_dot_Axis[i] - extend_i) * invDA;
                    if (s > t0)
                    {
                        t0 = s;
                        n0 = -axis[i] * sign;
                    }

                    s = (OC_dot_Axis[i] + extend_i) * invDA;
                    if (s < t1)
                    {
                        t1 = s;
                        n1 = axis[i] * sign;
                    }

                    if (t0 > t1)
                    {
                        return intersection::none;
                    }
                }
            }
        }

        if (parallelMask)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (parallelMask & (1 << i))
                {
                    if (fabs(OC_dot_Axis[i] - t0 * Dr_dot_Axis[i]) > extend[i] ||
                        fabs(OC_dot_Axis[i] - t1 * Dr_dot_Axis[i]) > extend[i])
                    {
                        return intersection::none;
                    }
                }
            }
        }

        if (t1 < error)
        {
            return intersection::none;
        }
        else if (t0 < error)
        {
            return intersection::inside;
        }
        else
        {
            return intersection::intersect;
        }
    }

    template<typename value_type>
    intersection intersect(const ray<value_type, EDim::_3>& ray, const cube<value_type>& _cube,
        value_type error, value_type& t0, value_type& t1, vector_t<value_type, EDim::_3>& n0, vector_t<value_type, EDim::_3>& n1)
    {
        return intersect_cube(ray, _cube.center(),
            _cube.axis_x(), _cube.axis_y(), _cube.axis_z(),
            _cube.width(), _cube.height(), _cube.depth(),
            error, t0, t1, n0, n1);
    }

    template<typename value_type>
    intersection intersect_triangle(const ray<value_type, EDim::_3>& ray,
        const point<value_type, EDim::_3>& v0, const point<value_type, EDim::_3>& v1, const point<value_type, EDim::_3>& v2,
        value_type error, value_type& t, value_type&u, value_type& v, vector_t<value_type, EDim::_3>& normal)
    {
        //moller-trumbore
        vector_t<value_type, EDim::_3> v1v0 = v1 - v0;
        vector_t<value_type, EDim::_3> v2v0 = v2 - v0;

        vector_t<value_type, EDim::_3> pv = cross(ray.direction(), v2v0);
        float det = dot(v1v0, pv);
        if (det > value_type(0))
        {
            float invDet = F(1) / det;

            vector_t<value_type, EDim::_3> tv = ray.origin() - v0;
            u = dot(tv, pv) * invDet;
            if (u < value_type(0) || u > value_type(1))
            {
                return intersection::none;
            }

            vector_t<value_type, EDim::_3> qv = cross(tv, v1v0);
            v = dot(ray.direction(), qv) * invDet;
            if (v < value_type(0) || u + v > value_type(1))
            {
                return intersection::none;
            }

            t = dot(v2v0, qv) * invDet;
            if (t < error)
            {
                return intersection::none;
            }

            normal = cross(v2v0, v1v0).normalized();
            return intersection::intersect;
        }
        else
        {
            return intersection::none;
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
