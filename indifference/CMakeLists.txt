cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(Indifference LANGUAGES C CXX)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SDLIMAGE_INSTALL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDLIMAGE_BACKEND_STB ON CACHE BOOL "" FORCE)
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_GIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_SVG OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.10
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-3.2.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3 SDL3_image)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(indifference indifferenceDemo.cpp)
target_compile_features(indifference PRIVATE cxx_std_20)
target_compile_definitions(indifference PRIVATE INDIFFERENCE_NO_AUDIO)
target_include_directories(indifference PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glm-1.0.1-light")

if(TARGET SDL3::SDL3-static)
    set(SDL3_GAME_TARGET SDL3::SDL3-static)
else()
    set(SDL3_GAME_TARGET SDL3::SDL3)
endif()
target_link_libraries(indifference PRIVATE ${SDL3_GAME_TARGET} SDL3_image::SDL3_image)

option(INDIFFERENCE_EMBED_ASSETS "Embed game assets in the executable" OFF)
if(INDIFFERENCE_EMBED_ASSETS)
    file(GLOB_RECURSE EMBEDDED_ASSET_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/*"
    )
    set(EMBEDDED_ASSETS_HEADER "${CMAKE_CURRENT_BINARY_DIR}/assets_embedded.h")
    add_custom_command(
        OUTPUT "${EMBEDDED_ASSETS_HEADER}"
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embed_assets.py"
            --assets "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            --output "${EMBEDDED_ASSETS_HEADER}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embed_assets.py" ${EMBEDDED_ASSET_FILES}
        VERBATIM
    )
    add_custom_target(embed_assets DEPENDS "${EMBEDDED_ASSETS_HEADER}")
    add_dependencies(indifference embed_assets)
    target_compile_definitions(indifference PRIVATE EMBED_ASSETS)
    target_include_directories(indifference PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endif()

if(EMSCRIPTEN)
    target_link_options(indifference PRIVATE
        "-sASYNCIFY"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
        "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
    )
    set_target_properties(indifference PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/site"
    )
elseif(WIN32)
    set_target_properties(indifference PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
