cmake_minimum_required(VERSION 3.12)

set(GfxInterface_Include ${CMAKE_CURRENT_SOURCE_DIR}/../../Include/GfxInterface.h)

set(GfxInterfaceVulkan_Inlcude
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanInclude.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanCommandQueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanDeviceContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanDeviceResource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanGraphicDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanGraphicModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanPipelineState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanShader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanSwapChain.h
)

set(GfxInterfaceVulkan_Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanCommandQueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanDeviceContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanGraphicDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanGraphicModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanPipelineState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanShader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanSwapChain.cpp
)

set(GfxInterfaceVulkan_SourceFiles 
    ${GfxInterface_Include}
    ${GfxInterfaceVulkan_Inlcude}
    ${GfxInterfaceVulkan_Source}
)

source_group("Include"          FILES ${GfxInterface_Include})
source_group("Source/Include"   FILES ${GfxInterfaceVulkan_Inlcude})
source_group("Source/Source"    FILES ${GfxInterfaceVulkan_Source})
add_library(GfxInterfaceVulkan SHARED ${GfxInterfaceVulkan_SourceFiles} )

find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)
target_include_directories(GfxInterfaceVulkan PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../Include")
target_include_directories(GfxInterfaceVulkan PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(GfxInterfaceVulkan Vulkan::Vulkan
    debug       ${Vulkan_INCLUDE_DIRS}/../Lib/shaderc_shared.lib
    optimized   Vulkan::shaderc_combined)


set_target_properties(GfxInterfaceVulkan PROPERTIES
    COMPILE_DEFINITIONS "GFX_RHI_EXPORT;UNICODE;_UNICODE"
    FOLDER "Engine/GfxInterface"
    RUNTIME_OUTPUT_DIRECTORY ${GameEngineOutputDirectory}
)
