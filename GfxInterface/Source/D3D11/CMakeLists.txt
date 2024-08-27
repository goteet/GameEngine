cmake_minimum_required(VERSION 3.12)

set(GfxInterface_Include ${CMAKE_CURRENT_SOURCE_DIR}/../../Include/GfxInterface.h)

set(GfxInterfaceD3D11_Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11Include.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11Buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11CommandQueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11DeviceContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11GraphicDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11GraphicModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11PipelineState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11SamplerState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11Shader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11SwapChain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/D3D11Texture.h
)

set(GfxInterfaceD3D11_Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11Buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11CommandQueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11DeviceContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11GraphicDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11GraphicModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11PipelineState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11SamplerState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11SwapChain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/D3D11Texture.cpp

)

set(GfxInterfaceD3D11_SourceFiles 
    ${GfxInterface_Include}
    ${GfxInterfaceD3D11_Include}
    ${GfxInterfaceD3D11_Source}
)

source_group("Include"          FILES ${GfxInterface_Include})
source_group("Source/Include"   FILES ${GfxInterfaceD3D11_Include})
source_group("Source/Source"    FILES ${GfxInterfaceD3D11_Source})
add_library(GfxInterfaceD3D11 SHARED ${GfxInterfaceD3D11_SourceFiles} )

target_include_directories(GfxInterfaceD3D11 PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/../../Include")
target_include_directories(GfxInterfaceD3D11 PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

if(MSVC)
    target_link_libraries(GfxInterfaceD3D11
        d3d11.lib
        dxguid.lib
        dxgi.lib
        d3dcompiler.lib
    )
endif(MSVC)

set_target_properties(GfxInterfaceD3D11 PROPERTIES
    COMPILE_DEFINITIONS "GFX_RHI_EXPORT;UNICODE;_UNICODE"
    FOLDER "Engine/GfxInterface"
    RUNTIME_OUTPUT_DIRECTORY ${GameEngineOutputDirectory}
)
