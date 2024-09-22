cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(${CMAKE_SOURCE_DIR}/shaders.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR}/headers)

project(dx12-radiance-cascades VERSION 0.1.0 LANGUAGES C CXX)

add_shader(shaders/Drawing.vs.hlsl vs_6_0 generated/Drawing.vs.h DrawingVS)
add_shader(shaders/Drawing.ps.hlsl ps_6_0 generated/Drawing.ps.h DrawingPS)
add_shader(shaders/CascadeTracing.hlsl lib_6_3 generated/CascadeTracing.h CascadeTracing)
add_shader(shaders/CascadeAccumulation.hlsl cs_6_0 generated/CascadeAccumulation.h CascadeAccumulation)
add_shader(shaders/DebugCascades.vs.hlsl vs_6_0 generated/DebugCascades.vs.h DebugCascadesVS)
add_shader(shaders/DebugCascades.ps.hlsl ps_6_0 generated/DebugCascades.ps.h DebugCascadesPS)

add_executable(dx12-radiance-cascades
    sources/main.cpp 
    sources/Application.cpp 
    sources/Renderer.cpp 
    sources/Device.cpp 
    sources/Model.cpp
    sources/RadianceCascades.cpp
    sources/Scene.cpp
    generated/Drawing.vs.h
    generated/Drawing.ps.h
    generated/CascadeTracing.h
    generated/CascadeAccumulation.h
    generated/DebugCascades.vs.h
    generated/DebugCascades.ps.h
)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
target_link_libraries(dx12-radiance-cascades PRIVATE dxgi d3d12 glfw assimp::assimp)