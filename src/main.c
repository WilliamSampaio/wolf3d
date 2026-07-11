#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { WIDTH = 320, HEIGHT = 200, SCALE = 3 };

int main(int argc, char **argv)
{
    const int check_only = argc == 2 && strcmp(argv[1], "--check") == 0;
    const uint32_t flags = check_only ? SDL_INIT_TIMER : SDL_INIT_VIDEO;

    if (SDL_Init(flags) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (check_only) {
        puts("SDL2 OK");
        SDL_Quit();
        return 0;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Wolfenstein 3D - Linux port",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = window ? SDL_CreateRenderer(window, -1, 0) : NULL;
    SDL_Texture *texture = renderer ? SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT) : NULL;

    if (!window || !renderer || !texture) {
        fprintf(stderr, "SDL video: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    uint32_t pixels[WIDTH * HEIGHT];
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            pixels[y * WIDTH + x] = 0xff000000u | (uint32_t)(x * 255 / WIDTH) << 16
                                  | (uint32_t)(y * 255 / HEIGHT) << 8;

    for (int running = 1; running;) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                running = 0;

        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * (int)sizeof(*pixels));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
