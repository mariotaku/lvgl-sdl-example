//
// Created by Mariotaku on 2021/08/25.
//

#include "indev_sdl.h"

#include <SDL_events.h>

#include <SDL_syswm.h>
#include <hal/lv_hal.h>


lv_indev_t *lv_sdl_init_pointer();

#if HAS_SCROLL_EVENT
lv_indev_t *lv_sdl_init_scroll();
#endif

void indev_init(SDL_Window *window) {
#if HAS_SCROLL_EVENT
    Uint32 start = SDL_RegisterEvents(0x30);
    SDL_assert(start == SDL_USEREVENT);
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWindowWMInfo(window, &wminfo);
    if (wminfo.subsystem == SDL_SYSWM_WINDOWS) {
        _indev_handle_syswm_setup_win(&wminfo);
    }

    lv_sdl_init_scroll();
#endif
    lv_sdl_init_pointer();
}

void indev_handle_syswm_evt(SDL_SysWMEvent *event) {
    switch (event->msg->subsystem) {
#if __WIN32__
        case SDL_SYSWM_WINDOWS: {
            _indev_handle_syswm_evt_win(event);
            break;
        }
#endif
        default: {
            break;
        }
    }
}


static void indev_pointer_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void) drv;
    static SDL_Event e;
    data->continue_reading = SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEBUTTONUP) > 0;
    static lv_indev_state_t state = LV_INDEV_STATE_RELEASED;
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        state = LV_INDEV_STATE_PRESSED;
        data->point = (lv_point_t) {.x = e.button.x, .y = e.button.y};
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        state = LV_INDEV_STATE_RELEASED;
        data->point = (lv_point_t) {.x = e.button.x, .y = e.button.y};
    } else {
        data->point = (lv_point_t) {.x = e.motion.x, .y = e.motion.y};
    }
    data->state = state;
}

lv_indev_t *lv_sdl_init_pointer() {
    lv_indev_drv_t *indev_drv = malloc(sizeof(lv_indev_drv_t));
    lv_indev_drv_init(indev_drv);
    indev_drv->type = LV_INDEV_TYPE_POINTER;
    indev_drv->read_cb = indev_pointer_read;

    return lv_indev_drv_register(indev_drv);
}

#if HAS_SCROLL_EVENT
static void indev_scroll_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void) drv;
    static SDL_Event e;
    data->continue_reading = SDL_PeepEvents(&e, 1, SDL_GETEVENT, INDEV_SCROLL, INDEV_SCROLL) > 0;
    if (!data->continue_reading) {
        data->point.x = -1;
        data->point.y = -1;
        return;
    }
    data->scroll.x = e.motion.xrel;
    data->scroll.y = e.motion.yrel;
    data->point.x = e.motion.x;
    data->point.y = e.motion.y;
    data->state = e.motion.state == SDL_PRESSED ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

lv_indev_t *lv_sdl_init_scroll() {
    lv_indev_drv_t *indev_drv = malloc(sizeof(lv_indev_drv_t));
    lv_indev_drv_init(indev_drv);
    indev_drv->type = LV_INDEV_TYPE_SCROLL;
    indev_drv->scroll_limit = 0;
    indev_drv->read_cb = indev_scroll_read;

    return lv_indev_drv_register(indev_drv);
}
#endif