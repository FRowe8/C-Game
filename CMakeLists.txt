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

    if(WIN32)
        add_link_options(-mconsole)
    endif()
endif()

# ImGui
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

add_library(ImGui STATIC ${IMGUI_SOURCES})

target_include_directories(ImGui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ImGui PUBLIC OpenGL::GL)

# Dependencies
find_package(OpenGL REQUIRED)

if(PLATFORM_WEB)
    set(SDL2_FOUND TRUE)
    message(STATUS "Using Emscripten's built-in SDL2")

    # --- FIX START ---
    # Apply Emscripten SDL compile options to the ImGui library target
    target_compile_options(ImGui PRIVATE
            "-sUSE_SDL=2"
            "-sUSE_SDL_TTF=2"
    )
    # --- FIX END ---

elseif(PLATFORM_ANDROID OR PLATFORM_IOS)
    if(PLATFORM_ANDROID)
        find_library(SDL2_LIBRARY SDL2)
        set(SDL2_FOUND TRUE)
    endif()

else()
    find_package(SDL2 CONFIG QUIET)

    if(NOT SDL2_FOUND)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(SDL2 QUIET sdl2)
        endif()
    endif()

    if(NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 not found!")
    endif()

    find_package(SDL2_ttf CONFIG QUIET)
    if(NOT SDL2_ttf_FOUND)
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_search_module(SDL2_TTF QUIET SDL2_ttf)
            if(SDL2_TTF_FOUND)
                set(SDL2_ttf_FOUND TRUE)
            endif()
        endif()
    endif()

    if(NOT SDL2_ttf_FOUND)
        message(FATAL_ERROR "SDL2_ttf not found!")
    endif()
endif()

# Make SDL2 available to ImGui backend
if(SDL2_FOUND)
    if(TARGET SDL2::SDL2)
        target_link_libraries(ImGui PUBLIC SDL2::SDL2)
        target_include_directories(ImGui PUBLIC $<TARGET_PROPERTY:SDL2::SDL2,INTERFACE_INCLUDE_DIRECTORIES>)
    else()
        target_include_directories(ImGui PUBLIC ${SDL2_INCLUDE_DIRS})
        target_link_libraries(ImGui PUBLIC ${SDL2_LIBRARIES})
    endif()
endif()


# Sources
file(GLOB_RECURSE ENGINE_SOURCES
        "src/engine/*.cpp"
        "src/engine/*.c"
)

# Move UIManager.cpp and relevant headers back to the game executable's sources
set(GAME_UI_SOURCES
        include/ImGuiUtils.h
        include/UIManager.h
        src/game/UIManager.cpp
)

file(GLOB_RECURSE GAME_SOURCES
        "src/game/*.cpp"
)
list(APPEND GAME_SOURCES ${GAME_UI_SOURCES}) # Append the UI sources to the main list


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

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/game/ui
)

# Linking
if(PLATFORM_WINDOWS)
    target_link_libraries(${PROJECT_NAME} PRIVATE ImGui)

    if(SDL2_FOUND)
        if(MINGW)
            target_link_libraries(${PROJECT_NAME} PRIVATE mingw32)
        endif()

        if(TARGET SDL2::SDL2)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2)
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
        endif()
    endif()

    if(SDL2_ttf_FOUND)
        if(TARGET SDL2_ttf::SDL2_ttf)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
        elseif(SDL2_TTF_FOUND)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
        endif()
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE winmm imm32 version setupapi)

elseif(PLATFORM_LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ImGui pthread dl)

    if(SDL2_FOUND)
        if(TARGET SDL2::SDL2)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
        endif()
    endif()

    if(SDL2_ttf_FOUND)
        if(TARGET SDL2_ttf::SDL2_ttf)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
        elseif(SDL2_TTF_FOUND)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
        endif()
    endif()

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

    if(SDL2_FOUND)
        if(TARGET SDL2::SDL2)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
        endif()
    endif()

    if(SDL2_ttf_FOUND)
        if(TARGET SDL2_ttf::SDL2_ttf)
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
        elseif(SDL2_TTF_FOUND)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
        endif()
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

elseif(PLATFORM_WEB)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            SUFFIX ".html"
            OUTPUT_NAME "QuantumIdle"
    )

    target_compile_options(${PROJECT_NAME} PRIVATE
            "-sUSE_SDL=2"
            "-sUSE_SDL_TTF=2"
    )

    set(EMSCRIPTEN_LINK_FLAGS
            "-sUSE_SDL=2"
            "-sUSE_SDL_TTF=2"
            "-lidbfs.js"
            "-sLEGACY_GL_EMULATION=1"
            "-sGL_UNSAFE_OPTS=0"
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

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-O3" "-sAGGRESSIVE_VARIABLE_ELIMINATION=1")
    else()
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-O0" "-g" "-sSAFE_HEAP=1")
    endif()

    string(REPLACE ";" " " EMSCRIPTEN_LINK_FLAGS_STR "${EMSCRIPTEN_LINK_FLAGS}")

    target_link_libraries(${PROJECT_NAME} PRIVATE ImGui)

    set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS_STR}"
    )

endif()

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION bin)