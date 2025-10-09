cmake_minimum_required(VERSION 3.12)

set(Base_InterSourceFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/Base/FileHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Base/MemoryHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Base/Serialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Base/ScopeHelper.h
)
set(Math_InterSourceFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/PredefinedConstantValues.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/HelperFunctions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/Vector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/Rotation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/Matrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/Geometry.h
)

set(FoundationLibrary_SourceFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/Dummy.cpp
    
    ${Math_InterSourceFiles}
    ${Base_InterSourceFiles}
)

set(FoundationBase_SourceFiles ${Base_InterSourceFiles} PARENT_SCOPE)
set(FoundationMath_SourceFiles ${Math_InterSourceFiles} PARENT_SCOPE)

source_group(Math FILES ${Math_InterSourceFiles})
source_group(Base FILES ${Base_InterSourceFiles})
add_library(FoundationLibrary ${FoundationLibrary_SourceFiles} )
target_include_directories(FoundationLibrary PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Base")
target_include_directories(FoundationLibrary PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Math")

set_target_properties(FoundationLibrary PROPERTIES
    FOLDER "Engine"
)
