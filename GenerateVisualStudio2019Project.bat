if not exist build (
    mkdir build
)
cd build
cmake .. -A x64 -G"Visual Studio 16 2019"
cd ..
pause