#pragma once
#include "Vector.h"
#include "Rotation.h"

namespace math
{
    template<typename value_type, EDim row_dim, EDim col_dim>
    struct matrix_t;

    template<typename value_type, EDim row_dim, EDim col_dim>
    struct translation_matrix_t;

    template<typename value_type, EDim row_dim, EDim col_dim>
    struct rotation_matrix_t;

    template<typename value_type, EDim row_dim, EDim col_dim>
    struct scale_matrix_t;


    template<typename value_type>
    struct matrix_t<value_type, EDim::_2, EDim::_2>
    {
        using index_type = size_t;
        static constexpr EDim row_dim = EDim::_2;
        static constexpr EDim col_dim = EDim::_2;
        static constexpr size_t cell_count = row_dim * col_dim;

        union
        {
            char mem[cell_count * sizeof(value_type)];
            value_type m[cell_count];
            value_type cells[row_dim][col_dim];
            vector_t<value_type, EDim::_2> rows[row_dim];
            struct
            {
                value_type
                    _00, _01,
                    _10, _11;
            };
        };

        constexpr matrix_t() : cells{
             { value_type(1), value_type(0) }
            ,{ value_type(0), value_type(1) } } {   }

        constexpr matrix_t(
            value_type _m00, value_type _m01,
            value_type _m10, value_type _m11) : cells{
             { _m00, _m01 }
            ,{ _m10, _m11 } } {  }

        constexpr matrix_t(
            const vector_t<value_type, EDim::_2>& row0,
            const vector_t<value_type, EDim::_2>& row1) : cells{
             { row0.x, row0.y }
            ,{ row1.x, row1.y } } { }

        constexpr matrix_t(const matrix_t& m22) : cells{
             { m22.cells[0][0], m22.cells[0][1] }
            ,{ m22.cells[1][0], m22.cells[1][1] } } {  }

        constexpr matrix_t& operator=(const matrix_t& r)
        {
            for (size_t ri = 0; ri < row_dim; ri++)
            {
                for (size_t ci = 0; ci < col_dim; ci++)
                {
                    c[ri][ci] = r.cells[ri][ci];
                }
            }
            return *this;
        }

        inline value_type& operator[] (index_type idx) { return m[idx]; }
        inline const value_type& operator[] (index_type idx) const { return m[idx]; }
        constexpr vector_t<value_type, EDim::_2> column(size_t idx) const { return vector_t<value_type, EDim::_2>(cells[0][idx], cells[1][idx]); }
        constexpr void set_column(size_t idx, vector_t<value_type, EDim::_2> _v)
        {
            cells[0][idx] = _v.x;
            cells[1][idx] = _v.y;
        }

        // static utilities
        static constexpr matrix_t identity()
        {
            return matrix_t();
        }

        static constexpr matrix_t flip_x()
        {
            return matrix_t(
                -value_type(1), value_type(0),
                value_type(0), value_type(1));
        }

        static constexpr matrix_t flip_y()
        {
            return matrix_t(
                value_type(1), value_type(0),
                value_type(0), -value_type(1));
        }

        static inline matrix_t rotation(const radian<value_type>& r);
        static constexpr matrix_t scale(value_type sx, value_type sy);
        static constexpr matrix_t scale(value_type s);
        static constexpr matrix_t scale(const vector_t<value_type, EDim::_2>& s);
    };

    template<typename value_type>
    struct matrix_t<value_type, EDim::_2, EDim::_3>
    {
        using index_type = size_t;
        static constexpr EDim row_dim = EDim::_2;
        static constexpr EDim col_dim = EDim::_3;
        static constexpr size_t cell_count = row_dim * col_dim;

        union
        {
            char mem[cell_count * sizeof(value_type)];
            value_type  m[cell_count];
            value_type  cells[row_dim][col_dim];
            vector_t<value_type, EDim::_3> rows[row_dim];
            struct
            {
                value_type
                    _00, _01, _02,
                    _10, _11, _12;
            };
        };

        constexpr matrix_t() : cells{
             { value_type(1), value_type(0), value_type(0) }
            ,{ value_type(0), value_type(1), value_type(0) } } { }

        constexpr matrix_t(
            value_type _m00, value_type _m01, value_type _m02,
            value_type _m10, value_type _m11, value_type _m12) : cells{
             { _m00, _m01, _m02 }
            ,{ _m10, _m11, _m12 } } { }

        constexpr matrix_t(
            const vector_t<value_type, EDim::_3>& row0,
            const vector_t<value_type, EDim::_3>& row1) : cells{
             { row0.x, row0.y,row0.z }
            ,{ row1.x, row1.y,row1.z } } { }

        constexpr matrix_t(const matrix_t& mat) : cells{
             { mat.cells[0][0], mat.cells[0][1], mat.cells[0][2] }
            ,{ mat.cells[1][0], mat.cells[1][1], mat.cells[1][2] } } { }

        constexpr matrix_t(const matrix_t<value_type, EDim::_2, EDim::_2>& mat_r,
            const vector_t<value_type, EDim::_2>& trans) : cells{
             { mat_r.cells[0][0], mat_r.cells[0][1], trans.x }
            ,{ mat_r.cells[1][0], mat_r.cells[1][1], trans.y } } { }

        constexpr matrix_t& operator=(const matrix_t& r)
        {
            for (size_t ri = 0; ri < row_dim; ri++)
            {
                for (size_t ci = 0; ci < col_dim; ci++)
                {
                    c[ri][ci] = r.cells[ri][ci];
                }
            }
            return *this;
        }

        inline value_type& operator[] (index_type idx) { return v[idx]; }
        inline const value_type& operator[] (index_type idx) const { return v[idx]; }
        constexpr vector_t<value_type, EDim::_2> row2d(size_t idx) const { return vector_t<value_type, EDim::_2>(cells[idx][0], cells[idx][1]); }
        constexpr vector_t<value_type, EDim::_2> column(size_t idx) const { return vector_t<value_type, EDim::_2>(cells[0][idx], cells[1][idx]); }
        constexpr vector_t<value_type, EDim::_3> column3(size_t idx) const { return vector_t<value_type, EDim::_3>(cells[0][idx], cells[1][idx], idx == 2 ? value_type(1) : value_type(0)); }
        constexpr void set_column(size_t idx, vector_t<value_type, EDim::_2> _v)
        {
            cells[0][idx] = _v.x;
            cells[1][idx] = _v.y;
        }

        //static utilities
        static constexpr matrix_t identity()
        {
            return matrix_t();
        }

        static constexpr matrix_t flip_x()
        {
            return matrix_t(
                -value_type(1), value_type(0), value_type(0),
                value_type(0), value_type(1), value_type(0));
        }

        static constexpr matrix_t flip_y()
        {
            return matrix_t(
                value_type(1), value_type(0), value_type(0),
                value_type(0), -value_type(1), value_type(0));
        }

        static inline matrix_t trs_inversed(const vector_t<value_type, EDim::_2>& t, const radian<value_type>& r, const vector_t<value_type, EDim::_2>& s)
        {
            /*
            sx,0,0 * +cos,sin,0 * 1,0,-tx
            0,sy,0   -sin,cos,0   0,1,-ty
            =
            sx,0,0 * +cos,sin,-cos*tx-sin*ty
            0,sy,0  -sin,cos,+sin*tx-cos*ty
            =
            +sx*cos, sx*sin, -sx*(cos*tx+sin*ty)
            -sy*sin, sy*cos, +sy*(sin*tx-cos*ty)
            */
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                +s.x * cosr, s.x * sinr, -s.x * (cosr * t.x - sinr * t.y),
                -s.y * sinr, s.y * cosr, +s.y * (sinr * t.x - cosr * t.y));
        }

        static inline matrix_t trs(const vector_t<value_type, EDim::_2>& t, const radian<value_type>& r, const vector_t<value_type, EDim::_2>& s)
        {
            /*
            1,0,tx * cos,-sin,0 * sx,0,0
            0,1,ty   sin, cos,0   0,sy,0
            =
            cos,-sin,tx * sx,0,0
            sin,+cos,ty   0,sy,0
            =
            cos*sx, -sin*sy, tx
            sin*sx, +cos*sy, ty
            */
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                cosr * s.x, -sinr * s.y, t.x,
                sinr * s.x, +cosr * s.y, t.y);
        }

        static inline matrix_t trsp(const vector_t<value_type, EDim::_2>& t, const radian<value_type>& r, const vector_t<value_type, EDim::_2>& s, const vector_t<value_type, EDim::_2>& p)
        {
            /*
            1,0,tx * cos,-sin,0 * sx,0,0 * 1,0,-px
            0,1,ty   sin, cos,0   0,sy,0   0,1,-py
            =
            cos,-sin,tx * sx,0,-sx*px
            sin,+cos,ty   0,sy,-sy*py
            =
            cos*sx, -sin*sy, -cos*sx*px+sin*sy*px + tx
            sin*sx, +cos*sy, -sin*sx*px-cos*sy*py + ty
            */
            value_type cosr = cos(r);
            value_type sinr = sin(r);
            value_type spx = -s.x * p.x;
            value_type spy = -s.y * p.y;

            return matrix_t(
                cosr * s.x, -sinr * s.y, cosr * spx - sinr * spy + t.x,
                sinr * s.x, +cosr * s.y, sinr * spx + cosr * spy + t.y);
        }

        static inline matrix_t trps(const vector_t<value_type, EDim::_2>& t, const radian<value_type>& r, const vector_t<value_type, EDim::_2>& p, const vector_t<value_type, EDim::_2>& s)
        {
            /*
            1,0,tx * cos,-sin,0 * 1,0,-px * sx,0,0
            0,1,ty   sin, cos,0   0,1,-px   0,sy,0
            =
            cos,-sin,ty * sx,0,-px
            sin, cos,ty   0,sy,-px
            =
            cos*sx, -sin*sy, -cos*px+sin*px + tx
            sin*sx, +cos*sy, -sin*px-cos*py + ty
            */
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                cosr * s.x, -sinr * s.y, -cosr * p.x + sinr * p.y + t.x,
                sinr * s.x, +cosr * s.y, -sinr * p.x - cosr * p.y + t.y);
        }

        static constexpr matrix_t translation(value_type tx, value_type ty);
        static constexpr matrix_t translation(const vector_t < value_type, EDim::_2>& t);
        static inline matrix_t rotation(const radian<value_type>& r);
        static constexpr matrix_t scale(value_type sx, value_type sy);
        static constexpr matrix_t scale(value_type s);
        static constexpr matrix_t scale(const vector_t<value_type, EDim::_2>& s);
    };

    template<typename value_type>
    struct matrix_t<value_type, EDim::_3, EDim::_3>
    {
        using index_type = size_t;
        static constexpr EDim row_dim = EDim::_3;
        static constexpr EDim col_dim = EDim::_3;
        static constexpr size_t  cell_count = row_dim * col_dim;

        union
        {
            char mem[cell_count * sizeof(value_type)];
            value_type m[cell_count];
            value_type cells[row_dim][col_dim];
            vector_t<value_type, EDim::_3> rows[row_dim];
            struct
            {
                value_type
                    _00, _01, _02,
                    _10, _11, _12,
                    _20, _21, _22;
            };
        };

        constexpr matrix_t() : cells{
             { value_type(1), value_type(0), value_type(0) }
            ,{ value_type(0), value_type(1), value_type(0) }
            ,{ value_type(0), value_type(0), value_type(1) } } { }

        constexpr matrix_t(
            value_type _m00, value_type _m01, value_type _m02,
            value_type _m10, value_type _m11, value_type _m12,
            value_type _m20, value_type _m21, value_type _m22) : cells{
             { _m00, _m01, _m02 }
            ,{ _m10, _m11, _m12 }
            ,{ _m20, _m21, _m22 } } { }

        constexpr matrix_t(
            const vector_t<value_type, EDim::_3>& row0,
            const vector_t<value_type, EDim::_3>& row1,
            const vector_t<value_type, EDim::_3>& row2) : cells{
             { row0.x, row0.y,row0.z }
            ,{ row1.x, row1.y,row1.z }
            ,{ row2.x, row2.y,row2.z } } { }

        constexpr matrix_t(const matrix_t& mat) : cells{
             { mat.cells[0][0], mat.cells[0][1], mat.cells[0][2] }
            ,{ mat.cells[1][0], mat.cells[1][1], mat.cells[1][2] }
            ,{ mat.cells[2][0], mat.cells[2][1], mat.cells[2][2] } } { }

        constexpr matrix_t(const matrix_t<value_type, EDim::_2, EDim::_2>& mat_r,
            const vector_t<value_type, EDim::_2>& trans) : cells{
             { mat_r.cells[0][0], mat_r.cells[0][1], trans.x }
            ,{ mat_r.cells[1][0], mat_r.cells[1][1], trans.y }
            ,{ value_type(0), value_type(0), value_type(1) } } { }

        constexpr matrix_t& operator=(const matrix_t& r)
        {
            for (size_t ri = 0; ri < row_dim; ri++)
            {
                for (size_t ci = 0; ci < col_dim; ci++)
                {
                    c[ri][ci] = r.cells[ri][ci];
                }
            }
            return *this;
        }

        inline value_type& operator[] (index_type idx) { return v[idx]; }
        inline const value_type& operator[] (index_type idx) const { return v[idx]; }
        constexpr vector_t<value_type, EDim::_2> column2(size_t idx) const { return vector_t<value_type, EDim::_2>(cells[0][idx], cells[1][idx]); }
        constexpr vector_t<value_type, EDim::_3> column(size_t idx) const { return vector_t<value_type, EDim::_3>(cells[0][idx], cells[1][idx], cells[2][idx]); }
        constexpr void set_column(size_t index, vector_t<value_type, EDim::_3> v)
        {
            cells[0][index] = v.x;
            cells[1][index] = v.y;
            cells[2][index] = v.z;
        }

        // static utilities
        static constexpr matrix_t identity()
        {
            return matrix_t();
        }

        static constexpr matrix_t flip_x()
        {
            return matrix_t(
                -value_type(1), value_type(0), value_type(0),
                value_type(0), value_type(1), value_type(0),
                value_type(0), value_type(0), value_type(1));
        }

        static constexpr matrix_t flip_y()
        {
            return matrix_t(
                value_type(1), value_type(0), value_type(0),
                value_type(0), -value_type(1), value_type(0),
                value_type(0), value_type(0), value_type(1));
        }

        static constexpr matrix_t flip_z()
        {
            return matrix_t(
                value_type(1), value_type(0), value_type(0),
                value_type(0), value_type(1), value_type(0),
                value_type(0), value_type(0), -value_type(1));
        }

        static inline matrix_t rotate_x(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                value_type(1), value_type(0), value_type(0),
                value_type(0), cosr, -sinr,
                value_type(0), sinr, cosr);
        }

        static inline matrix_t rotate_y(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                cosr, value_type(0), -sinr,
                value_type(0), value_type(1), value_type(0),
                sinr, value_type(0), cosr);
        }

        static inline matrix_t rotate_z(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                cosr, -sinr, value_type(0),
                sinr, cosr, value_type(0),
                value_type(0), value_type(0), value_type(1));
        }

        static inline matrix_t trs_inversed(const vector_t<value_type, EDim::_2>& t, const radian<value_type>& r, const vector_t<value_type, EDim::_2>& s)
        {
            /*
            sx,0,0 * +cos,sin,0 * 1,0,-tx
            0,sy,0   -sin,cos,0   0,1,-ty
            =
            sx,0,0 * +cos,sin,-cos*tx-sin*ty
            0,sy,0  -sin,cos,+sin*tx-cos*ty
            =
            +sx*cos, sx*sin, -sx*(cos*tx+sin*ty)
            -sy*sin, sy*cos, +sy*(sin*tx-cos*ty)
            */
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                +s.x * cosr, s.x * sinr, -s.x * (cosr * t.x - sinr * t.y),
                -s.y * sinr, s.y * cosr, +s.y * (sinr * t.x - cosr * t.y),
                value_type(0), value_type(0), value_type(1));
        }

        static inline matrix_t trs(const vector_t<value_type, EDim::_2>& t, const radian<value_type>& r, const vector_t<value_type, EDim::_2>& s)
        {
            /*
            1,0,tx * cos,-sin,0 * sx,0,0
            0,1,ty   sin, cos,0   0,sy,0
            =
            cos,-sin,tx * sx,0,0
            sin,+cos,ty   0,sy,0
            =
            cos*sx, -sin*sy, tx
            sin*sx, +cos*sy, ty
            */
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                cosr * s.x, -sinr * s.y, t.x,
                sinr * s.x, +cosr * s.y, t.y,
                value_type(0), value_type(0), value_type(1));
        }

        static constexpr matrix_t translation(value_type tx, value_type ty);
        static constexpr matrix_t translation(const vector_t < value_type, EDim::_2>& t);
        static inline matrix_t rotation(const radian<value_type>& r);
        static constexpr matrix_t rotation(const quaternion<value_type>& q);
        static constexpr matrix_t scale(value_type sx, value_type sy, value_type sz);
        static constexpr matrix_t scale(value_type s);
        static constexpr matrix_t scale(const vector_t<value_type, EDim::_3>& s);
        static constexpr matrix_t scale2d(value_type sx, value_type sy);
        static constexpr matrix_t scale2d(value_type s);
        static constexpr matrix_t scale2d(const vector_t<value_type, EDim::_2>& s);
    };

    template<typename value_type>
    struct matrix_t<value_type, EDim::_4, EDim::_4>
    {
        using index_type = size_t;
        static constexpr EDim row_dim = EDim::_4;
        static constexpr EDim col_dim = EDim::_4;
        static constexpr size_t cell_count = row_dim * col_dim;

        union
        {
            char mem[cell_count * sizeof(value_type)];
            value_type  m[cell_count];
            value_type  cells[row_dim][col_dim];
            vector_t<value_type, EDim::_4> rows[row_dim];
            struct
            {
                value_type
                    _00, _01, _02, _03,
                    _10, _11, _12, _13,
                    _20, _21, _22, _23,
                    _30, _31, _32, _33;
            };
        };

        constexpr matrix_t() : cells{
             { value_type(1), value_type(0), value_type(0), value_type(0) }
            ,{ value_type(0), value_type(1), value_type(0), value_type(0) }
            ,{ value_type(0), value_type(0), value_type(1), value_type(0) }
            ,{ value_type(0), value_type(0), value_type(0), value_type(1) } } { }

        constexpr matrix_t(
            value_type _m00, value_type _m01, value_type _m02, value_type _m03,
            value_type _m10, value_type _m11, value_type _m12, value_type _m13,
            value_type _m20, value_type _m21, value_type _m22, value_type _m23,
            value_type _m30, value_type _m31, value_type _m32, value_type _m33) : cells{
             { _m00, _m01, _m02, _m03 }
            ,{ _m10, _m11, _m12, _m13 }
            ,{ _m20, _m21, _m22, _m23 }
            ,{ _m30, _m31, _m32, _m33 } } { }

        constexpr matrix_t(
            const vector_t<value_type, EDim::_4>& row0,
            const vector_t<value_type, EDim::_4>& row1,
            const vector_t<value_type, EDim::_4>& row2,
            const vector_t<value_type, EDim::_4>& row3) : cells{
             { row0.x, row0.y, row0.z, row0.w }
            ,{ row1.x, row1.y, row1.z, row1.w }
            ,{ row2.x, row2.y, row2.z, row2.w }
            ,{ row3.x, row3.y, row3.z, row3.w } } { }

        constexpr matrix_t(const matrix_t& m44) : cells{
             { m44.cells[0][0], m44.cells[0][1], m44.cells[0][2], m44.cells[0][3] }
            ,{ m44.cells[1][0], m44.cells[1][1], m44.cells[1][2], m44.cells[1][3] }
            ,{ m44.cells[2][0], m44.cells[2][1], m44.cells[2][2], m44.cells[2][3] }
            ,{ m44.cells[3][0], m44.cells[3][1], m44.cells[3][2], m44.cells[3][3] } } { }

        constexpr matrix_t(const matrix_t<value_type, EDim::_3, EDim::_3>& mat_r, const vector_t<value_type, EDim::_3>& trans) : cells{
             { mat_r.cells[0][0], mat_r.cells[0][1], mat_r.cells[0][2], trans.x }
            ,{ mat_r.cells[1][0], mat_r.cells[1][1], mat_r.cells[1][2], trans.y }
            ,{ mat_r.cells[2][0], mat_r.cells[2][1], mat_r.cells[2][2], trans.z }
            ,{ value_type(0), value_type(0), value_type(0), value_type(1) } } { }

        constexpr matrix_t& operator=(const matrix_t& r)
        {
            for (size_t ri = 0; ri < row_dim; ri++)
            {
                for (size_t ci = 0; ci < col_dim; ci++)
                {
                    cells[ri][ci] = r.cells[ri][ci];
                }
            }
            return *this;
        }

        inline value_type& operator[] (index_type index) { return v[index]; }
        inline const value_type& operator[] (index_type index) const { return v[index]; }

        constexpr vector_t<value_type, EDim::_3> column3(size_t index) const
        {
            return vector_t<value_type, EDim::_3>(
                cells[0][index],
                cells[1][index],
                cells[2][index]);
        }

        constexpr vector_t<value_type, EDim::_4> column(size_t index) const
        {
            return vector_t<value_type, EDim::_4>(
                cells[0][index],
                cells[1][index],
                cells[2][index],
                cells[3][index]);
        }

        constexpr void set_column(size_t index, vector_t<value_type, EDim::_4> v)
        {
            cells[0][index] = v.x;
            cells[1][index] = v.y;
            cells[2][index] = v.z;
            cells[3][index] = v.w;
        }

        constexpr void set_column3(size_t index, vector_t<value_type, EDim::_3> v)
        {
            cells[0][index] = v.x;
            cells[1][index] = v.y;
            cells[2][index] = v.z;
        }

        // static utilities
        static constexpr matrix_t identity()
        {
            return matrix_t();
        }

        static constexpr matrix_t flip_x()
        {
            return matrix_t(
                -value_type(1), value_type(0), value_type(0), value_type(0),
                value_type(0), value_type(1), value_type(0), value_type(0),
                value_type(0), value_type(0), value_type(1), value_type(0),
                value_type(0), value_type(0), value_type(0), value_type(1));
        }

        static constexpr matrix_t flip_y()
        {
            return matrix_t(
                value_type(1), value_type(0), value_type(0), value_type(0),
                value_type(0), -value_type(1), value_type(0), value_type(0),
                value_type(0), value_type(0), value_type(1), value_type(0),
                value_type(0), value_type(0), value_type(0), value_type(1));
        }

        static constexpr matrix_t flip_z()
        {
            return matrix_t(
                value_type(1), value_type(0), value_type(0), value_type(0),
                value_type(0), value_type(1), value_type(0), value_type(0),
                value_type(0), value_type(0), -value_type(1), value_type(0),
                value_type(0), value_type(0), value_type(0), value_type(1));
        }

        static inline matrix_t rotate_x(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                value_type(1), value_type(0), value_type(0), value_type(0),
                value_type(0), cosr, -sinr, value_type(0),
                value_type(0), sinr, cosr, value_type(0),
                value_type(0), value_type(0), value_type(0), value_type(1)
            );
        }

        static inline matrix_t rotate_y(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                cosr, value_type(0), -sinr, value_type(0),
                value_type(0), value_type(1), value_type(0), value_type(0),
                sinr, value_type(0), cosr, value_type(0),
                value_type(0), value_type(0), value_type(0), value_type(1)
            );
        }

        static inline matrix_t rotate_z(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            return matrix_t(
                cosr, -sinr, value_type(0), value_type(0),
                sinr, cosr, value_type(0), value_type(0),
                value_type(0), value_type(0), value_type(1), value_type(0),
                value_type(0), value_type(0), value_type(0), value_type(1)
            );
        }

        static inline matrix_t tr(const vector_t<value_type, EDim::_3>& t, const quaternion<value_type>& r)
        {
            translation_matrix_t<value_type, EDim::_4, EDim::_4> matt(t);
            rotation_matrix_t<value_type, EDim::_4, EDim::_4> matr(r);
            return matt * matr;
        }

        static inline matrix_t trs(const vector_t<value_type, EDim::_3>& t,
            const quaternion<value_type>& r,
            const vector_t<value_type, EDim::_3>& s)
        {
            translation_matrix_t<value_type, EDim::_4, EDim::_4> matt(t);
            rotation_matrix_t<value_type, EDim::_4, EDim::_4> matr(r);
            scale_matrix_t<value_type, EDim::_4, EDim::_4> mats(s);
            return matt * matr * mats;
        }

        static inline matrix_t look_at(
            const vector_t<value_type, EDim::_3>& eye,
            const vector_t<value_type, EDim::_3>& look,
            const vector_t<value_type, EDim::_3>& up)
        {
            vector_t<value_type, EDim::_3> forward = normalized(look - eye);
            vector_t<value_type, EDim::_3> real_up = normalized(up - forward * dot(forward, up));
            vector_t<value_type, EDim::_3> right = cross(real_up, forward);
            return matrix_t(
                vector_t<value_type, EDim::_4>(right, -dot(eye, right)),
                vector_t<value_type, EDim::_4>(real_up, -dot(eye, real_up)),
                vector_t<value_type, EDim::_4>(forward, -dot(eye, forward)),
                vector_t<value_type, EDim::_4>(value_type(0), value_type(0), value_type(0), value_type(1)));
        }

        static inline matrix_t perspective_lh(const radian<value_type>& fov, value_type aspect, value_type znear, value_type zfar)
        {
            value_type near_top = tan(fov * value_type(0.5));
            value_type near_right = near_top * aspect;
            value_type znear2 = value_type(2) * znear;
            value_type z_range_inv = zfar / (zfar - znear);
            return matrix_t(
                znear2 / near_right, value_type(0), value_type(0), value_type(0),
                value_type(0), znear2 / near_top, value_type(0), value_type(0),
                value_type(0), value_type(0), z_range_inv, -znear * z_range_inv,
                value_type(0), value_type(0), value_type(1), value_type(0));
        }

        static inline matrix_t perspective_lh(const radian<value_type>& fov, value_type aspect, const vector2<value_type>& zpair) { perspective_lh(fov, aspect, zpair.x, zpair.y); }

        static inline matrix_t center_ortho_lh(value_type width, value_type height, value_type znear, value_type zfar)
        {
            value_type z_range_inv = value_type(1) / (zfar - znear);
            return matrix_t(
                value_type(2) / width, value_type(0), value_type(0), value_type(0),
                value_type(0), -value_type(2) / height, value_type(0), value_type(0),
                value_type(0), value_type(0), z_range_inv, -znear * z_range_inv,
                value_type(0), value_type(0), value_type(0), value_type(1)
            );
        }

        static inline matrix_t center_ortho_lh(const vector2<value_type>& size, const vector2<value_type>& zpair) { center_ortho_lh(size.x, size.y, zpair.x, zpair.y); }

        static inline matrix_t ortho2d_lh(value_type width, value_type height, value_type znear, value_type zfar)
        {
            value_type z_range_inv = value_type(1) / (zfar - znear);
            return matrix_t(
                value_type(2) / width, value_type(0), value_type(0), -value_type(1),
                value_type(0), -value_type(2) / height, value_type(0), value_type(1),
                value_type(0), value_type(0), z_range_inv, -znear * z_range_inv,
                value_type(0), value_type(0), value_type(0), value_type(1)
            );
        }

        static inline matrix_t ortho2d_lh(const vector2<value_type>& size, const vector2<value_type>& zpair) { ortho2d_lh(size.x, size.y, zpair.x, zpair.y); }

        static constexpr matrix_t translation(value_type tx, value_type ty, value_type tz);
        static constexpr matrix_t translation(const vector_t<value_type, EDim::_3>& t);
        static constexpr matrix_t rotation(const quaternion<value_type>& q);
        static constexpr matrix_t scale(value_type sx, value_type sy, value_type sz);
        static constexpr matrix_t scale(value_type s);
        static constexpr matrix_t scale(const vector_t<value_type, EDim::_3>& s);
    };

    template<typename value_type>
    struct translation_matrix_t<value_type, EDim::_2, EDim::_3>
        : public matrix_t<value_type, EDim::_2, EDim::_3>
    {
        constexpr translation_matrix_t(value_type tx, value_type ty)
            : matrix_t(
                value_type(1), value_type(0), tx,
                value_type(0), value_type(1), ty)
        { }

        constexpr translation_matrix_t(const vector_t<value_type, EDim::_2>& translation)
            : translation_matrix_t(translation.x, translation.y)
        { }
    };

    template<typename value_type>
    struct translation_matrix_t<value_type, EDim::_3, EDim::_3>
        : public matrix_t<value_type, EDim::_3, EDim::_3>
    {
        constexpr translation_matrix_t(value_type tx, value_type ty)
            : matrix_t(
                value_type(1), value_type(0), tx,
                value_type(0), value_type(1), ty)
        { }

        constexpr translation_matrix_t(const vector_t<value_type, EDim::_2>& translation)
            : translation_matrix_t(translation.x, translation.y)
        { }
    };

    template<typename value_type>
    struct translation_matrix_t<value_type, EDim::_4, EDim::_4>
        : public matrix_t<value_type, EDim::_4, EDim::_4>
    {
        constexpr translation_matrix_t(value_type tx, value_type ty, value_type tz)
            : matrix_t(
                value_type(1), value_type(0), value_type(0), tx,
                value_type(0), value_type(1), value_type(0), ty,
                value_type(0), value_type(0), value_type(1), tz,
                value_type(0), value_type(0), value_type(0), value_type(1)
            )
        { }

        constexpr translation_matrix_t(const vector_t<value_type, EDim::_3>& translation)
            : translation_matrix_t(translation.x, translation.y, translation.z)
        { }
    };


    template<typename value_type>
    struct rotation_matrix_t<value_type, EDim::_2, EDim::_2>
        : public matrix_t<value_type, EDim::_2, EDim::_2>
    {
        inline rotation_matrix_t(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            cells[0][0] = cosr;     cells[0][1] = -sinr;
            cells[1][0] = sinr;     cells[1][1] = cosr;
        }
    };

    template<typename value_type>
    struct rotation_matrix_t<value_type, EDim::_2, EDim::_3>
        : public matrix_t<value_type, EDim::_2, EDim::_3>
    {
        inline rotation_matrix_t(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            cells[0][0] = cosr;     cells[0][1] = -sinr;
            cells[1][0] = sinr;     cells[1][1] = cosr;
        }
    };

    template<typename value_type>
    struct rotation_matrix_t<value_type, EDim::_3, EDim::_3>
        : public matrix_t<value_type, EDim::_3, EDim::_3>
    {
        inline rotation_matrix_t(const radian<value_type>& r)
        {
            value_type cosr = cos(r);
            value_type sinr = sin(r);

            cells[0][0] = cosr;     cells[0][1] = -sinr;
            cells[1][0] = sinr;     cells[1][1] = cosr;
        }

        constexpr rotation_matrix_t(const quaternion<value_type>& q)
        {
            vector_t<value_type, EDim::_3> vsqr2 = q.v * q.v * value_type(2);
            value_type xy2 = q.v.x * q.v.y * value_type(2);
            value_type yz2 = q.v.y * q.v.z * value_type(2);
            value_type zx2 = q.v.z * q.v.x * value_type(2);
            value_type wx2 = q.w * q.v.x * value_type(2);
            value_type wy2 = q.w * q.v.y * value_type(2);
            value_type wz2 = q.w * q.v.z * value_type(2);

            cells[0][0] = value_type(1) - vsqr2.y - vsqr2.z;
            cells[1][1] = value_type(1) - vsqr2.z - vsqr2.x;
            cells[2][2] = value_type(1) - vsqr2.x - vsqr2.y;

            cells[0][1] = xy2 - wz2;
            cells[0][2] = zx2 + wy2;
            cells[1][0] = xy2 + wz2;

            cells[1][2] = yz2 - wx2;
            cells[2][0] = zx2 - wy2;
            cells[2][1] = yz2 + wx2;
        }
    };

    template<typename value_type>
    struct rotation_matrix_t<value_type, EDim::_4, EDim::_4>
        : public matrix_t<value_type, EDim::_4, EDim::_4>
    {
        constexpr rotation_matrix_t(const quaternion<value_type>& q)
        {
            vector_t<value_type, EDim::_3> vsqr2 = q.v * q.v * value_type(2);
            value_type xy2 = q.v.x * q.v.y * value_type(2);
            value_type yz2 = q.v.y * q.v.z * value_type(2);
            value_type zx2 = q.v.z * q.v.x * value_type(2);
            value_type wx2 = q.w * q.v.x * value_type(2);
            value_type wy2 = q.w * q.v.y * value_type(2);
            value_type wz2 = q.w * q.v.z * value_type(2);

            cells[0][0] = value_type(1) - vsqr2.y - vsqr2.z;
            cells[1][1] = value_type(1) - vsqr2.z - vsqr2.x;
            cells[2][2] = value_type(1) - vsqr2.x - vsqr2.y;

            cells[0][1] = xy2 - wz2;
            cells[0][2] = zx2 + wy2;
            cells[1][0] = xy2 + wz2;

            cells[1][2] = yz2 - wx2;
            cells[2][0] = zx2 - wy2;
            cells[2][1] = yz2 + wx2;
        }
    };


    template<typename value_type>
    struct scale_matrix_t<value_type, EDim::_2, EDim::_2>
        : public matrix_t<value_type, EDim::_2, EDim::_2>
    {
        constexpr scale_matrix_t(value_type sx, value_type sy)
            : matrix_t(
                sx, value_type(0),
                value_type(0), sy
            )
        { }

        constexpr scale_matrix_t(value_type s) : scale_matrix_t(s, s) { }

        constexpr scale_matrix_t(const vector_t<value_type, EDim::_2>& s) : scale_matrix_t(s.x, s.y) { }
    };

    template<typename value_type>
    struct scale_matrix_t<value_type, EDim::_2, EDim::_3>
        : public matrix_t<value_type, EDim::_2, EDim::_3>
    {
        constexpr scale_matrix_t(value_type sx, value_type sy)
            : matrix_t(
                sx, value_type(0),
                value_type(0), sy
            )
        { }

        constexpr scale_matrix_t(value_type s) : scale_matrix_t(s, s) { }

        constexpr scale_matrix_t(const vector_t<value_type, EDim::_2>& s) : scale_matrix_t(s.x, s.y) { }
    };

    template<typename value_type>
    struct scale_matrix_t<value_type, EDim::_3, EDim::_3>
        : public matrix_t<value_type, EDim::_3, EDim::_3>
    {
        constexpr scale_matrix_t(value_type sx, value_type sy, value_type sz)
            : matrix_t(
                sx, value_type(0), value_type(0)
                value_type(0), sy, value_type(0)
                value_type(0), value_type(0), sz
            )
        { }

        constexpr scale_matrix_t(value_type s) : scale_matrix_t(s, s, s) { }

        constexpr scale_matrix_t(const vector_t<value_type, EDim::_3>&s) : scale_matrix_t(s.x, s.y, s.z) { }

        static constexpr scale_matrix_t make2d(value_type sx, value_type sy)
        {
            return scale_matrix_t<value_type, EDim::_3, EDim::_3>(sx, sy, value_type(1));
        }

        static constexpr scale_matrix_t make2d(value_type s)
        {
            return scale_matrix_t<value_type, EDim::_3, EDim::_3>(s, s, value_type(1));
        }

        static constexpr scale_matrix_t make2d(const vector_t<value_type, EDim::_2>& s)
        {
            return scale_matrix_t<value_type, EDim::_3, EDim::_3>(s.x, s.y, value_type(1));
        }
    };

    template<typename value_type>
    struct scale_matrix_t<value_type, EDim::_4, EDim::_4>
        : public matrix_t<value_type, EDim::_4, EDim::_4>
    {
        constexpr scale_matrix_t(value_type sx, value_type sy, value_type sz)
            : matrix_t(
                sx, value_type(0), value_type(0), value_type(0)
                value_type(0), sy, value_type(0), value_type(0)
                value_type(0), value_type(0), sz, value_type(0)
                value_type(0), value_type(0), value_type(0), value_type(1)
            )
        { }

        constexpr scale_matrix_t(value_type s) : scale_matrix_t(s, s, s) { }

        constexpr scale_matrix_t(const vector_t<value_type, EDim::_3>&s) : scale_matrix_t(s.x, s.y, s.z) { }
    };

    namespace math_impl
    {
        template<typename value_type> struct is_matrix : std::false_type { };
        template<typename value_type> struct is_square_matrix : std::false_type { };

        template<typename value_type, EDim row_dim, EDim col_dim>
        struct is_matrix<matrix_t<value_type, row_dim, col_dim>> : std::true_type { };

        template<typename value_type, EDim dimension>
        struct is_square_matrix<matrix_t<value_type, dimension, dimension>> : std::true_type { };

        template<typename value_type>
        constexpr value_type determinant2x2(
            value_type a00, value_type a01,
            value_type a10, value_type a11)
        {
            return a00 * a11 - a01 * a10;
        }

        template<typename value_type>
        constexpr value_type determinant2x3(
            value_type a00, value_type a01, value_type a02,
            value_type a10, value_type a11, value_type a12)
        {
            return a00 * a11 - a01 * a10;
        }

        template<typename value_type>
        constexpr value_type determinant3x3(
            value_type a00, value_type a01, value_type a02,
            value_type a10, value_type a11, value_type a12,
            value_type a20, value_type a21, value_type a22)
        {
            return a00 * determinant2x2(a11, a12, a21, a22)
                - a01 * determinant2x2(a10, a12, a20, a22)
                + a02 * determinant2x2(a10, a11, a20, a21);
        }

        template<typename value_type>
        constexpr value_type determinant4x4(
            value_type a00, value_type a01, value_type a02, value_type a03,
            value_type a10, value_type a11, value_type a12, value_type a13,
            value_type a20, value_type a21, value_type a22, value_type a23,
            value_type a30, value_type a31, value_type a32, value_type a33)
        {
            return a00 * determinant3x3(a11, a12, a13, a21, a22, a23, a31, a32, a33)
                - a01 * determinant3x3(a10, a12, a13, a20, a22, a23, a30, a32, a33)
                + a02 * determinant3x3(a10, a11, a13, a20, a21, a23, a30, a31, a33)
                - a03 * determinant3x3(a10, a11, a12, a20, a21, a22, a30, a31, a32);
        }
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3>
        matrix_t<value_type, EDim::_2, EDim::_3>::translation(value_type tx, value_type ty)
    {
        return translation_matrix_t<value_type, EDim::_2, EDim::_3>(tx, ty);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3>
        matrix_t<value_type, EDim::_2, EDim::_3>::translation(const vector_t<value_type, EDim::_2>& translation)
    {
        return translation_matrix_t<value_type, EDim::_2, EDim::_3>(translation);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::translation(value_type tx, value_type ty)
    {
        return translation_matrix_t<value_type, EDim::_3, EDim::_3>(tx, ty);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::translation(const vector_t<value_type, EDim::_2>& translation)
    {
        return translation_matrix_t<value_type, EDim::_3, EDim::_3>(translation);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4>
        matrix_t<value_type, EDim::_4, EDim::_4>::translation(value_type tx, value_type ty, value_type tz)
    {
        return translation_matrix_t<value_type, EDim::_4, EDim::_4>(tx, ty, tz);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4>
        matrix_t<value_type, EDim::_4, EDim::_4>::translation(const vector_t<value_type, EDim::_3>& translation)
    {
        return translation_matrix_t<value_type, EDim::_4, EDim::_4>(translation);
    }

    template<typename value_type>
    inline matrix_t<value_type, EDim::_2, EDim::_2>
        matrix_t<value_type, EDim::_2, EDim::_2>::rotation(const radian<value_type>& r)
    {
        return rotation_matrix_t<value_type, EDim::_2, EDim::_2>(r);
    }

    template<typename value_type>
    inline matrix_t<value_type, EDim::_2, EDim::_3>
        matrix_t<value_type, EDim::_2, EDim::_3>::rotation(const radian<value_type>& r)
    {
        return rotation_matrix_t<value_type, EDim::_2, EDim::_3>(r);
    }

    template<typename value_type>
    inline matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::rotation(const radian<value_type>& r)
    {
        return rotation_matrix_t<value_type, EDim::_3, EDim::_3>(r);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::rotation(const quaternion<value_type>& q)
    {
        return rotation_matrix_t<value_type, EDim::_3, EDim::_3>(q);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4>
        matrix_t<value_type, EDim::_4, EDim::_4>::rotation(const quaternion<value_type>& q)
    {
        return rotation_matrix_t<value_type, EDim::_4, EDim::_4>(q);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_2>
        matrix_t<value_type, EDim::_2, EDim::_2>::scale(value_type sx, value_type sy)
    {
        return scale_matrix_t<value_type, EDim::_2, EDim::_2>(sx, sy);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_2>
        matrix_t<value_type, EDim::_2, EDim::_2>::scale(value_type s)
    {
        return scale_matrix_t<value_type, EDim::_2, EDim::_2>(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_2>
        matrix_t<value_type, EDim::_2, EDim::_2>::scale(const vector_t<value_type, EDim::_2>& s)
    {
        return scale_matrix_t<value_type, EDim::_2, EDim::_2>(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3>
        matrix_t<value_type, EDim::_2, EDim::_3>::scale(value_type sx, value_type sy)
    {
        return scale_matrix_t<value_type, EDim::_2, EDim::_3>(sx, sy);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3>
        matrix_t<value_type, EDim::_2, EDim::_3>::scale(value_type s)
    {
        return scale_matrix_t<value_type, EDim::_2, EDim::_3>(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3>
        matrix_t<value_type, EDim::_2, EDim::_3>::scale(const vector_t<value_type, EDim::_2>& s)
    {
        return scale_matrix_t<value_type, EDim::_2, EDim::_3>(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::scale(value_type sx, value_type sy, value_type sz)
    {
        return scale_matrix_t<value_type, EDim::_3, EDim::_3>(sx, sy, sz);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::scale(value_type s)
    {
        return scale_matrix_t<value_type, EDim::_3, EDim::_3>(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::scale(const vector_t<value_type, EDim::_3>& s)
    {
        return scale_matrix_t<value_type, EDim::_3, EDim::_3>(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::scale2d(value_type sx, value_type sy)
    {
        return scale_matrix_t<value_type, EDim::_3, EDim::_3>::make2d(sx, sy);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::scale2d(value_type s)
    {
        return scale_matrix_t<value_type, EDim::_3, EDim::_3>::make2d(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3>
        matrix_t<value_type, EDim::_3, EDim::_3>::scale2d(const vector_t<value_type, EDim::_2>& s)
    {
        return scale_matrix_t<value_type, EDim::_3, EDim::_3>::make2d(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4>
        matrix_t<value_type, EDim::_4, EDim::_4>::scale(value_type sx, value_type sy, value_type sz)
    {
        return scale_matrix_t<value_type, EDim::_4, EDim::_4>(sx, sy, sz);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4>
        matrix_t<value_type, EDim::_4, EDim::_4>::scale(value_type s)
    {
        return scale_matrix_t<value_type, EDim::_4, EDim::_4>(s);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4>
        matrix_t<value_type, EDim::_4, EDim::_4>::scale(const vector_t<value_type, EDim::_3>& s)
    {
        return scale_matrix_t<value_type, EDim::_4, EDim::_4>(s);
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr bool operator== (const matrix_t<value_type, Row, Col>& l, const matrix_t<value_type, Row, Col>& r)
    {
        for (size_t ri = 0; ri < Row; ri++)
        {
            for (size_t ci = 0; ci < Col; ci++)
            {
                if (!is_equal(l.cells[ri][ci], r.cells[ri][ci]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr bool operator!= (const matrix_t<value_type, Row, Col>& l, const matrix_t<value_type, Row, Col>& r)
    {
        return !(l == r);
    }


    template<typename value_type, EDim RowL, EDim ColLxRowR, EDim ColR>
    inline matrix_t<value_type, RowL, ColR> operator* (
        const matrix_t<value_type, RowL, ColLxRowR>& l,
        const matrix_t<value_type, ColLxRowR, ColR>& r)
    {
        matrix_t<value_type, RowL, ColR> rst;
        for (size_t ri = 0; ri < RowL; ri++)
        {
            for (size_t ci = 0; ci < ColR; ci++)
            {
                rst.cells[ri][ci] = dot(l.rows[ri], r.column(ci));
            }
        }
        return rst;
    }

    template<typename value_type>
    inline matrix_t<value_type, EDim::_2, EDim::_3> operator*(
        const matrix_t<value_type, EDim::_2, EDim::_3>& l,
        const matrix_t<value_type, EDim::_2, EDim::_3>& r)
    {
        matrix_t<value_type, EDim::_2, EDim::_3> rst;
        for (size_t ri = 0; ri < matrix_t<value_type, EDim::_2, EDim::_3>::Row; ri++)
        {
            for (size_t ci = 0; ci < matrix_t<value_type, EDim::_2, EDim::_3>::Col; ci++)
            {
                rst.cells[ri][ci] = dot(l.rows[ri], r.column3(ci));
            }
        }
        return rst;
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr const matrix_t<value_type, Row, Col> operator* (const matrix_t<value_type, Row, Col>& l, scaler_t<value_type> r)
    {
        matrix_t<value_type, Row, Col>  rst;
        for (size_t ri = 0; ri < Row; ri++)
        {
            for (size_t ci = 0; ci < Col; ci++)
            {
                rst.cells[ri][ci] = l.cells[ri][ci] * r;
            }
        }
        return rst;
    }

    template<typename value_type>
    inline vector_t<value_type, EDim::_3> operator* (const matrix_t<value_type, EDim::_2, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(dot(l.rows[0], r), dot(l.rows[1], r), r.z);
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr matrix_t<value_type, Row, Col> operator* (scaler_t<value_type> l, const matrix_t<value_type, Row, Col>& r)
    {
        return r * l;
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr vector_t<value_type, Row> operator* (const matrix_t<value_type, Row, Col>& l, const vector_t<value_type, Col>& r)
    {
        vector_t<value_type, Row> rst;
        for (size_t ri = 0; ri < Row; ri++)
        {
            rst.v[ri] = dot(l.rows[ri], r);
        }
        return rst;
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr vector_t<value_type, Col> operator* (const vector_t<value_type, Row>& l, const matrix_t<value_type, Row, Col>& r)
    {
        vector_t<value_type, Col> rst;
        for (size_t ci = 0; ci < Col; ci++)
        {
            rst.v[ci] = dot(l, r.column(ci));
        }
        return rst;
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr matrix_t<value_type, Row, Col>& operator*=(matrix_t<value_type, Row, Col>& l, const matrix_t<value_type, Row, Col>& r)
    {
        for (size_t ri = 0; ri < Row; ri++)
        {
            for (size_t ci = 0; ci < Col; ci++)
            {
                l.cells[ri][ci] *= r.cells[ri][ci];
            }
        }
        return l;
    }

    template<typename value_type, EDim Row, EDim Col>
    constexpr matrix_t<value_type, Row, Col>& operator*=(matrix_t<value_type, Row, Col>& l, scaler_t<value_type> r)
    {
        return l = l * r;
    }

    template<typename value_type>
    inline bool is_orthogonal(const matrix_t<value_type, EDim::_2, EDim::_2>& matrix)
    {
        vector_t<value_type, EDim::_2> r0 = matrix.rows[0];
        vector_t<value_type, EDim::_2> r1 = matrix.rows[1];
        if (!is_equal(magnitude_sqr(r0), value_type(1)) ||
            !is_equal(magnitude_sqr(r1), value_type(1)))
        {
            return false;
        }

        return is_equal(dot(r0, r1), value_type(0));
    }

    template<typename value_type>
    inline bool is_orthogonal(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        if (!is_equal(matrix.cells[0][2], value_type(0)) ||
            !is_equal(matrix.cells[1][2], value_type(0)))
        {
            return false;
        }

        vector_t<value_type, EDim::_2> r0 = matrix.rows[0];
        vector_t<value_type, EDim::_2> r1 = matrix.rows[1];
        if (!is_equal(magnitude_sqr(r0), value_type(1)) ||
            !is_equal(magnitude_sqr(r1), value_type(1)))
        {
            return false;
        }

        return is_equal(dot(r0, r1), value_type(0));
    }

    template<typename value_type>
    inline bool is_orthogonal(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        vector_t<value_type, EDim::_3> r0 = matrix.rows[0];
        vector_t<value_type, EDim::_3> r1 = matrix.rows[1];
        vector_t<value_type, EDim::_3> r2 = matrix.rows[2];
        if (!is_equal(magnitude_sqr(r0), value_type(1)) ||
            !is_equal(magnitude_sqr(r1), value_type(1)) ||
            !is_equal(magnitude_sqr(r2), value_type(1)))
        {
            return false;
        }

        return is_equal(dot(r0, r1), value_type(0))
            && is_equal(dot(r1, r2), value_type(0))
            && is_equal(dot(r2, r0), value_type(0));
    }

    template<typename value_type>
    inline bool is_orthogonal(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        vector_t<value_type, EDim::_4> r0 = matrix.rows[0];
        vector_t<value_type, EDim::_4> r1 = matrix.rows[1];
        vector_t<value_type, EDim::_4> r2 = matrix.rows[2];
        vector_t<value_type, EDim::_4> r3 = matrix.rows[3];
        if (!is_equal(magnitude_sqr(r0), value_type(1)) ||
            !is_equal(magnitude_sqr(r1), value_type(1)) ||
            !is_equal(magnitude_sqr(r2), value_type(1)) ||
            !is_equal(magnitude_sqr(r3), value_type(1)))
        {
            return false;
        }

        return is_equal(dot(r0, r1), value_type(0))
            && is_equal(dot(r0, r2), value_type(0))
            && is_equal(dot(r0, r3), value_type(0))
            && is_equal(dot(r1, r2), value_type(0))
            && is_equal(dot(r1, r3), value_type(0))
            && is_equal(dot(r2, r3), value_type(0));
    }

    template<typename value_type>
    constexpr value_type determinant(const matrix_t<value_type, EDim::_2, EDim::_2>& matrix)
    {
        return math_impl::determinant2x2
        (
            matrix.cells[0][0], matrix.cells[0][1],
            matrix.cells[1][0], matrix.cells[1][1]
        );
    }

    template<typename value_type>
    constexpr value_type determinant(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        return math_impl::determinant2x3
        (
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2],
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[2][2]
        );
    }

    template<typename value_type>
    constexpr value_type determinant(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        return math_impl::determinant3x3
        (
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2],
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[2][2],
            matrix.cells[2][0], matrix.cells[2][1], matrix.cells[2][2]
        );
    }

    template<typename value_type>
    constexpr value_type determinant(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        return math_impl::determinant4x4
        (
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2], matrix.cells[0][3],
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[1][2], matrix.cells[1][3],
            matrix.cells[2][0], matrix.cells[2][1], matrix.cells[2][2], matrix.cells[2][3],
            matrix.cells[3][0], matrix.cells[3][1], matrix.cells[3][2], matrix.cells[3][3]
        );
    }

    template<typename value_type>
    inline void transpose(matrix_t<value_type, EDim::_2, EDim::_2>& matrix)
    {
        std::swap(matrix.cells[1][0], matrix.cells[0][1]);
    }

    template<typename value_type>
    inline void transpose(matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        std::swap(matrix.cells[1][0], matrix.cells[0][1]);
        std::swap(matrix.cells[2][0], matrix.cells[0][2]);
        std::swap(matrix.cells[2][1], matrix.cells[1][2]);
    }

    template<typename value_type>
    inline void transpose(matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        std::swap(matrix.cells[1][0], matrix.cells[0][1]);
        std::swap(matrix.cells[2][0], matrix.cells[0][2]);
        std::swap(matrix.cells[3][0], matrix.cells[0][3]);
        std::swap(matrix.cells[2][1], matrix.cells[1][2]);
        std::swap(matrix.cells[3][1], matrix.cells[1][3]);
        std::swap(matrix.cells[3][2], matrix.cells[2][3]);
    }

    template<typename value_type, EDim dimension>
    constexpr matrix_t<value_type, dimension, dimension> transposed(const matrix_t<value_type, dimension, dimension>& matrix)
    {
        matrix_t<value_type, dimension, dimension> rst(matrix);
        transpose(rst);
        return rst;
    }

    template<typename value_type, EDim dimension>
    constexpr bool can_invert(const matrix_t<value_type, dimension, dimension>& matrix)
    {
        return is_orthogonal(matrix) || !is_equal(determinant(matrix), value_type(0));
    }

    template<typename value_type>
    constexpr bool can_invert(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        return is_orthogonal(matrix) || !is_equal(determinant(matrix), value_type(0));
    }

    template<typename value_type>
    inline void inverse(matrix_t<value_type, EDim::_2, EDim::_2>& matrix)
    {
        if (is_orthogonal(matrix))
        {
            transpose(matrix);
        }
        else
        {
            value_type det = determinant(matrix);
            if (!is_equal(det, value_type(0)))
            {
                //calc adjoint matrix
                std::swap(matrix.cells[0][0], matrix.cells[1][1]);
                matrix.cells[0][1] = -matrix.cells[0][1];
                matrix.cells[1][0] = -matrix.cells[1][0];
                matrix *= (value_type(1) / det);
            }
            else
            {
                //TODO:
                // can not invert here,
                // what should i do when this occur ?
                matrix = matrix_t<value_type, EDim::_2, EDim::_2>::identity();
            }
        }
    }

    /**
    *	it is dangerous unless u know what ur doing.
    */
    template<typename value_type>
    inline void inverse(matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        if (is_orthogonal(matrix))
        {
            std::swap(matrix.cells[0][1], matrix.cells[1][0]);
        }
        else
        {
            value_type det = determinant(matrix);
            if (!is_equal(det, value_type(0)))
            {
                matrix_t<value_type, EDim::_2, EDim::_3> old = matrix;

                swap(matrix.cells[0][0], matrix.cells[1][1]);
                matrix.cells[0][1] = -matrix.cells[0][1];
                matrix.cells[1][0] = -matrix.cells[1][0];
                matrix.cells[0][2] = +math_impl::determinant2x2(old.cells[0][1], old.cells[0][2], old.cells[1][1], old.cells[1][2]);
                matrix.cells[1][2] = -math_impl::determinant2x2(old.cells[0][0], old.cells[0][2], old.cells[1][0], old.cells[1][2]);
                matrix *= (value_type(1) / det);
            }
            else
            {
                //TODO:
                // can not invert here,
                // what should i do when this occur ?
                matrix = matrix_t<value_type, EDim::_2, EDim::_3>::identity();
            }
        }
    }

    template<typename value_type>
    inline void inverse(matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        if (is_orthogonal(matrix))
        {
            transpose(matrix);
        }
        else
        {
            value_type det = determinant(matrix);
            if (!is_equal(det, value_type(0)))
            {
                det = value_type(1) / det;
                matrix_t<value_type, EDim::_3, EDim::_3> old = matrix;

                //to-do det_impl is calculated above in determinant().
                //try to gcd
                matrix.cells[0][0] = +math_impl::determinant2x2(old.cells[1][1], old.cells[1][2], old.cells[2][1], old.cells[2][2]) * det;
                matrix.cells[1][0] = -math_impl::determinant2x2(old.cells[1][0], old.cells[1][2], old.cells[2][0], old.cells[2][2]) * det;
                matrix.cells[2][0] = +math_impl::determinant2x2(old.cells[1][0], old.cells[1][1], old.cells[2][0], old.cells[2][1]) * det;

                matrix.cells[0][1] = -math_impl::determinant2x2(old.cells[0][1], old.cells[0][2], old.cells[2][1], old.cells[2][2]) * det;
                matrix.cells[1][1] = +math_impl::determinant2x2(old.cells[0][0], old.cells[0][2], old.cells[2][0], old.cells[2][2]) * det;
                matrix.cells[2][1] = -math_impl::determinant2x2(old.cells[0][0], old.cells[0][1], old.cells[2][0], old.cells[2][1]) * det;

                matrix.cells[0][2] = +math_impl::determinant2x2(old.cells[0][1], old.cells[0][2], old.cells[1][1], old.cells[1][2]) * det;
                matrix.cells[1][2] = -math_impl::determinant2x2(old.cells[0][0], old.cells[0][2], old.cells[1][0], old.cells[1][2]) * det;
                matrix.cells[2][2] = +math_impl::determinant2x2(old.cells[0][0], old.cells[0][1], old.cells[1][0], old.cells[1][1]) * det;
            }
            else
            {
                //TODO:
                // can not invert here,
                // what should i do when this occur ?
                matrix = matrix_t<value_type, EDim::_3, EDim::_3>::identity();
            }
        }
    }

    template<typename value_type>
    inline void inverse(matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        if (is_orthogonal(matrix))
        {
            transpose(matrix);
        }
        else
        {
            value_type det = determinant(matrix);
            if (!is_equal(det, value_type(0)))
            {
                det = value_type(1) / det;
                matrix_t<value_type, EDim::_4, EDim::_4> old = matrix;

                //to-do det_impl is calculated above in determinant().
                //try to gcd
                matrix.cells[0][0] = +math_impl::determinant3x3(old.cells[1][1], old.cells[1][2], old.cells[1][3], old.cells[2][1], old.cells[2][2], old.cells[2][3], old.cells[3][1], old.cells[3][2], old.cells[3][3]) * det;
                matrix.cells[1][0] = -math_impl::determinant3x3(old.cells[1][0], old.cells[1][2], old.cells[1][3], old.cells[2][0], old.cells[2][2], old.cells[2][3], old.cells[3][0], old.cells[3][2], old.cells[3][3]) * det;
                matrix.cells[2][0] = +math_impl::determinant3x3(old.cells[1][0], old.cells[1][1], old.cells[1][3], old.cells[2][0], old.cells[2][1], old.cells[2][3], old.cells[3][0], old.cells[3][1], old.cells[3][3]) * det;
                matrix.cells[3][0] = -math_impl::determinant3x3(old.cells[1][0], old.cells[1][1], old.cells[1][2], old.cells[2][0], old.cells[2][1], old.cells[2][2], old.cells[3][0], old.cells[3][1], old.cells[3][2]) * det;

                matrix.cells[0][1] = -math_impl::determinant3x3(old.cells[0][1], old.cells[0][2], old.cells[0][3], old.cells[2][1], old.cells[2][2], old.cells[2][3], old.cells[3][1], old.cells[3][2], old.cells[3][3]) * det;
                matrix.cells[1][1] = +math_impl::determinant3x3(old.cells[0][0], old.cells[0][2], old.cells[0][3], old.cells[2][0], old.cells[2][2], old.cells[2][3], old.cells[3][0], old.cells[3][2], old.cells[3][3]) * det;
                matrix.cells[2][1] = -math_impl::determinant3x3(old.cells[0][0], old.cells[0][1], old.cells[0][3], old.cells[2][0], old.cells[2][1], old.cells[2][3], old.cells[3][0], old.cells[3][1], old.cells[3][3]) * det;
                matrix.cells[3][1] = +math_impl::determinant3x3(old.cells[0][0], old.cells[0][1], old.cells[0][2], old.cells[2][0], old.cells[2][1], old.cells[2][2], old.cells[3][0], old.cells[3][1], old.cells[3][2]) * det;

                matrix.cells[0][2] = +math_impl::determinant3x3(old.cells[0][1], old.cells[0][2], old.cells[0][3], old.cells[1][1], old.cells[1][2], old.cells[1][3], old.cells[3][1], old.cells[3][2], old.cells[3][3]) * det;
                matrix.cells[1][2] = -math_impl::determinant3x3(old.cells[0][0], old.cells[0][2], old.cells[0][3], old.cells[1][0], old.cells[1][2], old.cells[1][3], old.cells[3][0], old.cells[3][2], old.cells[3][3]) * det;
                matrix.cells[2][2] = +math_impl::determinant3x3(old.cells[0][0], old.cells[0][1], old.cells[0][3], old.cells[1][0], old.cells[1][1], old.cells[1][3], old.cells[3][0], old.cells[3][1], old.cells[3][3]) * det;
                matrix.cells[3][2] = -math_impl::determinant3x3(old.cells[0][0], old.cells[0][1], old.cells[0][2], old.cells[1][0], old.cells[1][1], old.cells[1][2], old.cells[3][0], old.cells[3][1], old.cells[3][2]) * det;

                matrix.cells[0][3] = -math_impl::determinant3x3(old.cells[0][1], old.cells[0][2], old.cells[0][3], old.cells[1][1], old.cells[1][2], old.cells[1][3], old.cells[2][1], old.cells[2][2], old.cells[2][3]) * det;
                matrix.cells[1][3] = +math_impl::determinant3x3(old.cells[0][0], old.cells[0][2], old.cells[0][3], old.cells[1][0], old.cells[1][2], old.cells[1][3], old.cells[2][0], old.cells[2][2], old.cells[2][3]) * det;
                matrix.cells[2][3] = -math_impl::determinant3x3(old.cells[0][0], old.cells[0][1], old.cells[0][3], old.cells[1][0], old.cells[1][1], old.cells[1][3], old.cells[2][0], old.cells[2][1], old.cells[2][3]) * det;
                matrix.cells[3][3] = +math_impl::determinant3x3(old.cells[0][0], old.cells[0][1], old.cells[0][2], old.cells[1][0], old.cells[1][1], old.cells[1][2], old.cells[2][0], old.cells[2][1], old.cells[2][2]) * det;
            }
            else
            {
                //TODO:
                // can not invert here,
                // what should i do when this occur ?
                matrix = matrix_t<value_type, EDim::_4, EDim::_4>::identity();
            }
        }
    }

    template<typename value_type, EDim dimension>
    inline matrix_t<value_type, dimension, dimension> inversed(const matrix_t<value_type, dimension, dimension>& matrix)
    {
        matrix_t<value_type, dimension, dimension> rst(matrix);
        inverse(rst);
        return rst;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> get_scale(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        value_type xscale = magnitude(matrix.column3(0));
        value_type yscale = magnitude(matrix.column3(1));
        return vector_t<value_type, EDim::_3>(xscale, yscale);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> get_scale(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        value_type xscale = magnitude(matrix.column3(0));
        value_type yscale = magnitude(matrix.column3(1));
        return vector_t<value_type, EDim::_3>(xscale, yscale);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> get_scale3(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        value_type xscale = magnitude(matrix.column3(0));
        value_type yscale = magnitude(matrix.column3(1));
        value_type zscale = magnitude(matrix.column3(2));
        return vector_t<value_type, EDim::_3>(xscale, yscale, zscale);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> get_scale(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        value_type xscale = magnitude(matrix.column3(0));
        value_type yscale = magnitude(matrix.column3(1));
        value_type zscale = magnitude(matrix.column3(2));
        return vector_t<value_type, EDim::_3>(xscale, yscale, zscale);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_2> get_rotation_matrix2x2(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_2>(
            matrix.cells[0][0], matrix.cells[0][1],
            matrix.cells[1][0], matrix.cells[1][1]);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_2> get_rotation_matrix2x2(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_2>(
            matrix.cells[0][0], matrix.cells[0][1],
            matrix.cells[1][0], matrix.cells[1][1]);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3> get_rotation_matrix2x3(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], value_type(0),
            matrix.cells[1][0], matrix.cells[1][1], value_type(0));
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3> get_rotation_matrix2x3(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], value_type(0),
            matrix.cells[1][0], matrix.cells[1][1], value_type(0));
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3> get_rotation_matrix3x3(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        return matrix_t<value_type, EDim::_3, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2],
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[1][2],
            matrix.cells[2][0], matrix.cells[2][1], matrix.cells[2][2]);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4> get_rotation_matrix4x4(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        return matrix_t<value_type, EDim::_4, EDim::_4>(
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2], value_type(0),
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[1][2], value_type(0),
            matrix.cells[2][0], matrix.cells[2][1], matrix.cells[2][2], value_type(0),
            value_type(0), value_type(0), value_type(0), value_type(1));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> get_translation(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        vector_t<value_type, EDim::_2> r_t0 = matrix.column(0);
        vector_t<value_type, EDim::_2> r_t1 = matrix.column(1);
        vector_t<value_type, EDim::_2> t = matrix.column(2);

        return vector_t<value_type, EDim::_2>(-dot(r_t0, t), -dot(r_t1, t));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> get_translation(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        vector_t<value_type, EDim::_2> r_t0 = matrix.column2(0);
        vector_t<value_type, EDim::_2> r_t1 = matrix.column2(1);
        vector_t<value_type, EDim::_2> t = matrix.column2(2);

        return vector_t<value_type, EDim::_2>(-dot(r_t0, t), -dot(r_t1, t));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> get_translation(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        vector_t<value_type, EDim::_3> r_t0 = matrix.column3(0);
        vector_t<value_type, EDim::_3> r_t1 = matrix.column3(1);
        vector_t<value_type, EDim::_3> r_t2 = matrix.column3(2);
        vector_t<value_type, EDim::_3> t = matrix.column3(3);

        return vector_t<value_type, EDim::_3>(-dot(r_t0, t), -dot(r_t1, t), -dot(r_t2, t));
    }

    template<typename value_type>
    inline vector_t<value_type, EDim::_2> transform(
        const matrix_t<value_type, EDim::_2, EDim::_2>& l,
        const vector_t<value_type, EDim::_2>& r)
    {
        return l * r;
    }

    template<typename value_type>
    inline vector_t<value_type, EDim::_3> transform(
        const matrix_t<value_type, EDim::_2, EDim::_3>& l,
        const vector_t<value_type, EDim::_3>& r)
    {
        return l * r;
    }

    template<typename value_type>
    inline vector_t<value_type, EDim::_3> transform(
        const matrix_t<value_type, EDim::_3, EDim::_3>& l,
        const vector_t<value_type, EDim::_3>& r)
    {
        return l * r;
    }

    template<typename value_type>
    inline vector_t<value_type, EDim::_4> transform(
        const matrix_t<value_type, EDim::_4, EDim::_4>& l,
        const vector_t<value_type, EDim::_4>& r)
    {
        return l * r;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> transform(
        const matrix_t<value_type, EDim::_2, EDim::_3>& l,
        const vector_t<value_type, EDim::_2>& r)
    {
        vector_t<value_type, EDim::_3> rhs3(r, value_type(0));
        return vector_t<value_type, EDim::_2>(
            dot(l.rows[0], rhs3),
            dot(l.rows[1], rhs3));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> transform(
        const matrix_t<value_type, EDim::_3, EDim::_3>& l,
        const vector_t<value_type, EDim::_2>& r)
    {
        vector_t<value_type, EDim::_3> rhs3(r, value_type(0));
        return vector_t<value_type, EDim::_2>(
            dot(l.rows[0], rhs3),
            dot(l.rows[1], rhs3));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> transform(
        const matrix_t<value_type, EDim::_4, EDim::_4>& l,
        const vector_t<value_type, EDim::_3>& r)
    {
        vector_t<value_type, EDim::_4> rhs4(r, value_type(0));
        return vector_t<value_type, EDim::_3>(
            dot(l.rows[0], rhs4),
            dot(l.rows[1], rhs4),
            dot(l.rows[2], rhs4));
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3> convert_to_matrix2x3(const matrix_t<value_type, EDim::_2, EDim::_2>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], value_type(0),
            matrix.cells[1][0], matrix.cells[1][1], value_type(0));
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3> convert_to_matrix3x3(const matrix_t<value_type, EDim::_2, EDim::_2>& matrix)
    {
        return matrix_t<value_type, EDim::_3, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], value_type(0),
            matrix.cells[1][0], matrix.cells[1][1], value_type(0),
            value_type(0), value_type(0), value_type(1));
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_2> convert_to_matrix2x2(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_2>(
            matrix.cells[0][0], matrix.cells[0][1],
            matrix.cells[1][0], matrix.cells[1][1]);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3> convert_to_matrix3x3(const matrix_t<value_type, EDim::_2, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_3, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2],
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[1][2],
            value_type(0), value_type(0), value_type(1));
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_2> convert_to_matrix2x2(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_2>(
            matrix.cells[0][0], matrix.cells[0][1],
            matrix.cells[1][0], matrix.cells[1][1]);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_2, EDim::_3> convert_to_matrix2x3(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_2, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2],
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[1][2]);
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_4, EDim::_4> convert_to_matrix4x4(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        return matrix_t<value_type, EDim::_4, EDim::_4>(
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2], value_type(0),
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[1][2], value_type(0),
            matrix.cells[2][0], matrix.cells[2][1], matrix.cells[2][2], value_type(0),
            value_type(0), value_type(0), value_type(0), value_type(1));
    }

    template<typename value_type>
    constexpr matrix_t<value_type, EDim::_3, EDim::_3> convert_to_matrix3x3(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        return matrix_t<value_type, EDim::_3, EDim::_3>(
            matrix.cells[0][0], matrix.cells[0][1], matrix.cells[0][2],
            matrix.cells[1][0], matrix.cells[1][1], matrix.cells[1][2],
            matrix.cells[2][0], matrix.cells[2][1], matrix.cells[2][2]);
    }

    template<typename value_type>
    constexpr rotation_matrix_t<value_type, EDim::_3, EDim::_3> convert_to_matrix3x3(const quaternion<value_type>& q)
    {
        return rotation_matrix_t<value_type, EDim::_3, EDim::_3>(q);
    }

    template<typename value_type>
    constexpr rotation_matrix_t<value_type, EDim::_4, EDim::_4> convert_to_matrix4x4(const quaternion<value_type>& q)
    {
        return rotation_matrix_t<value_type, EDim::_4, EDim::_4>(q);
    }

    namespace math_impl
    {
        enum { WSqr, XSqr, YSqr, ZSqr };
    }
    template<typename value_type>
    inline quaternion<value_type> convert_to_quaternion(const matrix_t<value_type, EDim::_3, EDim::_3>& matrix)
    {
        value_type wsqr = matrix.cells[0][0] + matrix.cells[1][1] + matrix.cells[2][2];
        value_type xsqr = matrix.cells[0][0] - matrix.cells[1][1] - matrix.cells[2][2];
        value_type ysqr = matrix.cells[1][1] - matrix.cells[0][0] - matrix.cells[2][2];
        value_type zsqr = matrix.cells[2][2] - matrix.cells[0][0] - matrix.cells[1][1];

        int maxIndex = WSqr;
        value_type maxSqr = wsqr;
        if (xsqr > maxSqr)
        {
            maxSqr = xsqr;
            maxIndex = XSqr;
        }
        if (ysqr > maxSqr)
        {
            maxSqr = ysqr;
            maxIndex = YSqr;
        }
        if (zsqr > maxSqr)
        {
            maxSqr = zsqr;
            maxIndex = zSqr;
        }

        maxSqr = sqrtf(maxSqr + value_type(1)) * value_type(0.5);
        value_type base = value_type(0.25) / maxSqr;
        quaternion<value_type> rst;
        switch (maxIndex)
        {
        case WSqr:
            rst.w = maxSqr;
            rst.v.x = (matrix.cells[2][1] - matrix.cells[1][2]) * base;
            rst.v.y = (matrix.cells[0][2] - matrix.cells[2][0]) * base;
            rst.v.z = (matrix.cells[1][0] - matrix.cells[0][1]) * base;
            break;
        case XSqr:
            rst.v.x = maxSqr;
            rst.w = (matrix.cells[2][1] - matrix.cells[1][2]) * base;
            rst.v.y = (matrix.cells[1][0] + matrix.cells[0][1]) * base;
            rst.v.z = (matrix.cells[2][0] + matrix.cells[0][2]) * base;
            break;
        case YSqr:
            rst.v.y = maxSqr;
            rst.w = (matrix.cells[0][2] - matrix.cells[2][0]) * base;
            rst.v.x = (matrix.cells[1][0] + matrix.cells[0][1]) * base;
            rst.v.z = (matrix.cells[2][1] + matrix.cells[1][2]) * base;
            break;
        case ZSqr:
            rst.v.z = maxSqr;
            rst.w = (matrix.cells[1][0] - matrix.cells[0][1]) * base;
            rst.v.x = (matrix.cells[2][0] + matrix.cells[0][2]) * base;
            rst.v.y = (matrix.cells[2][1] + matrix.cells[1][2]) * base;
            break;
        }

        return normalized(rst);
    }

    template<typename value_type>
    inline quaternion<value_type> convert_to_quaternion(const matrix_t<value_type, EDim::_4, EDim::_4>& matrix)
    {
        value_type wsqr = matrix.cells[0][0] + matrix.cells[1][1] + matrix.cells[2][2];
        value_type xsqr = matrix.cells[0][0] - matrix.cells[1][1] - matrix.cells[2][2];
        value_type ysqr = matrix.cells[1][1] - matrix.cells[0][0] - matrix.cells[2][2];
        value_type zsqr = matrix.cells[2][2] - matrix.cells[0][0] - matrix.cells[1][1];

        int maxIndex = math_impl::WSqr;
        value_type maxSqr = wsqr;
        if (xsqr > maxSqr)
        {
            maxSqr = xsqr;
            maxIndex = math_impl::XSqr;
        }
        if (ysqr > maxSqr)
        {
            maxSqr = ysqr;
            maxIndex = math_impl::YSqr;
        }
        if (zsqr > maxSqr)
        {
            maxSqr = zsqr;
            maxIndex = math_impl::ZSqr;
        }

        maxSqr = sqrt(maxSqr + value_type(1)) * value_type(0.5);
        value_type base = value_type(0.25) / maxSqr;
        quaternion<value_type> rst;
        switch (maxIndex)
        {
        case math_impl::WSqr:
            rst.w = maxSqr;
            rst.v.x = (matrix.cells[2][1] - matrix.cells[1][2]) * base;
            rst.v.y = (matrix.cells[0][2] - matrix.cells[2][0]) * base;
            rst.v.z = (matrix.cells[1][0] - matrix.cells[0][1]) * base;
            break;
        case math_impl::XSqr:
            rst.v.x = maxSqr;
            rst.w = (matrix.cells[2][1] - matrix.cells[1][2]) * base;
            rst.v.y = (matrix.cells[1][0] + matrix.cells[0][1]) * base;
            rst.v.z = (matrix.cells[2][0] + matrix.cells[0][2]) * base;
            break;
        case math_impl::YSqr:
            rst.v.y = maxSqr;
            rst.w = (matrix.cells[0][2] - matrix.cells[2][0]) * base;
            rst.v.x = (matrix.cells[1][0] + matrix.cells[0][1]) * base;
            rst.v.z = (matrix.cells[2][1] + matrix.cells[1][2]) * base;
            break;
        case math_impl::ZSqr:
            rst.v.z = maxSqr;
            rst.w = (matrix.cells[1][0] - matrix.cells[0][1]) * base;
            rst.v.x = (matrix.cells[2][0] + matrix.cells[0][2]) * base;
            rst.v.y = (matrix.cells[2][1] + matrix.cells[1][2]) * base;
            break;
        }

        return normalized(rst);
    }

    template<typename value_type> using matrix2x2 = matrix_t<value_type, EDim::_2, EDim::_2>;
    template<typename value_type> using matrix2x3 = matrix_t<value_type, EDim::_2, EDim::_3>;
    template<typename value_type> using matrix3x3 = matrix_t<value_type, EDim::_3, EDim::_3>;
    template<typename value_type> using matrix4x4 = matrix_t<value_type, EDim::_4, EDim::_4>;


    template<typename value_type> using translation_matrix2x3 = translation_matrix_t<value_type, EDim::_2, EDim::_3>;
    template<typename value_type> using translation_matrix3x3 = translation_matrix_t<value_type, EDim::_3, EDim::_3>;
    template<typename value_type> using translation_matrix4x4 = translation_matrix_t<value_type, EDim::_4, EDim::_4>;
    template<typename value_type> using rotation_matrix3x3 = rotation_matrix_t<value_type, EDim::_3, EDim::_3>;
    template<typename value_type> using rotation_matrix4x4 = rotation_matrix_t<value_type, EDim::_4, EDim::_4>;
    template<typename value_type> using scale_matrix2x3 = scale_matrix_t<value_type, EDim::_2, EDim::_3>;
    template<typename value_type> using scale_matrix3x3 = scale_matrix_t<value_type, EDim::_3, EDim::_3>;
    template<typename value_type> using scale_matrix4x4 = scale_matrix_t<value_type, EDim::_4, EDim::_4>;

    using float2x2 = matrix2x2<float>;
    using float2x3 = matrix2x3<float>;
    using float3x3 = matrix3x3<float>;
    using float4x4 = matrix4x4<float>;

    using double2x2 = matrix2x2<double>;
    using double2x3 = matrix2x3<double>;
    using double3x3 = matrix3x3<double>;
    using double4x4 = matrix4x4<double>;


    using translation_float4x4 = translation_matrix4x4<float>;
    using rotation_float4x4 = rotation_matrix4x4<float>;
    using scale_float4x4 = scale_matrix4x4<float>;
}
