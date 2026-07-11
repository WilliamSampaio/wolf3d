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
    assert(raycast_hit(map, 2.5, 2.25, 1.0, 0.0, &hit));
    assert(hit.tile == 3 && hit.side == 0);
    assert(fabs(hit.distance - 2.5) < 0.000001);
    assert(fabs(hit.wall_position - 0.25) < 0.000001);

    map[2 * MAP_SIDE + 5] = 107;
    assert(!raycast_hit(map, 2.5, 2.25, 1.0, 0.0, &hit));
    puts("RAYCAST OK");
    return 0;
}
