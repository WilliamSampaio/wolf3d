#include "render.h"
#include "game_palette.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void make_map(WolfMap *map)
{
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_SIDE; ++y)
        for (int x = 0; x < MAP_SIDE; ++x)
            map->planes[0][y * MAP_SIDE + x] = x == 0 || y == 0 ||
                                                  x == MAP_SIDE - 1 ||
                                                  y == MAP_SIDE - 1
                                              ? 1 : 107;
    map->planes[1][2 * MAP_SIDE + 4] = 23;
}

int main(void)
{
    assert(game_palette[0] == 0 && game_palette[1] == 0 &&
           game_palette[2] == 0);
    assert(game_palette[3] == 0 && game_palette[4] == 0 &&
           game_palette[5] == 0x2a);
    assert(game_palette[765] == 0x26 && game_palette[766] == 0 &&
           game_palette[767] == 0x22);
    WolfMap map;
    make_map(&map);
    uint8_t wall[WALL_PIXELS];
    memset(wall, 10, sizeof(wall));
    VSwapSprite sprites[426] = {0};
    memset(sprites[2].pixels, 20, sizeof(sprites[2].pixels));
    memset(sprites[2].mask, 1, sizeof(sprites[2].mask));
    sprites[2].right = WALL_SIZE - 1;
    for (int index = 50; index < 58; ++index) {
        memset(sprites[index].pixels, index, sizeof(sprites[index].pixels));
        memset(sprites[index].mask, 1, sizeof(sprites[index].mask));
        sprites[index].right = WALL_SIZE - 1;
    }
    for (int index = 58; index < 90; ++index) {
        memset(sprites[index].pixels, index, sizeof(sprites[index].pixels));
        memset(sprites[index].mask, 1, sizeof(sprites[index].mask));
        sprites[index].right = WALL_SIZE - 1;
    }
    for (int index = 90; index < 96; ++index) {
        memset(sprites[index].pixels, index, sizeof(sprites[index].pixels));
        memset(sprites[index].mask, 1, sizeof(sprites[index].mask));
        sprites[index].right = WALL_SIZE - 1;
    }
    for (int index = 96; index < 99; ++index) {
        memset(sprites[index].pixels, 20 + (index - 96) * 20,
               sizeof(sprites[index].pixels));
        memset(sprites[index].mask, 1, sizeof(sprites[index].mask));
        sprites[index].right = WALL_SIZE - 1;
    }
    VSwap vswap = {
        .walls = {1, wall},
        .sprite_count = 426,
        .sprites = sprites
    };
    GameState game = {0};
    game.map = map;
    game.player = (Player){2.5, 2.5, 0.0};
    for (int cell = 0; cell < MAP_CELLS; ++cell)
        game.door_at[cell] = -1;
    game.static_count = 1;
    game.statics[0] = (StaticObject){4, 2, 2, 1, STATIC_DECORATION};
    uint32_t pixels[RENDER_WIDTH * RENDER_HEIGHT];

    game.map.planes[0][2 * MAP_SIDE + 3] = 1;
    render_scene(pixels, &game, &vswap);
    const uint32_t hidden = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                   RENDER_WIDTH / 2];
    game.map.planes[0][2 * MAP_SIDE + 3] = 107;
    render_scene(pixels, &game, &vswap);
    const uint32_t visible = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                    RENDER_WIDTH / 2];
    assert(hidden != visible);
    game.statics[0].active = 0;
    render_scene(pixels, &game, &vswap);
    const uint32_t removed = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                    RENDER_WIDTH / 2];
    assert(removed != visible);

    game.guard_count = 1;
    game.guards[0] = (Guard){.x = 4.5, .y = 2.5, .active = 1};
    render_scene(pixels, &game, &vswap);
    const uint32_t guard_east = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                       RENDER_WIDTH / 2];
    game.guards[0].direction = 1;
    render_scene(pixels, &game, &vswap);
    const uint32_t guard_north = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                        RENDER_WIDTH / 2];
    assert(guard_east != guard_north);
    game.map.planes[0][2 * MAP_SIDE + 3] = 1;
    render_scene(pixels, &game, &vswap);
    const uint32_t guard_hidden = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                         RENDER_WIDTH / 2];
    assert(guard_hidden != guard_north);
    game.map.planes[0][2 * MAP_SIDE + 3] = 107;
    game.guards[0].patrol = 1;
    game.guards[0].frame = 0;
    render_scene(pixels, &game, &vswap);
    const uint32_t walk_one = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                     RENDER_WIDTH / 2];
    game.guards[0].frame = 1;
    render_scene(pixels, &game, &vswap);
    const uint32_t walk_two = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                     RENDER_WIDTH / 2];
    assert(walk_one != walk_two);

    for (int index = 50; index < 58; ++index)
        memset(sprites[index].mask, 0, sizeof(sprites[index].mask));
    memset(sprites[56].mask, 1, sizeof(sprites[56].mask));
    game.player = (Player){4.5, 6.5, -1.5707963267948966};
    game.guards[0] = (Guard){.x = 4.5, .y = 4.5, .active = 1};
    render_scene(pixels, &game, &vswap);
    const uint32_t south_view = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                       RENDER_WIDTH / 2];
    game.guards[0].active = 0;
    render_scene(pixels, &game, &vswap);
    assert(south_view != pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                RENDER_WIDTH / 2]);

    game.guards[0] = (Guard){.x = 4.5, .y = 4.5, .active = 1, .dead = 1};
    render_scene(pixels, &game, &vswap);
    const uint32_t dying = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                  RENDER_WIDTH / 2];
    game.guards[0].death_seconds = 1.0;
    render_scene(pixels, &game, &vswap);
    const uint32_t corpse = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                   RENDER_WIDTH / 2];
    assert(dying != corpse);

    game.guards[0].active = 0;
    game.current_weapon = WEAPON_PISTOL;
    render_scene(pixels, &game, &vswap);
    const uint32_t no_weapon = pixels[(RENDER_HEIGHT - 21) * RENDER_WIDTH +
                                      RENDER_WIDTH / 2];
    memset(sprites[421].pixels, 120, sizeof(sprites[421].pixels));
    memset(sprites[421].mask, 1, sizeof(sprites[421].mask));
    sprites[421].right = WALL_SIZE - 1;
    render_scene(pixels, &game, &vswap);
    const uint32_t ready_weapon = pixels[(RENDER_HEIGHT - 21) * RENDER_WIDTH +
                                         RENDER_WIDTH / 2];
    assert(no_weapon != ready_weapon);
    memset(sprites[422].pixels, 121, sizeof(sprites[422].pixels));
    memset(sprites[422].mask, 1, sizeof(sprites[422].mask));
    sprites[422].right = WALL_SIZE - 1;
    game.weapon_frame = 1;
    render_scene(pixels, &game, &vswap);
    assert(ready_weapon != pixels[(RENDER_HEIGHT - 21) * RENDER_WIDTH +
                                  RENDER_WIDTH / 2]);

    game.current_weapon = -1;
    game.guards[0] = (Guard){.x = 4.5, .y = 4.5, .active = 1, .shooting = 1};
    render_scene(pixels, &game, &vswap);
    const uint32_t shoot_one = pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                                      RENDER_WIDTH / 2];
    game.guards[0].shoot_frame = 1;
    render_scene(pixels, &game, &vswap);
    assert(shoot_one != pixels[(RENDER_HEIGHT / 2) * RENDER_WIDTH +
                               RENDER_WIDTH / 2]);

    game.guards[0].active = 0;
    game.damage_seconds = 0.0;
    game.health = 100;
    game.ammo = 8;
    game.lives = 3;
    render_scene(pixels, &game, &vswap);
    uint64_t hud_before = 0;
    for (int index = (RENDER_HEIGHT - 20) * RENDER_WIDTH;
         index < RENDER_HEIGHT * RENDER_WIDTH; ++index)
        hud_before = hud_before * 33u + pixels[index];
    game.health = 99;
    render_scene(pixels, &game, &vswap);
    uint64_t hud_after = 0;
    for (int index = (RENDER_HEIGHT - 20) * RENDER_WIDTH;
         index < RENDER_HEIGHT * RENDER_WIDTH; ++index)
        hud_after = hud_after * 33u + pixels[index];
    assert(hud_before != hud_after);
    const uint32_t normal_view = pixels[0];
    game.damage_seconds = 0.1;
    render_scene(pixels, &game, &vswap);
    assert(normal_view != pixels[0]);
    game.player_dead = 1;
    game.player_death_seconds = 1.0;
    render_scene(pixels, &game, &vswap);
    assert(pixels[0] == 0xff000000u);
    puts("RENDER OK");
    return 0;
}
