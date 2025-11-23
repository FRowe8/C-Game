cmake_minimum_required(VERSION 3.15)
project(QuantumIdle VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(APPLE)
    if(IOS)
        set(PLATFORM_IOS TRUE)
        add_definitions(-DPLATFORM_IOS)
    else()
        set(PLATFORM_MACOS TRUE)
        add_definitions(-DPLATFORM_MACOS)
    endif()
elseif(ANDROID)
    set(PLATFORM_ANDROID TRUE)
    add_definitions(-DPLATFORM_ANDROID)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    add_definitions(-DPLATFORM_LINUX)
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX- /permissive-)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Find packages
find_package(OpenGL REQUIRED)

# SDL2 - cross-platform windowing and input
if(PLATFORM_ANDROID OR PLATFORM_IOS)
    # Mobile platforms - handle differently
    if(PLATFORM_ANDROID)
        find_library(SDL2_LIBRARY SDL2)
    endif()
else()
    # Desktop platforms
    find_package(SDL2 QUIET)
    if(NOT SDL2_FOUND)
        # Try pkg-config
        find_package(PkgConfig)
        if(PkgConfig_FOUND)
            pkg_check_modules(SDL2 REQUIRED sdl2)
        endif()
    endif()
endif()

# Source files
file(GLOB_RECURSE ENGINE_SOURCES
    "src/engine/*.cpp"
    "src/engine/*.c"
)

file(GLOB_RECURSE GAME_SOURCES
    "src/game/*.cpp"
)

# Main executable
if(PLATFORM_ANDROID)
    add_library(${PROJECT_NAME} SHARED
        ${ENGINE_SOURCES}
        ${GAME_SOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${ENGINE_SOURCES}
        ${GAME_SOURCES}
    )
endif()

# Link libraries
if(SDL2_FOUND OR SDL2_LIBRARY)
    if(TARGET SDL2::SDL2)
        target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
    elseif(SDL2_LIBRARIES)
        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    elseif(SDL2_LIBRARY)
        target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
    endif()
endif()

target_link_libraries(${PROJECT_NAME} OpenGL::GL)

# Platform-specific libraries
if(PLATFORM_WINDOWS)
    target_link_libraries(${PROJECT_NAME} winmm)
elseif(PLATFORM_LINUX)
    target_link_libraries(${PROJECT_NAME} pthread dl)
elseif(PLATFORM_MACOS)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

# Copy assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION bin
)
