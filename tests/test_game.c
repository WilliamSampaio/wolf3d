#include "game.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static WolfMap open_map(void)
{
    WolfMap map;
    memset(&map, 0, sizeof(map));
    map.player_x = 2;
    map.player_y = 2;
    map.player_direction = 1;
    for (int y = 0; y < MAP_SIDE; ++y)
        for (int x = 0; x < MAP_SIDE; ++x)
            map.planes[0][y * MAP_SIDE + x] = x == 0 || y == 0 ||
                                               x == MAP_SIDE - 1 || y == MAP_SIDE - 1
                                                   ? 1 : 107;
    return map;
}

int main(void)
{
    const WolfMap map = open_map();
    GameState first, second;
    game_init(&first, &map, 1234);
    game_init(&second, &map, 1234);

    for (int i = 0; i < 8; ++i)
        assert(game_random(&first) == game_random(&second));

    const PlayerCommand command = {
        .forward = 1.0,
        .turn = 1.0,
        .look_radians = -0.05,
        .use_pressed = 1,
        .attack_pressed = 1
    };
    game_update(&first, &command, 0.05);
    assert(first.player.x > 2.5);
    assert(fabs(first.player.angle - 0.05) < 0.000001);
    puts("GAME OK");
    return 0;
}
