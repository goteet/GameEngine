#pragma once
#include <type_traits>
#include <vector>
#include <string>
#include <map>

namespace base
{
    struct base_archive
    {
        virtual ~base_archive() = default;
        virtual bool is_at_end() = 0;
        virtual bool is_saving() = 0;
        virtual bool is_loading() { return !is_saving(); }
        virtual base_archive& serialize(void* data, uint32_t size_in_bytes) = 0;
    };

    struct base_serializable_object
    {
        virtual ~base_serializable_object() = default;
        virtual void on_serialize(base_archive& archive) = 0;

        friend inline base_archive& operator<<(base_archive& archive, base_serializable_object& v)
        {
            v.on_serialize(archive); return archive;
        }
    };

    template<typename T, typename std::enable_if_t<std::is_trivially_copyable_v<T>, bool> = true>
    inline base_archive& operator<<(base_archive& archive, T& v)
    {
        return archive.serialize(reinterpret_cast<void*>(&v), sizeof(T));
    }

    template<typename T, uint32_t N, std::enable_if_t<std::is_trivially_copyable_v<T>, bool> = true>
    inline base_archive& operator<<(base_archive& archive, T(&arr)[N])
    {
        uint32_t array_length = N;
        archive << array_length;

        if (archive.is_loading())
        {
            array_length = array_length > N ? N : array_length;
        }

        archive.serialize(arr, sizeof(T) * array_length);
        return archive;
    }

    template<typename T, uint32_t N, std::enable_if_t<!std::is_trivially_copyable_v<T>, bool> = true>
    inline base_archive& operator<<(base_archive& archive, T(&arr)[N])
    {
        uint32_t array_length = N;
        archive << array_length;

        if (archive.is_loading())
        {
            array_length = array_length > N ? N : array_length;
        }

        for (uint32_t index = 0; index < array_length; index++)
        {
            archive << arr[index];
        }
        return archive;
    }

    inline base_archive& operator<<(base_archive& archive, std::string& str)
    {
        uint32_t str_length = static_cast<uint32_t>(str.size());
        archive << str_length;
        if (archive.is_loading())
        {
            str.resize(str_length);
        }

        for (uint32_t index = 0; index < str_length; index++)
        {
            archive << str[index];
        }
        return archive;
    }

    template<typename T>
    inline base_archive& operator<< (base_archive& archive, std::vector<T>& arr)
    {
        uint32_t array_length = static_cast<uint32_t>(arr.size());
        archive << array_length;
        if(archive.is_loading())
        {
            arr.resize(array_length);
        }

        for (uint32_t index = 0; index < array_length; index++)
        {
            archive << arr[index];
        }

        return archive;
    }

    template<typename K, typename V>
    inline base_archive& operator<< (base_archive& archive, std::map<K, V>& m)
    {
        uint32_t length = static_cast<uint32_t>(m.size());
        archive << length;

        if (archive.is_saving())
        {
            for (std::pair<const K, V>& pair : m)
            {
                K key = pair.first;
                V& value = pair.second;
                archive <<key <<value;
            }
        }
        else
        {
            K key; V value;
            for (uint32_t index = 0; index < length; index++)
            {
                archive <<key <<value;
                m.emplace(std::pair<K, V>(key, value));
            }
        }

        return archive;
    }

}
