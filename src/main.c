#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "data_files.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "render.h"
#include "vswap.h"

enum { SCALE = 3 };

int main(int argc, char **argv)
{
    int check_only = 0;
    const char *data_directory = NULL;
    const char *edition = NULL;
    char error[512];
    int map_index = 0;
    VSwap vswap = {0};
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
        FILE *vswap_file = fopen(path, "rb");
        if (!vswap_file ||
            !vswap_load(vswap_file, &vswap, error, sizeof(error))) {
            fprintf(stderr, "%s: %s\n", path,
                    vswap_file ? error : "não foi possível abrir");
            if (vswap_file)
                fclose(vswap_file);
            return 1;
        }
        fclose(vswap_file);
        printf("%zu paredes e %zu sprites VSWAP OK\n",
               vswap.walls.count, vswap.sprite_count);

        WolfMap map;
        if (!map_load(data_directory, edition, 0, &map, error, sizeof(error))) {
            fprintf(stderr, "%s\n", error);
            vswap_free(&vswap);
            return 1;
        }
        printf("Mapa 0 '%s' OK; jogador em %d,%d\n",
               map.name, map.player_x, map.player_y);
        game_init(&game, &map, 1);
    }

    const uint32_t flags = check_only ? SDL_INIT_TIMER : SDL_INIT_VIDEO;

    if (SDL_Init(flags) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        vswap_free(&vswap);
        return 1;
    }
    if (check_only) {
        puts("SDL2 OK");
        vswap_free(&vswap);
        SDL_Quit();
        return 0;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Wolfenstein 3D - Linux port",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        RENDER_WIDTH * SCALE, RENDER_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = window ? SDL_CreateRenderer(window, -1, 0) : NULL;
    SDL_Texture *texture = renderer ? SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH, RENDER_HEIGHT) : NULL;

    if (!window || !renderer || !texture) {
        fprintf(stderr, "SDL video: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        vswap_free(&vswap);
        SDL_Quit();
        return 1;
    }
    if (SDL_SetRelativeMouseMode(SDL_TRUE) < 0) {
        fprintf(stderr, "SDL mouse: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        vswap_free(&vswap);
        SDL_Quit();
        return 1;
    }

    uint32_t pixels[RENDER_WIDTH * RENDER_HEIGHT];
    uint64_t previous = SDL_GetPerformanceCounter();
    const double frequency = (double)SDL_GetPerformanceFrequency();

    for (int running = 1; running;) {
        SDL_Event event;
        int mouse_x = 0;
        int use_pressed = 0;
        int attack_pressed = 0;
        int requested_weapon = 0;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEMOTION)
                mouse_x += event.motion.xrel;
            if (event.type == SDL_KEYDOWN && !event.key.repeat &&
                event.key.keysym.sym == SDLK_SPACE)
                use_pressed = 1;
            if ((event.type == SDL_KEYDOWN && !event.key.repeat &&
                 (event.key.keysym.sym == SDLK_LCTRL ||
                  event.key.keysym.sym == SDLK_RCTRL)) ||
                (event.type == SDL_MOUSEBUTTONDOWN &&
                 event.button.button == SDL_BUTTON_LEFT))
                attack_pressed = 1;
            if (event.type == SDL_KEYDOWN && !event.key.repeat &&
                event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_4)
                requested_weapon = event.key.keysym.sym - SDLK_1 + 1;
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                running = 0;
        }

        const uint64_t now = SDL_GetPerformanceCounter();
        const double seconds = (now - previous) / frequency;
        previous = now;
        const uint8_t *keys = SDL_GetKeyboardState(NULL);
        const Uint32 mouse_buttons = SDL_GetMouseState(NULL, NULL);
        const PlayerCommand command = {
            .forward = (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) -
                       (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]),
            .turn = (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) -
                    (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]),
            .look_radians = mouse_x * 0.0025,
            .use_pressed = use_pressed,
            .attack_pressed = attack_pressed,
            .attack_held = keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL] ||
                           (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)),
            .requested_weapon = requested_weapon
        };
        game_update(&game, &command, seconds);
        if (game.level_complete) {
            WolfMap map;
            const int next_map = map_index + 1;
            if (!map_load(data_directory, edition, next_map, &map,
                          error, sizeof(error))) {
                fprintf(stderr, "%s\n", error);
                running = 0;
                continue;
            }
            map_index = next_map;
            game_next_level(&game, &map, (uint32_t)map_index + 1);
            printf("Mapa %d '%s' OK; jogador em %d,%d\n", map_index,
                   map.name, map.player_x, map.player_y);
        }
        render_scene(pixels, &game, &vswap);
        SDL_UpdateTexture(texture, NULL, pixels,
                          RENDER_WIDTH * (int)sizeof(*pixels));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    vswap_free(&vswap);
    SDL_Quit();
    return 0;
}
