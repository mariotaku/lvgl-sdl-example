#include <stdio.h>
#include <SDL.h>

#include "gpu/lv_gpu_sdl.h"
#include "lvgl.h"
#include "lv_demos/lv_demo.h"
#include "lvgl/examples/lv_examples.h"
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
            lv_obj_t *item = NULL;
            switch (lv_rand(0, 3)) {
                case 1:
                    item = lv_spinner_create(scr, 10000, 60);
                    break;
                case 2: {
                    item = lv_obj_create(scr);
                    lv_obj_set_style_radius(item, LV_RADIUS_CIRCLE, 0);
                    lv_obj_set_style_clip_corner(item, true, 0);
                    lv_obj_t *label = lv_label_create(item);
                    lv_label_set_text(label, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                                             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                                             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                                             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                                             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                                             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                                             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
                                             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
                    break;
                }
                case 3: {
                    item = lv_obj_create(scr);
                    lv_obj_set_style_bg_color(item, lv_palette_main(LV_PALETTE_RED), 0);
                    lv_obj_set_style_radius(item, LV_RADIUS_CIRCLE, 0);
                    lv_obj_set_style_clip_corner(item, true, 0);
                    lv_obj_t *obj2 = lv_obj_create(item);
                    lv_obj_set_pos(obj2, 30, 30);
                    lv_obj_set_style_bg_color(obj2, lv_palette_main(LV_PALETTE_BLUE), 0);
                    break;
                }
                default: {
                    item = lv_btn_create(scr);
                    lv_obj_t *label = lv_label_create(item);
                    lv_label_set_text_fmt(label, "Item %d", row * cols + col);
                    break;
                }
            }
            SDL_assert(item);
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

static const lv_demo_entry_t demo_entries[] = {
        {NULL,                "Demos"},
        {lv_demo_widgets,     "Widgets"},
        {lv_demo_music,       "Music Player"},
        {lv_demo_benchmark,   "Benchmark"},
        {lv_demo_stress,      "Stress Test"},
        {lv_demo_hw_accel,    "HW Accel Playground"},
        {NULL,                "Arc Examples"},
        {lv_example_arc_1,    "Arc 1"},
        {lv_example_arc_2,    "Arc 2"},
        {NULL,                "Chart Examples"},
        {lv_example_chart_1,  "Chart 1"},
        {lv_example_chart_2,  "Chart 2"},
        {lv_example_chart_3,  "Chart 3"},
        {lv_example_chart_4,  "Chart 4"},
        {lv_example_chart_5,  "Chart 5"},
        {lv_example_chart_6,  "Chart 6"},
        {lv_example_chart_7,  "Chart 7"},
        {NULL,                "Label Examples"},
        {lv_example_label_1,  "Line wrap, re-color, line align and text scrolling"},
        {lv_example_label_2,  "Fake Shadow"},
        {lv_example_label_3,  "LTR, RTL and Chinese label"},
        {NULL,                "Scroll Examples"},
        {lv_example_scroll_6, "Scroll 6"},
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
        lv_demo_entry_t entry = demo_entries[i];
        if (entry.action) {
            lv_obj_t *item = lv_list_add_btn(list, LV_SYMBOL_DUMMY, entry.title);
            lv_obj_add_event_cb(item, lv_demo_entry_handle_item, LV_EVENT_CLICKED, entry.action);
        } else {
            lv_list_add_text(list, entry.title);
        }
    }
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(0, &mode);
    mode.w = 1280;
    mode.h = 800;
    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          mode.w, mode.h, SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    lv_init();
    lv_disp_t *disp = lv_sdl_display_init(window);
    disp->theme->font_small = &lv_font_montserrat_12;
    disp->theme->font_large = &lv_font_montserrat_16;
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