#pragma once
#include <string>
#include <vector>

#include "ScopeHelper.h"
#include "Serialization.h"
#include "MemoryHelper.h"

namespace base
{
    struct file_archive : public base_archive
    {
        virtual ~file_archive() { fclose(file); }
        virtual bool is_at_end() sealed override { return file_size > 0 && file_size <= static_cast<uint32_t>(ftell(file)); }
        virtual void destroy() = 0;

        uint32_t size() { return file_size; }

    protected:
        file_archive(FILE* f) : file(f)
        {
            fseek(f, 0, SEEK_END);
            file_size = ftell(f);
            fseek(f, 0, SEEK_SET);
        };
        FILE* file = nullptr;
        uint32_t file_size = 0;

    private:
        file_archive(file_archive& rhs) = delete;
        file_archive(file_archive&& rhs) = delete;
    };


    struct file_archive_read : public file_archive
    {

        virtual bool is_saving() sealed override { return false; }
        virtual base_archive& serialize(void* data, uint32_t size_in_bytes) override { fread(data, 1, size_in_bytes, file); return *this; }
        virtual void destroy() sealed override { delete this; }
        friend file_archive* create_archive_file_read(const std::string& path);

    protected:
        file_archive_read(FILE* f) : file_archive(f) { }
    };

    struct file_archive_write : public file_archive
    {
        virtual bool is_saving() sealed override { return true; }
        virtual base_archive& serialize(void* data, uint32_t size_in_bytes) override
        {
            size_t written_size_in_bytes = fwrite(data, 1, size_in_bytes, file);
            file_size += static_cast<uint32_t>(written_size_in_bytes);
            return *this;
        }
        virtual void destroy() sealed override { delete this; }
        friend file_archive* create_archive_file_write(const std::string& path);

    protected:
        file_archive_write(FILE* f) : file_archive(f) { }
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


    bool read_entire_file(const std::string& path, memory_archive_read& archive)
    {
        file_archive* file_archive = create_archive_file_read(path);
        if (file_archive != nullptr)
        {
            archive.expand_to(file_archive->size());
            *file_archive << archive;
            file_archive->destroy();
            return true;
        }
        else
        {
            return false;
        }
    }
}
