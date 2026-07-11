#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "data_files.h"
#include "map.h"
#include "vswap.h"

enum { WIDTH = 320, HEIGHT = 200, SCALE = 3 };

int main(int argc, char **argv)
{
    int check_only = 0;
    const char *data_directory = NULL;
    const char *edition = NULL;
    uint8_t wall[WALL_PIXELS];
    WolfMap map;

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

        if (!map_load_first(data_directory, edition, &map, error, sizeof(error))) {
            fprintf(stderr, "%s\n", error);
            return 1;
        }
        printf("Mapa 0 '%s' OK; jogador em %d,%d\n",
               map.name, map.player_x, map.player_y);
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
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
        pixels[i] = 0xff101018u;

    enum { MAP_SCALE = 3, MAP_X = (WIDTH - MAP_SIDE * MAP_SCALE) / 2,
           MAP_Y = (HEIGHT - MAP_SIDE * MAP_SCALE) / 2 };
    for (int y = 0; y < MAP_SIDE; ++y) {
        for (int x = 0; x < MAP_SIDE; ++x) {
            const uint16_t tile = map.planes[0][y * MAP_SIDE + x];
            uint32_t color = 0xff202028u;
            if (tile >= 90 && tile <= 101)
                color = 0xffffc020u;
            else if (tile && tile < 107)
                color = 0xffa0a0a8u;
            for (int py = 0; py < MAP_SCALE; ++py)
                for (int px = 0; px < MAP_SCALE; ++px)
                    pixels[(MAP_Y + y * MAP_SCALE + py) * WIDTH +
                           MAP_X + x * MAP_SCALE + px] = color;
        }
    }

    const int directions[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    const int player_x = MAP_X + map.player_x * MAP_SCALE + 1;
    const int player_y = MAP_Y + map.player_y * MAP_SCALE + 1;
    pixels[player_y * WIDTH + player_x] = 0xffff2020u;
    pixels[(player_y + directions[map.player_direction][1] * 2) * WIDTH +
           player_x + directions[map.player_direction][0] * 2] = 0xffffffffu;

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
