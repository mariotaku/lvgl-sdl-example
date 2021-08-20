#include <stdio.h>
#include <SDL2/SDL.h>

static int running = SDL_TRUE;

#define WINDOW_TITLE "LVGL Sample"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
                                          SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    while (running) {

        static Uint32 fps_ticks = 0, framecount = 0;
        Uint32 start_ticks = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;
            }
        }
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        Uint32 end_ticks = SDL_GetTicks();
        Sint32 deltams = end_ticks - start_ticks;
        if (deltams < 0) {
            deltams = 0;
        }
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
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
