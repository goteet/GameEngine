#pragma once
#include "PredefinedConstantValues.h"

#include <algorithm>

namespace math
{
    template<typename value_type>
    struct helper_identity { using type = value_type; };

    template<typename value_type>
    using scaler_t = typename helper_identity<value_type>::type;

    // Utility Functions
    template<typename value_type>
    constexpr value_type frac(value_type f)
    {
        return f - static_cast<int>(f);
    }

    template<typename value_type, typename = std::enable_if_t<std::is_floating_point_v<value_type>>>
    constexpr bool near_zero(value_type v, value_type small_number = SMALL_NUM<value_type>)
    {
        return std::fabs(v) <= small_number;
    }

    template<typename value_type, typename = std::enable_if_t<std::is_floating_point_v<value_type>>>
    constexpr bool near_one(value_type v, value_type small_number = SMALL_NUM<value_type>)
    {
        return near_zero(v - value_type(1));
    }

    template<typename value_type, typename = std::enable_if_t<std::is_floating_point_v<value_type>>>
    constexpr bool near_one_length(value_type v, value_type small_number = SMALL_NUM<value_type>)
    {
        return (v + small_number >= value_type(1)) || (v - small_number <= value_type(-1));
    }

    template<typename n_value_type, typename value_type, typename = std::enable_if_t<std::is_integral_v<n_value_type>>>
    constexpr int floor2(value_type f)
    {
        return static_cast<n_value_type>(f);
    }

    template<typename n_value_type, typename value_type, typename = std::enable_if_t<std::is_integral_v<n_value_type>>>
    constexpr int ceil2(value_type f)
    {
        return static_cast<n_value_type>(std::ceil(f));
    }

    template<typename value_type, typename std::enable_if<!std::is_floating_point_v<value_type>, bool>::type = true>
    constexpr bool is_equal(value_type lhs, value_type rhs)
    {
        return lhs == rhs;
    }

    template<typename value_type, typename std::enable_if<std::is_floating_point_v<value_type>, bool>::type = true>
    constexpr bool is_equal(value_type lhs, value_type rhs)
    {
        return lhs == rhs || near_zero<value_type>(lhs - rhs);
    }

    template<typename value_type>
    constexpr value_type clamp(scaler_t <value_type> value, value_type minValue, scaler_t<value_type> maxValue)
    {
        return value < minValue
            ? minValue
            : (value > maxValue
                ? maxValue
                : value);
    }

    template<typename value_type>
    constexpr value_type saturate(value_type value)
    {
        return clamp(value, value_type(0), value_type(1));
    }

    template<typename value_type>
    constexpr value_type range(scaler_t <value_type> value, value_type maxValue)
    {
        return maxValue < 0
            ? clamp(value, +maxValue, -maxValue)
            : clamp(value, -maxValue, +maxValue);
    }

    template<typename value_type>
    constexpr value_type lerp(value_type l, scaler_t<value_type> r, float f)
    {
        return l + (r - l) * f;
    }

    template<typename value_type>
    constexpr value_type lerp(value_type l, scaler_t<value_type> r, double f)
    {
        return l + (r - l) * f;
    }

    namespace power_impl
    {
        template<int exp, typename value_type, typename enable = std::enable_if_t<exp % 2 != 0>>
        struct term
        {
            static constexpr value_type eval(value_type base)
            {
                return term<exp - 1, value_type, void>::eval(base) * base;
            }
        };

        template<typename value_type>
        struct term<0, value_type, void>
        {
            static constexpr value_type eval(value_type base) { return value_type(1); }
        };

        template<typename value_type>
        struct term<1, value_type, void>
        {
            static constexpr value_type eval(value_type base) { return base; }
        };

        template<typename value_type>
        struct term<2, value_type, void>
        {
            static constexpr value_type eval(value_type base) { return base * base; }
        };

        template<int exp, typename value_type>
        struct term<exp, value_type, std::enable_if_t<exp % 2 == 0>>
        {
            static constexpr value_type eval(value_type base)
            {
                value_type sqrt_value = term<exp / 2, value_type, void>::eval(base);
                return term<2, value_type, void>::eval(sqrt_value);
            }
        };
    }

    template<int exp, typename value_type>
    constexpr value_type power(value_type base)
    {
        return power_impl::term<exp, value_type, void>::eval(base);
    }

    template<typename value_type>
    constexpr value_type square(value_type base)
    {
        return power<2>(base);
    }

    template<typename value_type>
    constexpr value_type inverse_sqrt(value_type v)
    {
        return value_type(1) / sqrt(v);
    }

    template<typename value_type>
    constexpr value_type min2(value_type a, scaler_t<value_type> b)
    {
        return a < b ? a : b;
    }

    template<typename value_type>
    constexpr value_type max2(value_type a, scaler_t<value_type> b)
    {
        return a > b ? a : b;
    }

    template<typename value_type>
    constexpr value_type min3(value_type a, scaler_t<value_type> b, scaler_t<value_type> c)
    {
        return a < b
            ? (a < c ? a : c)
            : (b < c ? b : c);
    }

    template<typename value_type>
    constexpr value_type max3(value_type a, scaler_t<value_type> b, scaler_t<value_type> c)
    {
        return a > b
            ? (a > c ? a : c)
            : (b > c ? b : c);
    }
}
