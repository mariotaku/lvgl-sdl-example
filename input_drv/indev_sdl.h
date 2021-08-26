//
// Created by Mariotaku on 2021/08/25.
//

#ifndef LVGL_SDL_EXAMPLE_INDEV_SDL_H
#define LVGL_SDL_EXAMPLE_INDEV_SDL_H

#define HAS_SCROLL_EVENT 0

#include <SDL.h>
#include <SDL_syswm.h>

typedef enum {
    INDEV_SCROLL = SDL_USEREVENT + 0x10,
    INDEV_LAST
} INDEV_USER_EVENTS_T;

void indev_init(SDL_Window *window);

void indev_handle_syswm_evt(SDL_SysWMEvent *event);

void _indev_handle_syswm_evt_win(SDL_SysWMEvent *event);

void _indev_handle_syswm_setup_win(SDL_SysWMinfo *info);

#endif //LVGL_SDL_EXAMPLE_INDEV_SDL_H
