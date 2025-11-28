cmake_minimum_required(VERSION 3.15)
project(QuantumIdle VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
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

---

## 🎨 ImGui Library Definition

# Define the source directory for ImGui (assuming it's checked out to 'external/imgui')
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create the ImGui Static Library
add_library(ImGui STATIC ${IMGUI_SOURCES})

# Set the include directories for ImGui
target_include_directories(ImGui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Tell ImGui's backends what headers they need (SDL2, OpenGL)
target_link_libraries(ImGui PUBLIC OpenGL::GL)


---

## 📦 Dependency Finding

# Find packages
find_package(OpenGL REQUIRED)

# SDL2 - cross-platform windowing and input
# Find SDL2 using platform-specific methods
if(PLATFORM_WEB)
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
    # Desktop platforms (Windows, Linux, macOS)
    # Try vcpkg/CMake config first, then pkg-config
    find_package(SDL2 CONFIG QUIET)

    if(SDL2_FOUND)
        message(STATUS "Found SDL2 via vcpkg/CMake CONFIG")
    else()
        # Try pkg-config (Linux/macOS)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(SDL2 QUIET sdl2)
            if(SDL2_FOUND)
                message(STATUS "Found SDL2 via pkg-config")
            endif()
        endif()
    endif()

    if(NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 not found!\n"
            "Windows (vcpkg): vcpkg install sdl2\n"
            "Ubuntu/Debian: sudo apt-get install libsdl2-dev\n"
            "macOS: brew install sdl2")
    endif()

    # Find SDL2_ttf for font rendering
    find_package(SDL2_ttf CONFIG QUIET)
    if(NOT SDL2_ttf_FOUND)
        # Try pkg-config as fallback
        find_package(PkgConfig)
        if(PKG_CONFIG_FOUND)
            pkg_search_module(SDL2_TTF QUIET SDL2_ttf)
            if(SDL2_TTF_FOUND)
                set(SDL2_ttf_FOUND TRUE)
            endif()
        endif()
    endif()

    if(NOT SDL2_ttf_FOUND)
        message(FATAL_ERROR "SDL2_ttf not found!\n"
            "Windows (vcpkg): vcpkg install sdl2-ttf\n"
            "Ubuntu/Debian: sudo apt-get install libsdl2-ttf-dev\n"
            "macOS: brew install sdl2_ttf")
    endif()
endif()

---

## 🏃 Target Definition

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
        src/Main.cpp
        ${ENGINE_SOURCES}
        ${GAME_SOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        src/Main.cpp
        ${ENGINE_SOURCES}
        ${GAME_SOURCES}
    )
endif()

# Use target_include_directories for modern practice
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

---

## 🔗 Linking Libraries (Including ImGui)

# Link libraries - ORDER MATTERS for MinGW!
# Correct order for MinGW: mingw32 -> SDL2main -> SDL2 -> winmm -> opengl32

if(PLATFORM_WINDOWS)
    # Link ImGui first, which provides its own includes and links OpenGL
    target_link_libraries(${PROJECT_NAME} PRIVATE ImGui)

    if(SDL2_FOUND)
        # 1. Link Mingw32 FIRST (required for proper WinMain handling with MinGW)
        if(MINGW)
            target_link_libraries(${PROJECT_NAME} PRIVATE mingw32)
        endif()

        # 2. Link SDL2 - prefer imported targets from vcpkg
        if(TARGET SDL2::SDL2)
            # vcpkg provides imported targets
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2)
        else()
            # Fallback to variables
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
        endif()
    else()
        message(FATAL_ERROR "SDL2 is required but was not found!")
    endif()

    # Link SDL2_ttf
    if(SDL2_ttf_FOUND)
        if(TARGET SDL2_ttf::SDL2_ttf)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
        elseif(SDL2_TTF_FOUND)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
        endif()
    endif()

    # 3. Windows system libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm imm32 version setupapi)

    # 4. OpenGL (Linked via ImGui target, but explicit is fine if needed)
    # target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)


elseif(PLATFORM_LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ImGui pthread dl)

    # SDL2 for Linux
    if(SDL2_FOUND)
        if(TARGET SDL2::SDL2)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
        endif()
    else()
        message(FATAL_ERROR "SDL2 is required but was not found!")
    endif()

    # SDL2_ttf for Linux
    if(SDL2_ttf_FOUND)
        if(TARGET SDL2_ttf::SDL2_ttf)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
        elseif(SDL2_TTF_FOUND)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
        endif()
    endif()

    # OpenGL
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

elseif(PLATFORM_MACOS)
    target_link_libraries(${PROJECT_NAME} PRIVATE ImGui)
    
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )

    # SDL2 for macOS
    if(SDL2_FOUND)
        if(TARGET SDL2::SDL2)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
        endif()
    else()
        message(FATAL_ERROR "SDL2 is required but was not found!")
    endif()

    # SDL2_ttf for macOS
    if(SDL2_ttf_FOUND)
        if(TARGET SDL2_ttf::SDL2_ttf)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
        elseif(SDL2_TTF_FOUND)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
        endif()
    endif()

    # OpenGL
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

elseif(PLATFORM_WEB)
    # Emscripten-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        OUTPUT_NAME "QuantumIdle"
    )

    # Emscripten compile flags (MUST include SDL2 flags for compilation)
    target_compile_options(${PROJECT_NAME} PRIVATE
        "-sUSE_SDL=2"
        "-sUSE_SDL_TTF=2" # ImGui uses SDL_ttf for loading/generating fonts
    )

    # Emscripten linker flags
    set(EMSCRIPTEN_LINK_FLAGS
        "-sUSE_SDL=2"
        "-sUSE_SDL_TTF=2"
        "-lidbfs.js"  # Include IndexedDB filesystem for persistent saves
        "-sLEGACY_GL_EMULATION=1"  # Emulate legacy OpenGL (WebGL 1 compatible)
        "-sGL_UNSAFE_OPTS=0"  # Disable unsafe GL optimizations
        "-sMIN_WEBGL_VERSION=1"
        "-sMAX_WEBGL_VERSION=1" 
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=67108864" 
        "-sMAXIMUM_MEMORY=268435456"
        "-sSTACK_SIZE=5242880"
        "-sASSERTIONS=1"
        "-sGL_ASSERTIONS=1"
        "-sDISABLE_EXCEPTION_CATCHING=1"
        "--shell-file ${CMAKE_SOURCE_DIR}/web/index.html"
        "--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
    )

    # Optimization flags for release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND EMSCRIPTEN_LINK_FLAGS
            "-O3"
            "-sAGGRESSIVE_VARIABLE_ELIMINATION=1"
        )
    else()
        list(APPEND EMSCRIPTEN_LINK_FLAGS
            "-O0"
            "-g"
            "-sSAFE_HEAP=1"
        )
    endif()

    # Apply link flags
    string(REPLACE ";" " " EMSCRIPTEN_LINK_FLAGS_STR "${EMSCRIPTEN_LINK_FLAGS}")
    target_link_libraries(${PROJECT_NAME} PRIVATE ImGui) # Link the ImGui target
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS_STR}"
    )

    message(STATUS "Emscripten link flags: ${EMSCRIPTEN_LINK_FLAGS_STR}")

endif()

---

## 💾 Installation Rules

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

# Copy assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION bin
)
