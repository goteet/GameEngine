cmake_minimum_required(VERSION 3.12)

project(LitRenderer)


if(MSVC)

set(LitRenderer_SourceFiles
    WinMain.cpp
    
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