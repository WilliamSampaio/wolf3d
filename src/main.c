#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "data_files.h"
#include "game_palette.h"
#include "vswap.h"

enum { WIDTH = 320, HEIGHT = 200, SCALE = 3 };

int main(int argc, char **argv)
{
    int check_only = 0;
    const char *data_directory = NULL;
    const char *edition = NULL;
    uint8_t wall[WALL_PIXELS];

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--check") == 0)
            check_only = 1;
        else if (strcmp(argv[i], "--data") == 0 && i + 1 < argc)
            data_directory = argv[++i];
        else {
            fprintf(stderr, "uso: %s [--check] [--data DIRETORIO]\n", argv[0]);
            return 2;
        }
    }

    if (!check_only && !data_directory) {
        fprintf(stderr, "uso: %s --data DIRETORIO\n", argv[0]);
        return 2;
    }
    if (data_directory) {
        char error[512];
        edition = data_validate(data_directory, error, sizeof(error));
        if (!edition) {
            fprintf(stderr, "%s\n", error);
            return 1;
        }
        printf("Dados Wolfenstein 3D %s OK\n", edition);

        char path[4096];
        const int path_length = snprintf(path, sizeof(path), "%s/VSWAP.%s",
                                         data_directory, edition);
        if (path_length < 0 || (size_t)path_length >= sizeof(path)) {
            fprintf(stderr, "caminho dos dados é longo demais\n");
            return 1;
        }
        FILE *vswap = fopen(path, "rb");
        if (!vswap || !vswap_read_first_wall(vswap, wall, error, sizeof(error))) {
            fprintf(stderr, "%s: %s\n", path, vswap ? error : "não foi possível abrir");
            if (vswap)
                fclose(vswap);
            return 1;
        }
        fclose(vswap);
    }

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
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            const uint8_t color = wall[(y % WALL_SIZE) * WALL_SIZE + x % WALL_SIZE];
            const uint8_t *rgb = &game_palette[color * 3];
            pixels[y * WIDTH + x] = 0xff000000u | (uint32_t)(rgb[0] * 255 / 63) << 16
                                  | (uint32_t)(rgb[1] * 255 / 63) << 8
                                  | (uint32_t)(rgb[2] * 255 / 63);
        }
    }

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
