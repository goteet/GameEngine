cmake_minimum_required(VERSION 3.12)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(GameEngine)

set(GameEngineOutputDirectory ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_subdirectory(Foundation)
add_subdirectory(GfxInterface/Source/D3D11)
add_subdirectory(GfxInterface/Source/Vulkan)
add_subdirectory(Engine)
add_subdirectory(Application/LitRenderer)
add_subdirectory(Application/SimpleGame)
add_subdirectory(Application/MISTestbed)
