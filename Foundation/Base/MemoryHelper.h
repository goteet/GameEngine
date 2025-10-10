#pragma once
#include <type_traits>
#include "Serialization.h"

template<class PointerType> void SafeDelete(PointerType& pointer) { delete pointer; pointer = nullptr; }
template<class PointerType> void safe_delete(PointerType& pointer) { SafeDelete<PointerType>(pointer); }
template<class PointerType> void SafeDeleteArray(PointerType& pointer) { delete[] pointer;    pointer = nullptr; }
template<class PointerType> void safe_delete_array(PointerType& pointer) { SafeDeleteArray<PointerType>(pointer); }


namespace base
{
    namespace base_impl
    {
        template<class, class = std::void_t<> > struct HasReleaseInterface : std::false_type { };
        template<class, class = std::void_t<> > struct has_release_interface : std::false_type { };
        template<class T> struct HasReleaseInterface<T, std::void_t<decltype(std::declval<T>()->Release())>> : std::true_type { };
        template<class T> struct has_release_interface<T, std::void_t<decltype(std::declval<T>()->release())>> : std::true_type { };
        template<class T> using EnableRelease = std::enable_if_t<HasReleaseInterface<T>::value>;
        template<class T> using enable_release = std::enable_if_t<has_release_interface<T>::value>;
    }
}

template<typename InterfaceType, typename = base::base_impl::EnableRelease<InterfaceType>>
void SafeRelease(InterfaceType& pointer)
{
    if (pointer != nullptr)
    {
        pointer->Release();
        pointer = nullptr;
    }
}

template<typename InterfaceType, typename = base::base_impl::enable_release<InterfaceType>>
void safe_release(InterfaceType& pointer)
{
    if (pointer != nullptr)
    {
        pointer->release();
        pointer = nullptr;
    }
}

namespace base
{
    struct memory_archive : public base_archive, public base_serializable_object
    {
        virtual ~memory_archive() { release(); }
        virtual bool is_at_end() sealed override { return offset >= size; }
        virtual void on_serialize(base_archive& archive) sealed override
        {
            archive << size;
            archive.serialize(buffer, size);
        }

    protected:
        memory_archive(uint32_t size_in_bytes) : size(size_in_bytes), buffer(new uint8_t[size]), release_buffer(true) { }
        memory_archive(uint8_t* external_buffer, uint32_t size_in_byte) : size(size_in_byte), buffer(external_buffer), release_buffer(false) { }
        memory_archive(memory_archive& rhs) : size(rhs.size), offset(rhs.offset), buffer(rhs.buffer), release_buffer(rhs.release_buffer) { if (release_buffer) rhs.buffer = nullptr; }

        void release()
        {
            if (release_buffer)
                safe_delete_array(buffer);
            buffer = nullptr;
        }

        bool release_buffer = false;
        uint32_t size = 0;
        uint32_t offset = 0;
        uint8_t* buffer = nullptr;
    };

    struct memory_archive_read : public memory_archive
    {
        memory_archive_read(uint8_t* buffer, uint32_t size_in_bytes) : memory_archive(buffer, size_in_bytes) { }
        memory_archive_read(uint32_t size_in_bytes) : memory_archive(size_in_bytes) { }
        virtual bool is_saving() sealed override { return false; }
        virtual base_archive& serialize(void* data, uint32_t size_in_bytes) sealed override
        {
            if (offset + size_in_bytes > size)
            {
                size_in_bytes = size - offset;
            }

            if (size_in_bytes > 0)
            {
                std::memcpy(data, buffer + offset, size_in_bytes);
                offset += size_in_bytes;
            }
            return *this;
        }

        void expand_to(uint32_t size_in_bytes)
        {
            if (size_in_bytes > size)
            {
                release();
                release_buffer = true;
                buffer = new uint8_t[size_in_bytes];
                size = size_in_bytes;
                offset = 0;
            }
        }
    };

    struct memory_archive_write : public memory_archive
    {
        memory_archive_write(uint32_t size_in_bytes) : memory_archive(size_in_bytes) { }
        memory_archive_write(uint8_t* external_buffer, uint32_t size_in_bytes) : memory_archive(external_buffer, size_in_bytes) { }

        virtual bool is_saving() sealed override { return true; }
        virtual base_archive& serialize(void* data, uint32_t size_in_bytes) sealed override
        {
            if (offset + size_in_bytes > size)
            {
                size_in_bytes = size - offset;
            }

            if (size_in_bytes > 0)
            {
                std::memcpy(buffer + offset, data, size_in_bytes);
                offset += size_in_bytes;
            }
            return *this;
        }
    };

    template<uint32_t N>
    struct stack_memory_archive_write : memory_archive_write
    {
        static const uint32_t size = N;
        stack_memory_archive_write() : memory_archive_write(stack_buffer, N) { }
        uint8_t stack_buffer[N];
    };

    template<uint32_t N>
    using stack_memory_archive = stack_memory_archive_write<N>;

    //template<uint32_t N>
    //struct stack_memory_archive_read : memory_archive_read
    //{
    //    static const uint32_t size = N;
    //    stack_memory_archive_read() : memory_archive_read(stack_buffer, N) { }
    //    uint8_t stack_buffer[N];
    //};
}
