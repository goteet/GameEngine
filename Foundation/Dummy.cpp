#include "Vector.h"

int MathLibraryDummy()
{
    math::vector3<long> x;

    x = x / 10;

    return 0;

}


#include "Serialization.h"
#include "FileHelper.h"
#include "MemoryHelper.h"


struct TrivialA
{
    int a = 0;

    TrivialA(int x = 5) :a(x) {}
};
struct NonTrivialB : public base::base_serializable_object
{
    int b = 0;
    NonTrivialB(int x = 5) :b(x) {}
    NonTrivialB(NonTrivialB&) { }

    virtual void on_serialize(base::base_archive& ar)
    {
        ar << b;
    }
};

int serialization_dummy()
{
    using namespace base;

    constexpr bool is_trivial_copyable_b = std::is_trivially_copyable_v<NonTrivialB>;
    constexpr bool is_trivial_copyable_barr = std::is_trivially_copyable_v<NonTrivialB[5]>;

    NonTrivialB array_b_write[5] = { 2,4,6,8,10 };
    float array_c_write[5] = { 1.1f, 2.2f, 3.3f, 4.4f, 5.5f };
    TrivialA a(9);
    std::vector<TrivialA> array_a = { TrivialA(1), TrivialA(3), TrivialA(5), TrivialA(7) };
    std::map<int, std::string> dictionary = { {-1, "hello"}, {-2, "world"} };

    std::string file_path = std::string("e://test.txt");

    base::stack_memory_archive_write<256> stack_archive;
    {
        base_archive& archive = stack_archive;
        archive << a << array_a << file_path << array_b_write << dictionary << array_c_write;

        base::file_archive* file_archive = base::create_archive_file_write(file_path);
        ON_EXIT{ if (file_archive != nullptr) file_archive->destroy(); };

        *file_archive << stack_archive;
    }


    {
        TrivialA b;
        std::vector<TrivialA> array_a_read;
        std::string string_read;
        NonTrivialB array_b_read[5];
        float array_c_write[5];
        dictionary.clear();

        memory_archive_read memory_archive(256);
        if (read_entire_file(file_path, memory_archive))
        {
            base_archive& archive = memory_archive;

            archive << b << array_a_read << string_read << array_b_read << dictionary << array_c_write;
        }
    }

    return 0;
}

/*
int main()
{
    return serialization_dummy();
}
//*/
