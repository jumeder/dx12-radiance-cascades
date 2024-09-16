cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

project(dx12-radiance-cascades VERSION 0.1.0 LANGUAGES C CXX)

add_executable(dx12-radiance-cascades main.cpp application.cpp)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(dx12-radiance-cascades PRIVATE d3d12 glfw)