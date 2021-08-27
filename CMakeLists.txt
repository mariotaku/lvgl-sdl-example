cmake_minimum_required(VERSION 3.13)
project(lvgl_sdl_example C)

set(CMAKE_C_STANDARD 11)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

find_package(SDL2 QUIET)

if (NOT SDL2_FOUND)
    include(FetchContent)
    if (MSVC)
        FetchContent_Declare(
                SDL2
                URL https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip
                URL_MD5 22bdb85ce4874673c4fafcce772d8b72
        )
    elseif (MINGW)
        FetchContent_Declare(
                SDL2
                URL https://www.libsdl.org/release/SDL2-devel-2.0.9-mingw.tar.gz
                URL_MD5 f5645eed64214c3bc22a3e157fc1f15f
        )
    else ()
        message(FATAL_ERROR "SDL2 not available")
    endif ()
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_ROOT_DIR ${sdl2_SOURCE_DIR})
endif ()
if (WIN32)
    #    find_package(WinHID)
endif ()

find_package(SDL2 REQUIRED)

if (NOT WIN32)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif ()

add_subdirectory(lvgl)

add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE)

target_include_directories(lvgl PUBLIC ${CMAKE_SOURCE_DIR}/lvgl/src PRIVATE ${CMAKE_SOURCE_DIR}/lvgl_drv)
target_link_libraries(lvgl PRIVATE SDL2::SDL2)
#target_link_libraries(lvgl_examples lvgl)

add_subdirectory(lv_demos)
target_link_libraries(lv_examples lvgl)

add_executable(example main.c input_drv/indev_sdl.c input_drv/syswm_win.c)

file(GLOB_RECURSE LVGL_EXAMPLE_SOURCES lvgl/examples/*.c)
target_sources(example PRIVATE ${LVGL_EXAMPLE_SOURCES})

if (MINGW)
    target_link_libraries(example PRIVATE mingw32)
endif ()

if (SDL2MAIN_LIBRARIES)
    target_link_libraries(example PRIVATE SDL2::SDL2main)
else ()
    target_link_libraries(example PRIVATE SDL2::SDL2)
endif ()

target_link_libraries(example PRIVATE lvgl lv_examples)
#target_link_libraries(example PRIVATE lvgl lvgl_examples lv_examples)

if (WINHID_FOUND)
    target_link_libraries(example PRIVATE ${WINHID_LIBRARY})
    target_include_directories(example SYSTEM PRIVATE ${WINHID_INCLUDE_DIRS})
endif ()
if (WIN32)
    # copy the .dll file to the same folder as the executable
    add_custom_command(TARGET example POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM)
endif ()