cmake_minimum_required(VERSION 3.20)
project(lvgl_sdl_example C)

set(CMAKE_C_STANDARD 11)


add_executable(example main.c)
target_link_libraries(example mingw32 SDL2main SDL2)