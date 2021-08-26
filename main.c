#include <stdio.h>
#include <SDL.h>

#include "gpu/lv_gpu_sdl.h"
#include "lvgl.h"
#include "lv_demos/lv_demo.h"
#include "input_drv/indev_sdl.h"


static int running = SDL_TRUE;

static int app_event_filter(void *userdata, SDL_Event *event);

lv_indev_t *lv_sdl_init_pointer(void);

lv_indev_t *lv_sdl_init_scroll(void);

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

static void lv_demo_long_list() {
    lv_obj_t *win = lv_win_create(lv_scr_act(), 40);
    lv_win_add_title(win, "Long List");
    lv_obj_t *content = lv_win_get_content(win);
    const static lv_coord_t cells_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(content, cells_dsc, cells_dsc);
    lv_obj_t *list = lv_list_create(content);
    lv_obj_set_grid_cell(list, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    for (int i = 0; i < 200; i++) {
        lv_list_add_btn(list, LV_SYMBOL_DUMMY, "Item");
    }
}

static void lv_demo_unicode() {
    lv_obj_t *win = lv_win_create(lv_scr_act(), 40);
    lv_win_add_title(win, "Unicode Font Support");
    lv_obj_t *content = lv_win_get_content(win);
    lv_obj_t *ltr_label = lv_label_create(content);
    lv_label_set_text(ltr_label, "In modern terminology, a microcontroller is similar to a system on a chip (SoC).");
    lv_obj_set_style_text_font(ltr_label, &lv_font_montserrat_16, 0);
    lv_obj_set_width(ltr_label, 310);
    lv_obj_align(ltr_label, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t *rtl_label = lv_label_create(content);
    lv_label_set_text(rtl_label, "מעבד, או בשמו המלא יחידת עיבוד מרכזית (באנגלית: CPU - Central Processing Unit).");
    lv_obj_set_style_base_dir(rtl_label, LV_BASE_DIR_RTL, 0);
    lv_obj_set_style_text_font(rtl_label, &lv_font_dejavu_16_persian_hebrew, 0);
    lv_obj_set_width(rtl_label, 310);
    lv_obj_align(rtl_label, LV_ALIGN_LEFT_MID, 5, 0);

    lv_obj_t *cz_label = lv_label_create(content);
    lv_label_set_text(cz_label, "嵌入式系统（Embedded System），\n是一种嵌入机械或电气系统内部、具有专一功能和实时计算性能的计算机系统。");
    lv_obj_set_style_text_font(cz_label, &lv_font_simsun_16_cjk, 0);
    lv_obj_set_width(cz_label, 310);
    lv_obj_align(cz_label, LV_ALIGN_BOTTOM_LEFT, 5, -5);
}

typedef struct lv_demo_entry_t {
    void (*action)();

    const char *title;
} lv_demo_entry_t;

static const lv_demo_entry_t demo_entries[] = {
        {lv_demo_long_list, "Long List"},
        {lv_demo_hw_accel,  "Hardware Accel Playground"},
        {lv_demo_widgets,   "Widgets"},
        {lv_demo_music,     "Music Player"},
        {lv_demo_benchmark, "Benchmark"},
        {lv_demo_unicode,   "Unicode Fonts"},
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
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    lv_init();
    lv_disp_t *disp = lv_sdl_display_init(window);
    indev_init(window);

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
    lv_sdl_display_deinit(disp);
//    lv_deinit();
    SDL_DestroyWindow(window);
    return 0;
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
        case SDL_SYSWMEVENT: {
            indev_handle_syswm_evt(&event->syswm);
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
        case SDL_MOUSEWHEEL: {
//            SDL_Log("SDL_MOUSEWHEEL: mw scroll %d, %d\n", event->wheel.x, event->wheel.y);
            break;
        }
//        case SDL_KEYDOWN:
//        case SDL_KEYUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case INDEV_SCROLL:
//        case SDL_MOUSEWHEEL:
            return 1;
        default: {
            return 0;
        }
    }
    return 0;
}