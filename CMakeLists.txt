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
if(NOT EXISTS "${TG_SOURCE_ROOT}/tengine/main.cpp")
    foreach(candidate
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )
        if(EXISTS "${candidate}/tengine/main.cpp")
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
        if(name STREQUAL "main.cpp" OR name STREQUAL "main.c" OR name STREQUAL "main.cc" OR name STREQUAL "main.cxx")
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
        "${TG_SOURCE_ROOT}/common/utils"
        "${TG_SOURCE_ROOT}/tengine"
        "${TG_SOURCE_ROOT}/tengine/client"
        "${TG_SOURCE_ROOT}/tengine/server"
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
        add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL ${target_sources} "src/tengine/common/net/net.h" "src/common/platform/memory/memory.h"  "src/common/utils/binary/binary.h" "src/common/utils/binary/binary.cpp" "src/common/platform/memory/memory.cpp" "src/common/formats/twad/tmaterial/tmaterial.h" "src/common/formats/twad/tmodel/tmodel.h" "src/common/formats/twad/tprog/tpog.h" "src/common/formats/twad/tsound/tsound.h")
    else()
        add_executable(${TARGET_NAME} ${target_sources} "src/tengine/common/net/net.h" "src/common/platform/memory/memory.h"  "src/common/utils/binary/binary.h" "src/common/utils/binary/binary.cpp" "src/common/platform/memory/memory.cpp" "src/common/formats/twad/tmaterial/tmaterial.h" "src/common/formats/twad/tmodel/tmodel.h" "src/common/formats/twad/tprog/tpog.h" "src/common/formats/twad/tsound/tsound.h")
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

tg_collect_sources(TG_SHARED_SOURCES
    "common/formats/twad"
)

tg_collect_sources(TG_ENGINE_SOURCES
    "tengine/client"
    "tengine/server"
    "tengine/common"
    "tengine/shared"
)

tg_collect_sources(TG_TBSPCOMPILER_SOURCES
    "compiler/tbsp"
    "tbsp"
)

tg_collect_sources(TG_TMAPEDITOR_SOURCES
    "editor/tmapeditor"
    "tmapeditor"
)

tg_collect_sources(TG_TWADEDITOR_SOURCES
    "editor/twad"
    "twad"
)

tg_collect_sources(TG_TEST_SOURCES
    "test"
    "common/formats/protocol"
)

# =============================================================================
# 1) tengine
# =============================================================================

set(TG_TENGINE_SOURCES
    ${TG_SHARED_SOURCES}
    ${TG_ENGINE_SOURCES}
)

if(EXISTS "${TG_SOURCE_ROOT}/tengine/main.cpp")
    list(APPEND TG_TENGINE_SOURCES "${TG_SOURCE_ROOT}/tengine/main.cpp")
else()
    message(WARNING "tengine: ${TG_SOURCE_ROOT}/tengine/main.cpp not found. Check TG_SOURCE_ROOT.")
endif()

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
    ${TG_SHARED_SOURCES}
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
    ${TG_SHARED_SOURCES}
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
    ${TG_SHARED_SOURCES}
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
