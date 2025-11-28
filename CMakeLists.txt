cmake_minimum_required(VERSION 3.20)
project(C_Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# ============================================================
# Emscripten / Web Platform Options
# ============================================================
if(PLATFORM_WEB)
    message(STATUS "Building for Web (Emscripten)")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(USE_FLAGS "-sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_MIXER=2 -sUSE_SDL_TTF=2 -sWASM=1 -sASYNCIFY")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
endif()

# ============================================================
# SDL2
# ============================================================
if(NOT PLATFORM_WEB)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

# ============================================================
# FetchContent — ImGui auto-download
# ============================================================
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
)

FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(ImGui STATIC ${IMGUI_SOURCES})

target_include_directories(ImGui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Desktop OpenGL only — NOT for Web
if(NOT PLATFORM_WEB)
    find_package(OpenGL REQUIRED)
    target_link_libraries(ImGui PUBLIC OpenGL::GL)
endif()

# ============================================================
# Game Source Files
# ============================================================
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(C_Game ${GAME_SOURCES})

target_include_directories(C_Game PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

if(PLATFORM_WEB)
    target_link_libraries(C_Game PUBLIC ImGui)
else()
    target_link_libraries(C_Game PUBLIC ImGui SDL2::SDL2 SDL2::SDL2main)
endif()

