cmake_minimum_required(VERSION 3.0)
project(ray)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

find_package(glm REQUIRED)
find_package(OpenCV REQUIRED)

add_definitions(-DRAY_TEXTURES_DIRECTORY="${CMAKE_SOURCE_DIR}/textures/")

add_subdirectory(src)
