cmake_minimum_required(VERSION 3.12)

project(GameEngine)

set(GameEngine_IncludeDir ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set(GameEngine_SourceDir ${CMAKE_CURRENT_SOURCE_DIR}/Source)
set(GameEngine_SourceDir_Core ${GameEngine_SourceDir}/Core)
set(GameEngine_SourceDir_Render ${GameEngine_SourceDir}/Render)
set(GameEngine_SourceDir_Scene ${GameEngine_SourceDir}/Scene)

set(GameEngine_HeaderFiles_Interface
    ${GameEngine_IncludeDir}/GEPredefinedMacros.h
    ${GameEngine_IncludeDir}/GEInput.h
    ${GameEngine_IncludeDir}/GEScene.h
    ${GameEngine_IncludeDir}/GERender.h
    ${GameEngine_IncludeDir}/GEInclude.h
)

set(GameEngine_SourceFiles_Root
    ${GameEngine_SourceDir}/GameEngineMain.cpp
)

set(GameEngine_HeaderFiles_Core
    ${GameEngine_SourceDir_Core}/GameEngine.h
)

set(GameEngine_SourceFiles_Core
    ${GameEngine_SourceDir_Core}/GameEngine.cpp
)

set(GameEngine_HeaderFiles_Render
    ${GameEngine_SourceDir_Render}/RenderSystem.h
)
set(GameEngine_SourceFiles_Render
    ${GameEngine_SourceDir_Render}/RenderSystem.cpp
)

set(GameEngine_HeaderFiles_Scene
    ${GameEngine_SourceDir_Scene}/Components.h
    ${GameEngine_SourceDir_Scene}/Scene.h
    ${GameEngine_SourceDir_Scene}/SceneNode.h
)
set(GameEngine_SourceFiles_Scene
    ${GameEngine_SourceDir_Scene}/Components.cpp
    ${GameEngine_SourceDir_Scene}/Scene.cpp
    ${GameEngine_SourceDir_Scene}/SceneNode.cpp
)


set(GameEngine_SourceFiles 
    ${GameEngine_HeaderFiles_Interface}
    ${GameEngine_SourceFiles_Root}
    
    ${FoundationBase_SourceFiles}
    ${FoundationMath_SourceFiles}

    ${GameEngine_HeaderFiles_Core}
    ${GameEngine_SourceFiles_Core}
    ${GameEngine_HeaderFiles_Render}
    ${GameEngine_SourceFiles_Render}
    ${GameEngine_HeaderFiles_Scene}
    ${GameEngine_SourceFiles_Scene}
)

source_group("Foundation/Math"              FILES ${FoundationBase_SourceFiles})
source_group("Foundation/Base"              FILES ${FoundationMath_SourceFiles})
source_group("Engine/Include"               FILES ${GameEngine_HeaderFiles_Interface})
source_group("Engine/Source"                FILES ${GameEngine_SourceFiles_Root})
source_group("Engine/Source/Core/Include"   FILES ${GameEngine_HeaderFiles_Core})
source_group("Engine/Source/Core/Source"    FILES ${GameEngine_SourceFiles_Core})
source_group("Engine/Source/Render/Include" FILES ${GameEngine_HeaderFiles_Render})
source_group("Engine/Source/Render/Source"  FILES ${GameEngine_SourceFiles_Render})
source_group("Engine/Source/Scene/Include" FILES ${GameEngine_HeaderFiles_Scene})
source_group("Engine/Source/Scene/Source"  FILES ${GameEngine_SourceFiles_Scene})



add_library(GameEngine SHARED ${GameEngine_SourceFiles})
target_include_directories(GameEngine PUBLIC "${GameEngine_IncludeDir}" "${CMAKE_SOURCE_DIR}")
target_include_directories(GameEngine PRIVATE "${GameEngine_SourceDir}")

if(MSVC)
    target_link_libraries(GameEngine d3d11.lib dxgi.lib d3dcompiler.lib)
endif(MSVC)

set_target_properties(GameEngine PROPERTIES
    COMPILE_DEFINITIONS "GAME_ENGINE_EXPORT"
    RUNTIME_OUTPUT_DIRECTORY ${GameEngineOutputDirectory}
)
