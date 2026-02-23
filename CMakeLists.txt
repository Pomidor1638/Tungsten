cmake_minimum_required(VERSION 3.20)
project(MegaGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_net CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
    
    src/client/audio/audio.cpp
    src/client/input/input.cpp
    src/client/render/render.cpp
    src/client/window/window.cpp
    src/client/client.cpp

    src/server/server.cpp

    src/common/common.cpp
    src/common/allocator/allocator.cpp
    src/common/bspfile/bspfile.cpp
    src/common/cmd/console.cpp
    src/common/logger/logger.cpp
    src/common/math/math.cpp
    src/common/net/net.cpp
    src/common/system/system.cpp
    src/common/system/tinyfiledialogs.c
    
   
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_impl_opengl3.cpp
    src/imgui/imgui_impl_sdl2.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
)

set(HEADERS
    src/client/audio/audio.h
    src/client/input/input.h
    src/client/render/render.h
    src/client/window/window.h
    src/client/client.h
    src/server/server.h
    src/common/common.h
    src/common/allocator/allocator.h
    src/common/bspfile/bspfile.h
    src/common/cmd/console.h
    src/common/logger/logger.h
    src/common/math/math.h
    src/common/math/fixed/fixed.hpp
    src/common/math/fixed/math.hpp
    src/common/math/fixed/ios.hpp
    src/common/net/enet.h
    src/common/net/net.h
    src/common/system/system.h
    src/common/system/tinyfiledialogs.h

    src/imgui/imconfig.h
    src/imgui/imgui.h
    src/imgui/imgui_impl_opengl3.h
    src/imgui/imgui_impl_opengl3_loader.h
    src/imgui/imgui_impl_sdl2.h
    src/imgui/imgui_internal.h
    src/imgui/imstb_rectpack.h
    src/imgui/imstb_textedit.h
    src/imgui/imstb_truetype.h   
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_net::SDL2_net
    glm::glm
    OpenGL::GL
    glad::glad
)