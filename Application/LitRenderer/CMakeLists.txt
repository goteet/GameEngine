cmake_minimum_required(VERSION 3.12)

project(LitRenderer)


if(MSVC)

set(LitRenderer_SourceFiles
    ${CMAKE_CURRENT_SOURCE_DIR}/Integrator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Integrator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LDRFilm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LDRFilm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LitRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LitRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WinMain.cpp
    
#   经过了多种尝试，发现直接加入项目里是最方便的。
    ${FoundationBase_SourceFiles}
    ${FoundationMath_SourceFiles}
)

source_group(Foundation/Base FILES ${FoundationBase_SourceFiles})
source_group(Foundation/Math FILES ${FoundationMath_SourceFiles})

add_executable(LitRenderer WIN32 ${LitRenderer_SourceFiles})
target_include_directories(LitRenderer PRIVATE ${CMAKE_SOURCE_DIR})
set_target_properties(LitRenderer PROPERTIES COMPILE_DEFINITIONS "UNICODE;_UNICODE")
set_target_properties(LitRenderer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

endif(MSVC)
