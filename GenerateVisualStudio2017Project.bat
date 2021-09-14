if not exist build (
    mkdir build
)
cd build
cmake .. -A x64 -G"Visual Studio 15 2017"