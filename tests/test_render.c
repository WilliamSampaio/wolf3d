#include "render.h"

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
    WolfMap map;
    make_map(&map);
    uint8_t wall[WALL_PIXELS];
    memset(wall, 10, sizeof(wall));
    VSwapSprite sprites[58] = {0};
    memset(sprites[2].pixels, 20, sizeof(sprites[2].pixels));
    memset(sprites[2].mask, 1, sizeof(sprites[2].mask));
    sprites[2].right = WALL_SIZE - 1;
    for (int index = 50; index < 58; ++index) {
        memset(sprites[index].pixels, index, sizeof(sprites[index].pixels));
        memset(sprites[index].mask, 1, sizeof(sprites[index].mask));
        sprites[index].right = WALL_SIZE - 1;
    }
    VSwap vswap = {
        .walls = {1, wall},
        .sprite_count = 58,
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
    game.guards[0] = (Guard){4.5, 2.5, 0, 0, 1};
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
    puts("RENDER OK");
    return 0;
}
