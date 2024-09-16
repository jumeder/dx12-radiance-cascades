cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)

project(dx12-radiance-cascades VERSION 0.1.0 LANGUAGES C CXX)

find_program(DXC dxc.exe)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/Drawing.vs.h
    MAIN_DEPENDENCY Drawing.vs.hlsl
    COMMAND ${DXC} /T vs_6_0 /Fh ${CMAKE_SOURCE_DIR}/Drawing.vs.h /Vn DrawingVS ${CMAKE_SOURCE_DIR}/Drawing.vs.hlsl
    VERBATIM
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/Drawing.ps.h
    DEPENDS Drawing.ps.hlsl
    COMMAND ${DXC} /T ps_6_0 /Fh ${CMAKE_SOURCE_DIR}/Drawing.ps.h /Vn DrawingPS ${CMAKE_SOURCE_DIR}/Drawing.ps.hlsl
    VERBATIM
)

add_executable(dx12-radiance-cascades 
    main.cpp 
    Application.cpp 
    Renderer.cpp 
    Device.cpp 
    Model.cpp
    Drawing.vs.h
    Drawing.ps.h
)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
target_link_libraries(dx12-radiance-cascades PRIVATE dxgi d3d12 glfw assimp::assimp)