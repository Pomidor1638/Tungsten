cmake_minimum_required(VERSION 3.21)

project(Tungsten LANGUAGES C CXX)

# =============================================================================
# Global settings
# =============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# If CMakeLists.txt is not located near src/, pass:
#   -DTG_SOURCE_ROOT=C:/path/to/your/source/root/src
set(TG_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE PATH "Tungsten source root")

# Try to auto-detect the real source root when CMakeLists.txt was put one level above.
if(NOT EXISTS "${TG_SOURCE_ROOT}/common/formats/protocol/protocol.h")
    foreach(candidate
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )
        if(EXISTS "${candidate}/common/formats/protocol/protocol.h")
            set(TG_SOURCE_ROOT "${candidate}" CACHE PATH "Tungsten source root" FORCE)
            break()
        endif()
    endforeach()
endif()

message(STATUS "TG_SOURCE_ROOT = ${TG_SOURCE_ROOT}")

# Executables will be placed into build/bin/<config>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${CONFIG}" CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin/${CONFIG}")
endforeach()

# =============================================================================
# Helpers
# =============================================================================

function(tg_collect_sources OUT_VAR)
    set(result)

    foreach(dir IN LISTS ARGN)
        set(abs_dir "${TG_SOURCE_ROOT}/${dir}")

        if(EXISTS "${abs_dir}")
            file(GLOB_RECURSE dir_sources CONFIGURE_DEPENDS
                "${abs_dir}/*.h"
                "${abs_dir}/*.hpp"
                "${abs_dir}/*.inl"
                "${abs_dir}/*.c"
                "${abs_dir}/*.cpp"
                "${abs_dir}/*.cc"
                "${abs_dir}/*.cxx"
            )
            list(APPEND result ${dir_sources})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES result)
    set(${OUT_VAR} ${result} PARENT_SCOPE)
endfunction()

function(tg_make_stub_main OUT_FILE TARGET_NAME)
    set(stub_dir "${CMAKE_BINARY_DIR}/generated/${TARGET_NAME}")
    file(MAKE_DIRECTORY "${stub_dir}")

    set(stub_file "${stub_dir}/main.cpp")
    file(WRITE "${stub_file}"
"// Auto-generated stub for ${TARGET_NAME}.\n"
"// Add a real main.cpp to this target folder to replace this.\n"
"#include <iostream>\n\n"
"int main(int argc, char** argv)\n"
"{\n"
"    (void)argc;\n"
"    (void)argv;\n"
"    std::cout << \"${TARGET_NAME}: stub executable\\n\";\n"
"    return 0;\n"
"}\n"
    )

    set(${OUT_FILE} "${stub_file}" PARENT_SCOPE)
endfunction()

function(tg_contains_main OUT_VAR)
    set(found FALSE)

    foreach(src IN LISTS ARGN)
        get_filename_component(name "${src}" NAME)
        if(name MATCHES "^main(_.+)?\\.(c|cc|cpp|cxx)$")
            set(found TRUE)
            break()
        endif()
    endforeach()

    set(${OUT_VAR} ${found} PARENT_SCOPE)
endfunction()

function(tg_apply_common_settings TARGET_NAME)
    target_include_directories(${TARGET_NAME} PRIVATE
        "${TG_SOURCE_ROOT}"
        "${TG_SOURCE_ROOT}/common"
        "${TG_SOURCE_ROOT}/common/formats"
        "${TG_SOURCE_ROOT}/runtime"
        "${TG_SOURCE_ROOT}/runtime/common"
        "${TG_SOURCE_ROOT}/runtime/game"
        "${TG_SOURCE_ROOT}/runtime/game/client"
        "${TG_SOURCE_ROOT}/runtime/tclient"
        "${TG_SOURCE_ROOT}/runtime/tclient/world"
        "${TG_SOURCE_ROOT}/runtime/tserver"
    )

    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /permissive-
            /utf-8
            /Zc:__cplusplus
        )

        target_compile_definitions(${TARGET_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            NOMINMAX
            WIN32_LEAN_AND_MEAN
        )
    else()
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()
endfunction()

function(tg_link_sdl TARGET_NAME)
    find_package(SDL2 CONFIG QUIET)

    if(TARGET SDL2::SDL2)
        if(TARGET SDL2::SDL2main)
            target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2main)
        endif()
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2)
    elseif(TARGET SDL2::SDL2-static)
        if(TARGET SDL2::SDL2main)
            target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2main)
        endif()
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2-static)
    else()
        find_package(SDL2 REQUIRED)
        target_include_directories(${TARGET_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} PRIVATE ${SDL2_LIBRARIES})
    endif()

    if(WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE
            comdlg32
            ole32
            user32
            shell32
        )
    endif()
endfunction()

function(tg_link_opengl TARGET_NAME)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenGL::GL)

    find_package(glad CONFIG QUIET)
    if(TARGET glad::glad)
        target_link_libraries(${TARGET_NAME} PRIVATE glad::glad)
    endif()
endfunction()

function(tg_link_glm_if_available TARGET_NAME)
    find_package(glm CONFIG QUIET)
    if(TARGET glm::glm)
        target_link_libraries(${TARGET_NAME} PRIVATE glm::glm)
    endif()
endfunction()

function(tg_add_executable TARGET_NAME)
    cmake_parse_arguments(ARG "EXCLUDE_FROM_ALL" "FOLDER;NEEDS_SDL;NEEDS_OPENGL" "SOURCES" ${ARGN})

    set(target_sources ${ARG_SOURCES})
    list(REMOVE_DUPLICATES target_sources)

    tg_contains_main(has_main ${target_sources})
    if(NOT has_main)
        tg_make_stub_main(stub_main "${TARGET_NAME}")
        list(APPEND target_sources "${stub_main}")
        message(WARNING "${TARGET_NAME}: no main.cpp found for this target, using generated stub: ${stub_main}")
    endif()

    if(ARG_EXCLUDE_FROM_ALL)
        add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL ${target_sources}   "src/runtime/common/sys/sys_event.h" "src/runtime/common/host/host_local.h" "src/runtime/common/host/host_args.cpp" "src/runtime/common/host/host_zone.cpp" "src/runtime/common/host/host_window.cpp" "src/runtime/common/host/host_events.cpp" "src/runtime/common/sys/sdl/sys_event.cpp")
    else()
        add_executable(${TARGET_NAME} ${target_sources}   "src/runtime/common/sys/sys_event.h" "src/runtime/common/host/host_local.h" "src/runtime/common/host/host_args.cpp" "src/runtime/common/host/host_zone.cpp" "src/runtime/common/host/host_window.cpp" "src/runtime/common/host/host_events.cpp" "src/runtime/common/sys/sdl/sys_event.cpp")
    endif()

    tg_apply_common_settings(${TARGET_NAME})
    tg_link_glm_if_available(${TARGET_NAME})

    if(ARG_NEEDS_SDL)
        tg_link_sdl(${TARGET_NAME})
    endif()

    if(ARG_NEEDS_OPENGL)
        tg_link_opengl(${TARGET_NAME})
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES
        FOLDER "${ARG_FOLDER}"
        VS_DEBUGGER_WORKING_DIRECTORY "${TG_SOURCE_ROOT}"
    )
endfunction()

# =============================================================================
# Source sets
# =============================================================================
set(TG_COMMON_BASE_SOURCES)

tg_collect_sources(TG_COMMON_SHARED_SOURCES
    "common/formats"
    "common/allocator"
    "common/binary"
    "common/compress"
    "common/container"
    "common/endian"
    "common/hash"
    "common/math"
    "common/string"
)

tg_collect_sources(TG_RUNTIME_COMMON_SOURCES
    "runtime/common/console"
    "runtime/common/host"
    "runtime/common/net"
)

set(TG_GAME_CLIENT_MAIN "${TG_SOURCE_ROOT}/runtime/game/client/main_sdl.cpp")
set(TG_GAME_SERVER_MAIN "${TG_SOURCE_ROOT}/runtime/game/server/main.cpp")

set(TG_SYS_BACKEND "sdl" CACHE STRING "Tungsten sys backend")
set_property(CACHE TG_SYS_BACKEND PROPERTY STRINGS sdl ps2)

set(TG_RUNTIME_SYS_SOURCES)

foreach(src IN ITEMS
    "runtime/common/sys/sys.h"
    "runtime/common/sys/sys_local.h"
    "runtime/common/sys/sys.cpp"
    "runtime/common/sys/tinyfiledialogs.h"
    "runtime/common/sys/tinyfiledialogs.c"
)
    if(EXISTS "${TG_SOURCE_ROOT}/${src}")
        list(APPEND TG_RUNTIME_SYS_SOURCES "${TG_SOURCE_ROOT}/${src}")
    endif()
endforeach()

if(TG_SYS_BACKEND STREQUAL "sdl")
    set(TG_SYS_BACKEND_SOURCE "runtime/common/sys/sdl/sys_sdl.cpp")
elseif(TG_SYS_BACKEND STREQUAL "ps2")
    set(TG_SYS_BACKEND_SOURCE "runtime/common/sys/ps2/sys_ps2.cpp")
else()
    message(FATAL_ERROR "Unknown TG_SYS_BACKEND: ${TG_SYS_BACKEND}")
endif()

if(EXISTS "${TG_SOURCE_ROOT}/${TG_SYS_BACKEND_SOURCE}")
    list(APPEND TG_RUNTIME_SYS_SOURCES "${TG_SOURCE_ROOT}/${TG_SYS_BACKEND_SOURCE}")
else()
    message(WARNING "Selected sys backend source not found: ${TG_SOURCE_ROOT}/${TG_SYS_BACKEND_SOURCE}")
endif()

tg_collect_sources(TG_TCLIENT_SOURCES
    "runtime/tclient"
)
list(REMOVE_ITEM TG_TCLIENT_SOURCES
    "${TG_SOURCE_ROOT}/runtime/tclient/main.cpp"
)

tg_collect_sources(TG_TSERVER_SOURCES
    "runtime/tserver"
)
list(REMOVE_ITEM TG_TSERVER_SOURCES
    "${TG_SOURCE_ROOT}/runtime/tserver/main.cpp"
)

tg_collect_sources(TG_TBSPCOMPILER_SOURCES
    "tools/tbsp"
)

tg_collect_sources(TG_TMAPEDITOR_SOURCES
    "tools/tmapeditor"
)

tg_collect_sources(TG_TWADEDITOR_SOURCES
    "tools/twad"
)

tg_collect_sources(TG_TEST_SOURCES
    "test"
    "common/formats/protocol"
    "common/binary"
)

# =============================================================================
# 1) tengine
# =============================================================================

set(TG_TENGINE_SOURCES
    ${TG_COMMON_BASE_SOURCES}
    ${TG_COMMON_SHARED_SOURCES}
    
    ${TG_RUNTIME_COMMON_SOURCES}
    ${TG_RUNTIME_SYS_SOURCES}
    ${TG_GAME_CLIENT_MAIN}
    ${TG_TCLIENT_SOURCES}
    ${TG_TSERVER_SOURCES}
)

tg_add_executable(tengine
    FOLDER "runtime"
    NEEDS_SDL TRUE
    NEEDS_OPENGL TRUE
    SOURCES ${TG_TENGINE_SOURCES}
)

# =============================================================================
# 2) tbspcompiler
# =============================================================================

set(TG_TBSPCOMPILER_ALL_SOURCES
    ${TG_COMMON_BASE_SOURCES}
    ${TG_COMMON_SHARED_SOURCES}
    
    ${TG_TBSPCOMPILER_SOURCES}
)

tg_add_executable(tbspcompiler
    FOLDER "tools/compiler"
    NEEDS_SDL TRUE
    NEEDS_OPENGL FALSE
    SOURCES ${TG_TBSPCOMPILER_ALL_SOURCES}
)

# =============================================================================
# 3) tmapeditor
# =============================================================================

set(TG_TMAPEDITOR_ALL_SOURCES
    ${TG_COMMON_BASE_SOURCES}
    ${TG_COMMON_SHARED_SOURCES}
    
    ${TG_TMAPEDITOR_SOURCES}
)

tg_add_executable(tmapeditor
    FOLDER "tools/editor"
    NEEDS_SDL TRUE
    NEEDS_OPENGL TRUE
    SOURCES ${TG_TMAPEDITOR_ALL_SOURCES}
)

# =============================================================================
# 4) twadeditor
# =============================================================================

set(TG_TWADEDITOR_ALL_SOURCES
    ${TG_COMMON_BASE_SOURCES}
    ${TG_COMMON_SHARED_SOURCES}
    
    ${TG_TWADEDITOR_SOURCES}
)

tg_add_executable(twadeditor
    FOLDER "tools/editor"
    NEEDS_SDL TRUE
    NEEDS_OPENGL TRUE
    SOURCES ${TG_TWADEDITOR_ALL_SOURCES}
)

# =============================================================================
# 5) test
# =============================================================================

set(TG_TEST_ALL_SOURCES
    ${TG_TEST_SOURCES}
)

tg_add_executable(test
    EXCLUDE_FROM_ALL
    FOLDER "tests"
    NEEDS_SDL FALSE
    NEEDS_OPENGL FALSE
    SOURCES ${TG_TEST_ALL_SOURCES}
)

# =============================================================================
# Visual Studio
# =============================================================================

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT tengine)




