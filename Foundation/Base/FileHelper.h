#pragma once
#include <string>
#include <vector>
#include "ScopeHelper.h"

namespace base
{
    struct file_buffer
    {
        std::vector<char> data;

        file_buffer() = default;
        file_buffer(const file_buffer &o) : data(o.data) { }
        file_buffer(file_buffer && o) : data(std::move(o.data)) { }
        file_buffer& operator=(const file_buffer &o) { data = o.data; return *this; }
        file_buffer& operator=(file_buffer &&o) { data = std::move(o.data); return *this; }
        char* data_ptr() { return&(data[0]); }
        const char* data_ptr() const { return&(data[0]); }
        operator bool() const { return data.size() > 0; }
        size_t length() const { return data.size(); }
    };

    bool read_entire_file(const std::string& path, file_buffer& buffer)
    {
        FILE* f = fopen(path.c_str(), "rb");
        if (f != nullptr)
        {
            ON_EXIT{ fclose(f); };

            fseek(f, 0, SEEK_END);
            size_t file_length = ftell(f);


            if (file_length > 0)
            {
                fseek(f, 0, SEEK_SET);
                buffer.data.resize(file_length);
                file_length = static_cast<uint32_t>(fread(buffer.data_ptr(), 1, file_length, f));
                if (file_length < buffer.length())
                {
                    buffer.data.resize(file_length);
                }
                return true;
            }

        }
        return false;
    }
}
