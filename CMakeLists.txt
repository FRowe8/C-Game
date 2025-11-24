cmake_minimum_required(VERSION 3.15)
project(QuantumIdle VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(EMSCRIPTEN OR PLATFORM_WEB)
    set(PLATFORM_WEB TRUE)
    add_definitions(-DPLATFORM_WEB)
    message(STATUS "Building for Web (Emscripten)")
elseif(WIN32)
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

    # MinGW specific: Use console subsystem to allow regular main() entry point
    if(WIN32)
        add_link_options(-mconsole)
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
# Try vcpkg first (if using CLion with vcpkg integration)
find_package(SDL2 CONFIG QUIET)

if(SDL2_FOUND)
    message(STATUS "Found SDL2 via vcpkg/find_package")
    # vcpkg provides SDL2::SDL2 and SDL2::SDL2main targets
    set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2)
    set(SDL2_FOUND TRUE)
endif()

# If vcpkg didn't find it, use platform-specific methods
if(NOT SDL2_FOUND AND PLATFORM_WINDOWS)
    # --- FIX: Point specifically to the 64-bit MinGW folder ---
    set(SDL2_ROOT_PATH "C:/SDL2/x86_64-w64-mingw32")

    # 1. Set Include Paths explicitly
    set(SDL2_INCLUDE_DIR "${SDL2_ROOT_PATH}/include/SDL2")

    # 2. Set Library Paths explicitly
    set(SDL2_LIBRARY "${SDL2_ROOT_PATH}/lib/libSDL2.dll.a")
    set(SDL2MAIN_LIBRARY "${SDL2_ROOT_PATH}/lib/libSDL2main.a")

    # 3. Verify they exist to prevent vague errors
    if(EXISTS "${SDL2_INCLUDE_DIR}" AND EXISTS "${SDL2_LIBRARY}")
        set(SDL2_FOUND TRUE)
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

        # Link Order is crucial for MinGW: SDL2main -> SDL2
        set(SDL2_LIBRARIES ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY})

        message(STATUS "SUCCESS: Found SDL2 at ${SDL2_ROOT_PATH}")
        message(STATUS "  Include: ${SDL2_INCLUDE_DIR}")
        message(STATUS "  Libraries: ${SDL2_LIBRARIES}")
    else()
        message(FATAL_ERROR "Could not find SDL2 at ${SDL2_ROOT_PATH}. \nPlease check that C:/SDL2/x86_64-w64-mingw32 exists.\nSee MINGW_BUILD.md for instructions.")
    endif()

elseif(PLATFORM_WEB)
    # Emscripten has SDL2 built-in
    set(SDL2_FOUND TRUE)
    message(STATUS "Using Emscripten's built-in SDL2")

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

# Link libraries - ORDER MATTERS for MinGW!
# Correct order for MinGW: mingw32 -> SDL2main -> SDL2 -> winmm -> opengl32

if(PLATFORM_WINDOWS)
    if(SDL2_FOUND)
        # 1. Link Mingw32 FIRST (required for proper WinMain handling)
        target_link_libraries(${PROJECT_NAME} mingw32)

        # 2. Include Directories
        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

        # 3. Link SDL2Main and SDL2 (in this order!)
        target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    else()
        message(FATAL_ERROR "SDL2 is required but was not found!")
    endif()

    # 4. Windows system libraries
    target_link_libraries(${PROJECT_NAME} winmm imm32 version setupapi)

    # 5. OpenGL
    target_link_libraries(${PROJECT_NAME} OpenGL::GL)

elseif(PLATFORM_LINUX)
    target_link_libraries(${PROJECT_NAME} pthread dl)

    # SDL2 for Linux
    if(SDL2_FOUND)
        if(TARGET SDL2::SDL2)
            target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
        else()
            target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
            target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
        endif()
    else()
        message(FATAL_ERROR "SDL2 is required but was not found!")
    endif()

    # OpenGL
    target_link_libraries(${PROJECT_NAME} OpenGL::GL)

elseif(PLATFORM_MACOS)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )

    # SDL2 for macOS
    if(SDL2_FOUND)
        if(TARGET SDL2::SDL2)
            target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
        else()
            target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
            target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
        endif()
    else()
        message(FATAL_ERROR "SDL2 is required but was not found!")
    endif()

    # OpenGL
    target_link_libraries(${PROJECT_NAME} OpenGL::GL)

elseif(PLATFORM_WEB)
    # Emscripten-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        OUTPUT_NAME "QuantumIdle"
    )

    # Emscripten compile flags (MUST include SDL2 flag for compilation)
    target_compile_options(${PROJECT_NAME} PRIVATE
        "-sUSE_SDL=2"
    )

    # Emscripten linker flags
    set(EMSCRIPTEN_LINK_FLAGS
        "-sUSE_SDL=2"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=67108864"  # 64MB
        "-sMAXIMUM_MEMORY=268435456"  # 256MB max
        "-sSTACK_SIZE=5242880"  # 5MB stack
        "-sASSERTIONS=0"
        "-sGL_ASSERTIONS=0"
        "-sDISABLE_EXCEPTION_CATCHING=1"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        "--shell-file ${CMAKE_SOURCE_DIR}/web/index.html"
    )

    # Optimization flags for release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND EMSCRIPTEN_LINK_FLAGS
            "-O3"
            "-sAGGRESSIVE_VARIABLE_ELIMINATION=1"
            "--closure=1"
        )
    else()
        list(APPEND EMSCRIPTEN_LINK_FLAGS
            "-O0"
            "-g"
            "-sSAFE_HEAP=1"
        )
    endif()

    # Join flags and apply
    string(REPLACE ";" " " EMSCRIPTEN_LINK_FLAGS_STR "${EMSCRIPTEN_LINK_FLAGS}")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS_STR}"
    )

    message(STATUS "Emscripten link flags: ${EMSCRIPTEN_LINK_FLAGS_STR}")

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
