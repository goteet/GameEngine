cmake_minimum_required(VERSION 3.12)

project(SimpleGame)


if(MSVC)

set(SimpleGame_SourceFiles
    WinMain.cpp
    SimpleTimer.h
    
#   经过了多种尝试，发现直接加入项目里是最方便的。
    ${FoundationBase_SourceFiles}
    ${FoundationMath_SourceFiles}
)

source_group(Foundation/Base FILES ${FoundationBase_SourceFiles})
source_group(Foundation/Math FILES ${FoundationMath_SourceFiles})

add_executable(SimpleGame WIN32 ${SimpleGame_SourceFiles})
target_include_directories(SimpleGame PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(SimpleGame GameEngine)
set_target_properties(SimpleGame PROPERTIES
    COMPILE_DEFINITIONS "UNICODE;_UNICODE"
    RUNTIME_OUTPUT_DIRECTORY ${GameEngineOutputDirectory}
    VS_DEBUGGER_WORKING_DIRECTORY ${GameEngineOutputDirectory}
)

endif(MSVC)
