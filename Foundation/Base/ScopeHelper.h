#pragma once
#include <type_traits>

namespace base
{
    template<class functor_type> class scope_guard;
    template<class functor_type> scope_guard<functor_type> make_scope_guard(functor_type&& callback);

    template<class functor_type>
    class scope_guard
    {
    public:
        scope_guard(functor_type&& callback) : callback(std::move(callback)) { }
        scope_guard(scope_guard&& other) : callback(std::move(other.callback)), dismissed(other.dismissed) { other.dismissed = true; }
        ~scope_guard() { if (!is_dismissed())callback(); }
        void dismiss() { dismissed = true; }
        bool is_dismissed() const { return dismissed; }
        operator bool const () { return !dismissed; }

    private:
        functor_type callback;
        bool dismissed = false;
    };

    template<class functor_type>
    scope_guard<functor_type> make_scope_guard(functor_type&& callback)
    {
        return scope_guard<functor_type>(std::move(callback));
    }

    namespace base_impl
    {
        struct scope_syntax
        {
            template<class functor_type> scope_guard<functor_type> operator<<(functor_type&& callback) { return make_scope_guard<functor_type>(std::forward<functor_type>(callback)); }
            static scope_syntax& on_exit_helper() { static scope_syntax instance; return instance; }
        };
    }
}

#define FOUNDATION_BASE_JOIN_IMPL(string_a, string_b) string_a##string_b
#define FOUNDATION_BASE_JOIN(string_a, string_b) FOUNDATION_BASE_JOIN_IMPL(string_a, string_b)
#define ON_EXIT const auto FOUNDATION_BASE_JOIN(scope_guard_at_line,__LINE__) = base_impl::scope_syntax::on_exit_helper() << [&]()
