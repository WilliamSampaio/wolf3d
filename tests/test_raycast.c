#include "raycast.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
    uint16_t map[MAP_CELLS];
    for (int i = 0; i < MAP_CELLS; ++i)
        map[i] = 107;
    map[2 * MAP_SIDE + 5] = 3;

    RayHit hit;
    assert(raycast_hit(map, NULL, NULL, 2.5, 2.25, 1.0, 0.0, &hit));
    assert(hit.tile == 3 && hit.side == 0);
    assert(fabs(hit.distance - 2.5) < 0.000001);
    assert(fabs(hit.wall_position - 0.25) < 0.000001);

    map[2 * MAP_SIDE + 5] = 107;
    assert(!raycast_hit(map, NULL, NULL, 2.5, 2.25, 1.0, 0.0, &hit));

    int16_t door_at[MAP_CELLS];
    for (int i = 0; i < MAP_CELLS; ++i)
        door_at[i] = -1;
    Door doors[1] = {{4, 2, 1, DOOR_NORMAL, DOOR_CLOSED, 0.0, 0.0}};
    map[2 * MAP_SIDE + 4] = 90;
    map[2 * MAP_SIDE + 5] = 3;
    door_at[2 * MAP_SIDE + 4] = 0;
    assert(raycast_hit(map, door_at, doors, 2.5, 2.25, 1.0, 0.0, &hit));
    assert(hit.kind == RAY_HIT_DOOR && hit.door_index == 0);
    assert(fabs(hit.distance - 2.0) < 0.000001);
    assert(raycast_hit(map, door_at, doors, 4.25, 2.25, 1.0, 0.0, &hit));
    assert(hit.kind == RAY_HIT_DOOR);
    assert(fabs(hit.distance - 0.25) < 0.000001);
    doors[0].position = 0.5;
    assert(raycast_hit(map, door_at, doors, 2.5, 2.25, 1.0, 0.0, &hit));
    assert(hit.kind == RAY_HIT_WALL && hit.tile == 3);
    doors[0].position = 0.1;
    assert(raycast_hit(map, door_at, doors, 2.5, 2.25, 1.0, 0.0, &hit));
    assert(hit.kind == RAY_HIT_DOOR);
    puts("RAYCAST OK");
    return 0;
}
