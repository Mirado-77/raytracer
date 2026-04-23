##
## EPITECH PROJECT, 2026
## Raytracer
## File description:
## CMakeLists
##

cmake_minimum_required(VERSION 3.20)

project(raytracer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

# Output binaries and shared libraries to the repo root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)


find_package(SFML 2 REQUIRED COMPONENTS graphics window system)
find_library(LIBCONFIG_LIB config++ REQUIRED)

# core binary
file(GLOB_RECURSE SRC_RAYTRACER CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM SRC_RAYTRACER ${CMAKE_SOURCE_DIR}/src/denoise_tool.cpp)

add_executable(${PROJECT_NAME} ${SRC_RAYTRACER})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBCONFIG_LIB}
    dl
    pthread
    sfml-graphics
    sfml-window
    sfml-system
)
target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)


foreach(CATEGORY primitives materials textures)
    file(GLOB CATEGORY_SRCS CONFIGURE_DEPENDS plugins/src/${CATEGORY}/*.cpp)
    foreach(SRC ${CATEGORY_SRCS})
        get_filename_component(PLUGIN_NAME ${SRC} NAME_WE)
        add_plugin(${PLUGIN_NAME} SRCS ${SRC})
    endforeach()
endforeach()
