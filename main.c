#include <stdio.h>
#include <SDL.h>
#include <gpu/lv_gpu_sdl.h>

#include "lvgl.h"
#include "lv_demos/lv_demo.h"

static int running = SDL_TRUE;

static int app_event_filter(void *userdata, SDL_Event *event);

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

typedef struct lv_demo_entry_t {
    void (*action)();

    const char *title;
} lv_demo_entry_t;

static const lv_demo_entry_t demo_entries[4] = {
        {lv_demo_hw_accel,  "Hardware Accel Playground"},
        {lv_demo_widgets,   "Widgets"},
        {lv_demo_music,     "Music Player"},
        {lv_demo_benchmark, "Benchmark"},
//        {lv_demo_stress,    "Stress Test"},
};
static const size_t demo_entry_size = sizeof(demo_entries) / sizeof(lv_demo_entry_t);

static lv_obj_t *entry_screen;

void lv_demo_entry_handle_item(lv_event_t *e) {
    void (*action)() =(void (*)()) e->user_data;
    action();
    lv_obj_del(entry_screen);
    entry_screen = NULL;
}

void lv_demo_entry() {
    entry_screen = lv_win_create(lv_scr_act(), 40);
    lv_obj_t *content = lv_win_get_content(entry_screen);
    lv_win_add_title(entry_screen, "LVGL Demos");
    const static lv_coord_t cells_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(content, cells_dsc, cells_dsc);
    lv_obj_t *list = lv_list_create(content);
    lv_obj_set_grid_cell(list, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
//    lv_obj_set_size(list, lv_obj_get_content_width(lv_scr_act()), lv_obj_get_content_height(lv_scr_act()));
    for (int i = 0; i < demo_entry_size; i++) {
        lv_obj_t *item = lv_list_add_btn(list, LV_SYMBOL_DUMMY, demo_entries[i].title);
        lv_obj_add_event_cb(item, lv_demo_entry_handle_item, LV_EVENT_CLICKED, demo_entries[i].action);
    }
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    int width = 1280, height = 800;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                          SDL_WINDOW_ALLOW_HIGHDPI);
    lv_init();
    lv_disp_t *disp = lv_sdl_display_init(window);
    lv_sdl_init_pointer_input();

    lv_demo_entry();

    while (running) {
        static Uint32 fps_ticks = 0, framecount = 0;

        SDL_PumpEvents();
        SDL_FilterEvents(app_event_filter, NULL);
        lv_task_handler();
        SDL_Delay(1);

        Uint32 end_ticks = SDL_GetTicks();
        if ((end_ticks - fps_ticks) >= 1000) {
            static char wintitle[64];
            snprintf(wintitle, 64, "%s | %d FPS", WINDOW_TITLE, (int) (framecount * 1000.0 / (end_ticks - fps_ticks)));
            SDL_SetWindowTitle(window, wintitle);
            fps_ticks = end_ticks;
            framecount = 0;
        } else {
            framecount++;
        }
    }
    lv_sdl_deinit_pointer_input();
    lv_sdl_display_deinit(disp);
//    lv_deinit();
    SDL_DestroyWindow(window);
    return 0;
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
        case SDL_WINDOWEVENT: {
            switch (event->window.event) {
                case SDL_WINDOWEVENT_EXPOSED: {
                    lv_obj_invalidate(lv_scr_act());
                    break;
                }
            }
            break;
        }
//        case SDL_RENDER_TARGETS_RESET: {
//            lv_disp_t *disp = lv_disp_get_default();
//            SDL_RenderClear((SDL_Renderer *) lv_disp_get_draw_buf(disp)->buf_act);
//            lv_refr_now(disp);
//            break;
//        }
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