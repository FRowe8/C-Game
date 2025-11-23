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
set(SDL2_FOUND FALSE)

if(PLATFORM_WINDOWS)
    # Windows: Look for SDL2 in common locations
    if(DEFINED ENV{SDL2_DIR})
        set(SDL2_DIR $ENV{SDL2_DIR})
    endif()

    # Search paths
    set(SDL2_SEARCH_PATHS
        ${SDL2_DIR}
        "C:/SDL2"
        "C:/Program Files/SDL2"
        "C:/Program Files (x86)/SDL2"
        "${CMAKE_SOURCE_DIR}/external/SDL2"
    )

    # Find include directory
    find_path(SDL2_INCLUDE_DIR SDL.h
        PATH_SUFFIXES include SDL2
        PATHS ${SDL2_SEARCH_PATHS}
    )

    # Find library
    find_library(SDL2_LIBRARY
        NAMES SDL2
        PATH_SUFFIXES lib lib/x64 lib/x86
        PATHS ${SDL2_SEARCH_PATHS}
    )

    if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
        set(SDL2_FOUND TRUE)
        # Don't link SDL2main - we use SDL_MAIN_HANDLED for MinGW compatibility
        set(SDL2_LIBRARIES ${SDL2_LIBRARY})
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
        message(STATUS "Found SDL2: ${SDL2_LIBRARY}")
    else()
        message(WARNING "SDL2 not found! Please set SDL2_DIR or download from https://github.com/libsdl-org/SDL/releases")
        message(WARNING "See WINDOWS_BUILD.md for instructions")
    endif()

elseif(PLATFORM_ANDROID OR PLATFORM_IOS)
    # Mobile platforms
    if(PLATFORM_ANDROID)
        find_library(SDL2_LIBRARY SDL2)
        set(SDL2_FOUND TRUE)
    endif()
else()
    # Linux/macOS: Try find_package first, then pkg-config
    find_package(SDL2 QUIET)
    if(NOT SDL2_FOUND)
        find_package(PkgConfig)
        if(PkgConfig_FOUND)
            pkg_check_modules(SDL2 sdl2)
            if(SDL2_FOUND)
                message(STATUS "Found SDL2 via pkg-config")
            endif()
        endif()
    endif()

    if(NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 not found! Install with: sudo apt-get install libsdl2-dev (Ubuntu) or brew install sdl2 (macOS)")
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
if(SDL2_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

    if(TARGET SDL2::SDL2)
        target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
    else()
        target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    endif()

    message(STATUS "SDL2 include dirs: ${SDL2_INCLUDE_DIRS}")
    message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
else()
    message(FATAL_ERROR "SDL2 is required but was not found!")
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
