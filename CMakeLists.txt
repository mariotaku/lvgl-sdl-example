cmake_minimum_required(VERSION 3.13)
project(lvgl_sdl_example C)

set(CMAKE_C_STANDARD 11)

include(cmake/CPM.cmake)
find_package(SDL2 QUIET)
if (NOT SDL2_FOUND)
    CPMAddPackage(GITHUB_REPOSITORY libsdl-org/SDL
            NAME SDL2
            GIT_TAG release-2.0.16
            OPTIONS
            "SDL_SHARED_ENABLED_BY_DEFAULT OFF"
            "SDL_STATIC_ENABLED_BY_DEFAULT ON"
            "OPT_DEF_LIBC TRUE"
            "CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG}"
            "CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL}"
            "CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO}"
            "CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE}")
    if (SDL2_ADDED)
        set(SDL2_INCLUDE_DIRS ${SDL2_SOURCE_DIR}/include)
        set(SDL2_LIBRARIES SDL2-static SDL2main)
        # SDL_ttf tries to link to SDL2::SDL2 even in static builds.
        add_library(SDL2::SDL2 ALIAS SDL2-static)
    endif ()
endif ()

add_subdirectory(lvgl)

target_include_directories(lvgl PUBLIC ${CMAKE_SOURCE_DIR}/lvgl/src PRIVATE ${CMAKE_SOURCE_DIR}/lvgl_drv)
target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=${CMAKE_SOURCE_DIR}/lv_conf.h PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

target_link_libraries(lvgl PRIVATE ${SDL2_LIBRARIES})
target_include_directories(lvgl PUBLIC ${SDL2_INCLUDE_DIRS})

add_executable(example main.c)
target_link_libraries(example PRIVATE ${SDL2_LIBRARIES} lvgl)
target_include_directories(example PUBLIC ${SDL2_INCLUDE_DIRS})