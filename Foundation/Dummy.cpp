#include "Vector.h"

int MathLibraryDummy()
{
    math::vector3<long> x;

    x = x / 10;

    return 0;

}


#include "Serialization.h"
#include "FileHelper.h"


struct A
{
    int a = 0;

    A(int x = 5) :a(x) {}
    A(A&) = default;
};

int serialization_dummy()
{
    std::vector<A> saved_array = {A(), A(), A(), A()};
    int length = static_cast<int>(saved_array.size());

    std::string file_path = std::string("e://test.txt");
    base::file_archive* archive = base::create_archive_file_write(file_path);

    A a;
    *archive << a;

    archive->destroy();

    return 0;
}
