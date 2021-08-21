#include <stdio.h>
#include <SDL.h>
#include <gpu/lv_gpu_sdl2_render.h>

#include "lvgl.h"
#include "lv_demos/lv_demo.h"

static int running = SDL_TRUE;

lv_disp_t *lv_sdl_display_init();

static int app_event_filter(void *userdata, SDL_Event *event);

void lv_sdl_display_deinit(lv_disp_t *);

lv_indev_t *lv_sdl_init_pointer_input(void);

void lv_sdl_deinit_pointer_input(void);

#define WINDOW_TITLE "LVGL Sample [HW]"


static void lv_demo_hw_accel() {
    lv_obj_t *scr = lv_scr_act();
#define cols 16
#define rows 9
    lv_obj_t *labels[cols * rows];
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            lv_obj_t *item;
            switch (lv_rand(0, 1)) {
                case 1:
                    item = lv_spinner_create(scr, 1000, 60);
                    break;
                default: {
                    item = lv_btn_create(scr);
                    lv_obj_t *label = lv_label_create(item);
                    lv_label_set_text_fmt(label, "Item %d", row * cols + col);
                    break;
                }
            }

            lv_obj_set_size(item, 100, 100);
            lv_obj_set_pos(item, col * 120 + 10, row * 120 + 10);
            labels[row * cols + col] = item;
        }
    }
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    int width = 1280, height = 720;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                          SDL_WINDOW_ALLOW_HIGHDPI);
    lv_init();
    lv_disp_t *disp = lv_sdl_display_init(window, width, height);
    lv_sdl_init_pointer_input();
    lv_gpu_sdl2_renderer_init();

//    lv_demo_hw_accel();
    lv_demo_widgets();
//    lv_demo_music();

    while (running) {

        static Uint32 fps_ticks = 0, framecount = 0;

        SDL_PumpEvents();
        SDL_FilterEvents(app_event_filter, NULL);
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
    lv_gpu_sdl2_renderer_deinit();
    lv_sdl_deinit_pointer_input();
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
    driver->user_data = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STREAMING,
                                          width, height);

    return lv_disp_drv_register(driver);
}

void lv_sdl_display_deinit(lv_disp_t *disp) {
    SDL_DestroyTexture((SDL_Texture *) disp->driver->user_data);
    SDL_DestroyRenderer((SDL_Renderer *) disp->driver->draw_buf->buf1);
    free(disp->driver->draw_buf);
    free(disp->driver);
}


static void sdl_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void) drv;
    static SDL_Event e;
    data->continue_reading = SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEBUTTONUP) > 0;
    static lv_indev_state_t state = LV_INDEV_STATE_REL;
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        state = LV_INDEV_STATE_PR;
        data->point = (lv_point_t) {.x = e.button.x, .y = e.button.y};
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        state = LV_INDEV_STATE_REL;
        data->point = (lv_point_t) {.x = e.button.x, .y = e.button.y};
    } else {
        data->point = (lv_point_t) {.x = e.motion.x, .y = e.motion.y};
    }
    data->state = state;
}

lv_indev_t *lv_sdl_init_pointer_input(void) {
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_input_read;

    return lv_indev_drv_register(&indev_drv);
}

void lv_sdl_deinit_pointer_input(void) {
}

static int app_event_filter(void *userdata, SDL_Event *event) {
    switch (event->type) {
        case SDL_USEREVENT: {
            break;
        }
        case SDL_QUIT: {
            running = SDL_FALSE;
            break;
        }
//        case SDL_KEYDOWN:
//        case SDL_KEYUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
//        case SDL_MOUSEWHEEL:
            return 1;
        default:
            return 0;
    }
    return 0;
}