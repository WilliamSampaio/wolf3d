#include <SDL2/SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "data_files.h"
#include "game.h"
#include "game_palette.h"
#include "map.h"
#include "player.h"
#include "raycast.h"
#include "vswap.h"

enum { WIDTH = 320, HEIGHT = 200, SCALE = 3 };

static uint32_t palette_color(uint8_t index, int brightness)
{
    const uint8_t *rgb = &game_palette[index * 3];
    return 0xff000000u | (uint32_t)(rgb[0] * 255 / 63 * brightness / 4) << 16 |
           (uint32_t)(rgb[1] * 255 / 63 * brightness / 4) << 8 |
           (uint32_t)(rgb[2] * 255 / 63 * brightness / 4);
}

static void render_scene(uint32_t pixels[WIDTH * HEIGHT], const WolfMap *map,
                         const VSwapWalls *walls, const Player *player)
{
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            pixels[y * WIDTH + x] = palette_color(y < HEIGHT / 2 ? 0x1d : 0x19, 4);

    const double direction_x = cos(player->angle);
    const double direction_y = sin(player->angle);
    const double plane_x = -direction_y * 0.66;
    const double plane_y = direction_x * 0.66;

    for (int x = 0; x < WIDTH; ++x) {
        const double camera_x = 2.0 * x / WIDTH - 1.0;
        const double ray_x = direction_x + plane_x * camera_x;
        const double ray_y = direction_y + plane_y * camera_x;
        RayHit hit;
        if (!raycast_hit(map->planes[0], player->x, player->y, ray_x, ray_y, &hit))
            continue;

        size_t page = 0;
        if (hit.tile < 64)
            page = (hit.tile - 1) * 2 + (hit.side == 0);
        else if (hit.tile >= 90 && hit.tile <= 101 && walls->count >= 8)
            page = walls->count - 8 + (hit.side == 0);
        if (page >= walls->count)
            page = 0;

        int texture_x = (int)(hit.wall_position * WALL_SIZE);
        if ((hit.side == 0 && ray_x > 0.0) || (hit.side == 1 && ray_y < 0.0))
            texture_x = WALL_SIZE - 1 - texture_x;

        const int line_height = (int)(HEIGHT / hit.distance);
        const int line_start = (HEIGHT - line_height) / 2;
        int draw_start = line_start < 0 ? 0 : line_start;
        int draw_end = line_start + line_height;
        if (draw_end > HEIGHT)
            draw_end = HEIGHT;
        const uint8_t *texture = walls->pixels + page * WALL_PIXELS;
        for (int y = draw_start; y < draw_end; ++y) {
            const int texture_y = (y - line_start) * WALL_SIZE / line_height;
            pixels[y * WIDTH + x] = palette_color(
                texture[texture_y * WALL_SIZE + texture_x], hit.side ? 3 : 4);
        }
    }
}

int main(int argc, char **argv)
{
    int check_only = 0;
    const char *data_directory = NULL;
    const char *edition = NULL;
    VSwapWalls walls = {0};
    GameState game;

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
        if (!vswap || !vswap_load_walls(vswap, &walls, error, sizeof(error))) {
            fprintf(stderr, "%s: %s\n", path, vswap ? error : "não foi possível abrir");
            if (vswap)
                fclose(vswap);
            return 1;
        }
        fclose(vswap);
        printf("%zu páginas de parede VSWAP OK\n", walls.count);

        WolfMap map;
        if (!map_load(data_directory, edition, 0, &map, error, sizeof(error))) {
            fprintf(stderr, "%s\n", error);
            return 1;
        }
        printf("Mapa 0 '%s' OK; jogador em %d,%d\n",
               map.name, map.player_x, map.player_y);
        game_init(&game, &map, 1);
    }

    const uint32_t flags = check_only ? SDL_INIT_TIMER : SDL_INIT_VIDEO;

    if (SDL_Init(flags) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (check_only) {
        puts("SDL2 OK");
        vswap_free_walls(&walls);
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
    if (SDL_SetRelativeMouseMode(SDL_TRUE) < 0) {
        fprintf(stderr, "SDL mouse: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        vswap_free_walls(&walls);
        SDL_Quit();
        return 1;
    }

    uint32_t pixels[WIDTH * HEIGHT];
    uint64_t previous = SDL_GetPerformanceCounter();
    const double frequency = (double)SDL_GetPerformanceFrequency();

    for (int running = 1; running;) {
        SDL_Event event;
        int mouse_x = 0;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEMOTION)
                mouse_x += event.motion.xrel;
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                running = 0;
        }

        const uint64_t now = SDL_GetPerformanceCounter();
        const double seconds = (now - previous) / frequency;
        previous = now;
        const uint8_t *keys = SDL_GetKeyboardState(NULL);
        const PlayerCommand command = {
            .forward = (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) -
                       (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]),
            .turn = (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) -
                    (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]),
            .look_radians = mouse_x * 0.0025
        };
        game_update(&game, &command, seconds);
        render_scene(pixels, &game.map, &walls, &game.player);
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * (int)sizeof(*pixels));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    vswap_free_walls(&walls);
    SDL_Quit();
    return 0;
}
