if not exist ProjectDirectory (
    mkdir ProjectDirectory
)
cd ProjectDirectory
cmake .. -A x64 -G"Visual Studio 17 2022"