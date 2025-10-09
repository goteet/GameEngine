#pragma once
#include <string>
#include <vector>

#include "ScopeHelper.h"
#include "Serialization.h"

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
        FILE* f = nullptr;
        fopen_s(&f, path.c_str(), "rb");
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

    struct file_archive : public base_archive
    {
        virtual ~file_archive() { fclose(file); }
        virtual bool is_at_end() { return false; }

        void destroy() { delete this; }

    protected:
        file_archive(FILE* f) : file(f) { };
        FILE* file = nullptr;

        file_archive(file_archive& rhs) = delete;
        file_archive(file_archive&& rhs) = delete;
    };


    struct file_archive_read : public file_archive
    {
        virtual bool is_saving() sealed override { return false; }
        virtual base_archive& serialize(void* data, uint32_t length) override { fread(data, 1, length, file); return *this; }

        friend file_archive* create_archive_file_read(const std::string& path);

    protected:
        file_archive_read(FILE* f) : file_archive(f) { }
    };

    struct file_archive_write : public file_archive
    {
        virtual bool is_saving() sealed override { return true; }
        virtual base_archive& serialize(void* data, uint32_t length) override { fwrite(data, 1, length, file); return *this; }
        virtual bool is_at_end() override { return file_length > 0 && file_length <= ftell(file); }

        friend file_archive* create_archive_file_write(const std::string& path);

    protected:
        file_archive_write(FILE* f) : file_archive(f)
        {
            fseek(f, 0, SEEK_END);
            file_length = ftell(f);
            fseek(f, 0, SEEK_SET);
        }

    private:
        size_t file_length = 0;
    };


    file_archive* create_archive_file_read(const std::string& path)
    {
        FILE* f = nullptr;
        errno_t err = fopen_s(&f, path.c_str(), "rb");
        return (err == 0) ? new file_archive_read(f) : nullptr;
    }

    file_archive* create_archive_file_write(const std::string& path)
    {
        FILE* f = nullptr;
        errno_t err = fopen_s(&f, path.c_str(), "wb");
        return (err == 0) ? new file_archive_write(f) : nullptr;
    }


}
