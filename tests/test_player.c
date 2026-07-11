#include "player.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void clear_map(uint16_t map[MAP_CELLS])
{
    for (int y = 0; y < MAP_SIDE; ++y)
        for (int x = 0; x < MAP_SIDE; ++x)
            map[y * MAP_SIDE + x] = x == 0 || y == 0 ||
                                     x == MAP_SIDE - 1 || y == MAP_SIDE - 1
                                         ? 1 : 107;
}

int main(void)
{
    uint16_t map[MAP_CELLS];
    clear_map(map);
    Player player = {2.5, 2.5, 0.0};
    for (int i = 0; i < 10; ++i)
        player_update(&player, map, 1.0, 0.0, 0.01);
    assert(fabs(player.x - 2.75) < 0.000001 && player.y == 2.5);

    Player same_time = {2.5, 2.5, 0.0};
    player_update(&same_time, map, 1.0, 0.0, 0.05);
    player_update(&same_time, map, 1.0, 0.0, 0.05);
    assert(fabs(same_time.x - player.x) < 0.000001);
    player_update(&player, map, 0.0, 1.0, 0.05);
    assert(fabs(player.angle - 0.1) < 0.000001);
    player_rotate(&player, -0.2);
    assert(fabs(player.angle + 0.1) < 0.000001);
    player_rotate(&player, 20.0);
    assert(player.angle >= -3.14159265358979323846 &&
           player.angle < 3.14159265358979323846);
    player.angle = 0.0;

    map[2 * MAP_SIDE + 3] = 1;
    for (int i = 0; i < 20; ++i)
        player_update(&player, map, 1.0, 0.0, 0.05);
    assert(player.x < 2.81);

    player = (Player){2.5, 2.5, 3.14159265358979323846 / 4.0};
    for (int i = 0; i < 4; ++i)
        player_update(&player, map, 1.0, 0.0, 0.05);
    assert(player.x < 2.81 && player.y > 2.8);

    map[2 * MAP_SIDE + 3] = 90;
    player = (Player){2.5, 2.5, 0.0};
    for (int i = 0; i < 10; ++i)
        player_update(&player, map, 1.0, 0.0, 0.05);
    assert(player.x < 2.81);
    puts("PLAYER OK");
    return 0;
}
