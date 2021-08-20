cmake_minimum_required(VERSION 3.20)
project(lvgl_sdl_example C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${CMAKE_SOURCE_DIR}/lvgl/src PRIVATE ${CMAKE_SOURCE_DIR}/lvgl_drv)
target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=${CMAKE_SOURCE_DIR}/lv_conf.h PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_link_libraries(lvgl mingw32 SDL2main SDL2)

add_executable(example main.c)
target_link_libraries(example mingw32 SDL2main SDL2 lvgl)