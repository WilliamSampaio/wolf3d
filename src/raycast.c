#include "raycast.h"

#include <math.h>

int raycast_hit(const uint16_t map[MAP_CELLS], double x, double y,
                double direction_x, double direction_y, RayHit *hit)
{
    int map_x = (int)floor(x);
    int map_y = (int)floor(y);
    if (map_x < 0 || map_x >= MAP_SIDE || map_y < 0 || map_y >= MAP_SIDE ||
        (direction_x == 0.0 && direction_y == 0.0))
        return 0;

    const double delta_x = direction_x == 0.0 ? 1e30 : fabs(1.0 / direction_x);
    const double delta_y = direction_y == 0.0 ? 1e30 : fabs(1.0 / direction_y);
    const int step_x = direction_x < 0.0 ? -1 : 1;
    const int step_y = direction_y < 0.0 ? -1 : 1;
    double side_x = (direction_x < 0.0 ? x - map_x : map_x + 1.0 - x) * delta_x;
    double side_y = (direction_y < 0.0 ? y - map_y : map_y + 1.0 - y) * delta_y;

    for (int steps = 0; steps < MAP_CELLS; ++steps) {
        if (side_x < side_y) {
            side_x += delta_x;
            map_x += step_x;
            hit->side = 0;
        } else {
            side_y += delta_y;
            map_y += step_y;
            hit->side = 1;
        }
        if (map_x < 0 || map_x >= MAP_SIDE || map_y < 0 || map_y >= MAP_SIDE)
            return 0;

        hit->tile = map[map_y * MAP_SIDE + map_x];
        if (hit->tile && hit->tile < 107) {
            hit->distance = hit->side ? side_y - delta_y : side_x - delta_x;
            const double position = hit->side ? x + hit->distance * direction_x
                                              : y + hit->distance * direction_y;
            hit->wall_position = position - floor(position);
            return hit->distance > 0.0;
        }
    }
    return 0;
}
