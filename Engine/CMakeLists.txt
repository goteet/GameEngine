cmake_minimum_required(VERSION 3.12)

project(GameEngine)

set(GameEngine_InterIncludeFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/GEPredefinedMacros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/GEInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/GEScene.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/GERender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/GEInclude.h
)

set(GameEngine_InterSourceFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GameEngineMain.cpp
)

set(GameEngine_SourceFiles 
    ${GameEngine_InterIncludeFiles}
    ${GameEngine_InterSourceFiles}
    
    ${FoundationBase_SourceFiles}
    ${FoundationMath_SourceFiles}
)

source_group(Foundation/Math FILES ${FoundationBase_SourceFiles})
source_group(Foundation/Base FILES ${FoundationMath_SourceFiles})
source_group(Engine/Include FILES ${GameEngine_InterIncludeFiles})
source_group(Engine/Source FILES ${GameEngine_InterSourceFiles})

add_library(GameEngine SHARED ${GameEngine_SourceFiles} )
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include" "${CMAKE_SOURCE_DIR}")

if(MSVC)
    target_link_libraries(GameEngine d3d11.lib dxgi.lib d3dcompiler.lib)
endif(MSVC)

set_target_properties(GameEngine PROPERTIES COMPILE_DEFINITIONS "GAME_ENGINE_EXPORT")
