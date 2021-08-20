#include <stdio.h>
#include <SDL2/SDL.h>

#include "lvgl.h"

static int running = SDL_TRUE;

lv_disp_t *lv_sdl_display_init();

void lv_sdl_display_deinit(lv_disp_t *);

#define WINDOW_TITLE "LVGL Sample"

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    int width = 1920, height = 1080;
    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                          SDL_WINDOW_ALLOW_HIGHDPI);
    lv_init();
    lv_disp_t *disp = lv_sdl_display_init(window, width, height);

    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *labels[40 * 22];
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 22; j++) {
            lv_obj_t *label = lv_label_create(scr);
            lv_obj_set_size(label, 40, 40);
            lv_obj_set_pos(label, i * 48, j * 48);
            labels[i * 22 + j] = label;
        }
    }

    while (running) {

        static Uint32 fps_ticks = 0, framecount = 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;
            }
        }

        for (int i = 0; i < 40; i++) {
            for (int j = 0; j < 22; j++) {
                lv_obj_t *label = labels[i * 22 + j];
                lv_label_set_text_fmt(label, "%d", SDL_GetTicks() % 1000);
            }
        }

        lv_task_handler();
//        SDL_RenderClear(renderer);
//        SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
        SDL_RenderPresent((SDL_Renderer *) disp->driver->draw_buf->buf_act);


        Uint32 end_ticks = SDL_GetTicks();
        if ((end_ticks - fps_ticks) >= 1000) {
            static char wintitle[64];
            sprintf(wintitle, "%s | %d FPS", WINDOW_TITLE, (int) (framecount * 1000.0 / (end_ticks - fps_ticks)));
            SDL_SetWindowTitle(window, wintitle);
            fps_ticks = end_ticks;
            framecount = 0;
        } else {
            framecount++;
        }
    }
    lv_sdl_display_deinit(disp);
//    lv_deinit();
    SDL_DestroyWindow(window);
    return 0;
}

static void display_wait_cb(lv_disp_drv_t *disp_drv) {
    (void) disp_drv;
    //OPTIONAL: Called periodically while lvgl waits for an operation to be completed
    //          User can execute very simple tasks here or yield the task
}

static void sdl_fb_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_pixels_t src) {

    if (area->x2 < 0 || area->y2 < 0 ||
        area->x1 > disp_drv->hor_res - 1 || area->y1 > disp_drv->ver_res - 1) {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    SDL_Rect r;
    r.x = area->x1;
    r.y = area->y1;
    r.w = area->x2 - area->x1 + 1;
    r.h = area->y2 - area->y1 + 1;

    lv_disp_flush_ready(disp_drv);
}

lv_disp_t *lv_sdl_display_init(SDL_Window *window, int width, int height) {
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    lv_disp_draw_buf_t *buf = malloc(sizeof(lv_disp_draw_buf_t));
    lv_disp_draw_buf_init(buf, renderer, NULL, width * height);
    lv_disp_drv_t *driver = malloc(sizeof(lv_disp_drv_t));
    lv_disp_drv_init(driver);
    driver->draw_buf = buf;
    driver->wait_cb = display_wait_cb;
    driver->flush_cb = sdl_fb_flush;
    driver->hor_res = width;
    driver->ver_res = height;
    driver->user_data = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                          width, height);

    return lv_disp_drv_register(driver);
}

void lv_sdl_display_deinit(lv_disp_t *disp) {
    SDL_DestroyTexture((SDL_Texture *) disp->driver->user_data);
    SDL_DestroyRenderer((SDL_Renderer *) disp->driver->draw_buf->buf1);
    free(disp->driver->draw_buf);
    free(disp->driver);
}