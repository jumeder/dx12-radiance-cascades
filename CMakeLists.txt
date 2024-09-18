cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(${CMAKE_SOURCE_DIR}/shaders.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)

project(dx12-radiance-cascades VERSION 0.1.0 LANGUAGES C CXX)

add_shader(Drawing.vs.hlsl vs_6_0 Drawing.vs.h DrawingVS)
add_shader(Drawing.ps.hlsl ps_6_0 Drawing.ps.h DrawingPS)
add_shader(Raytracing.hlsl lib_6_3 Raytracing.h Raytracing)
add_shader(CascadeTracing.hlsl lib_6_3 CascadeTracing.h CascadeTracing)
add_shader(DebugCascades.vs.hlsl vs_6_0 DebugCascades.vs.h DebugCascadesVS)
add_shader(DebugCascades.ps.hlsl ps_6_0 DebugCascades.ps.h DebugCascadesPS)

add_executable(dx12-radiance-cascades
    main.cpp 
    Application.cpp 
    Renderer.cpp 
    Device.cpp 
    Model.cpp
    RadianceCascades.cpp
    Scene.cpp
    Drawing.vs.h
    Drawing.ps.h
    RayTracing.h
    CascadeTracing.h
    DebugCascades.vs.h
    DebugCascades.ps.h
)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
target_link_libraries(dx12-radiance-cascades PRIVATE dxgi d3d12 glfw assimp::assimp)