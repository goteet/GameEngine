#pragma once
#include <type_traits>

template<class PointerType> void SafeDelete(PointerType& pointer) { delete pointer; pointer = nullptr; }
template<class PointerType> void safe_delete(PointerType& pointer) { SafeDelete<PointerType>(pointer); }
template<class PointerType> void SafeDeleteArray(PointerType& pointer) { delete[] pointer;    pointer = nullptr; }
template<class PointerType> void safe_delete_array(PointerType& pointer) { SafeDeleteArray<PointerType>(pointer); }

namespace base_impl
{
    template<class, class = std::void_t<> > struct HasReleaseInterface : std::false_type { };
    template<class, class = std::void_t<> > struct has_release_interface : std::false_type { };
    template<class T> struct HasReleaseInterface<T, std::void_t<decltype(std::declval<T>()->Release())>> : std::true_type { };
    template<class T> struct has_release_interface<T, std::void_t<decltype(std::declval<T>()->release())>> : std::true_type { };
    template<class T> using EnableRelease = std::enable_if_t<HasReleaseInterface<T>::value>;
    template<class T> using enable_release = std::enable_if_t<has_release_interface<T>::value>;
}

template<class InterfaceType, class = base_impl::EnableRelease<InterfaceType>::value>
void SafeRelease(InterfaceType& pointer)
{
    if (pointer != nullptr)
    {
        pointer->Release(); pointer = nullptr;
    }
}

template<class InterfaceType, class = base_impl::enable_release<InterfaceType>::value>
void safe_release(InterfaceType& pointer)
{
    if (pointer != nullptr)
    {
        pointer->release(); pointer = nullptr;
    }
}