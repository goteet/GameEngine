cmake_minimum_required(VERSION 3.12)

set(GfxInterface_Include ${CMAKE_CURRENT_SOURCE_DIR}/../../Include/GfxInterface.h)

set(GfxInterfaceVulkan_Inlcude
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanInclude.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanGraphicDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/VulkanGraphicModule.h
)

set(GfxInterfaceVulkan_Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanGraphicDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanGraphicModule.cpp
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

find_package(Vulkan REQUIRED)
target_include_directories(GfxInterfaceVulkan PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../Include")
target_include_directories(GfxInterfaceVulkan PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(GfxInterfaceVulkan ${Vulkan_LIBRARIES})

set_target_properties(GfxInterfaceVulkan PROPERTIES
    COMPILE_DEFINITIONS "GFX_RHI_EXPORT;UNICODE;_UNICODE"
    FOLDER "Engine/GfxInterface"
    RUNTIME_OUTPUT_DIRECTORY ${GameEngineOutputDirectory}
)
