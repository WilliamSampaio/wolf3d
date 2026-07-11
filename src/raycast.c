#include "raycast.h"

#include <math.h>

static int hit_door(const uint16_t map[MAP_CELLS],
                    const int16_t door_at[MAP_CELLS], const Door *doors,
                    int map_x, int map_y, double x, double y,
                    double direction_x, double direction_y, RayHit *hit)
{
    if (!door_at || !doors)
        return 0;
    const int cell = map_y * MAP_SIDE + map_x;
    const int door_index = door_at[cell];
    if (door_index < 0)
        return 0;

    const Door *door = &doors[door_index];
    double distance, position;
    if (door->vertical) {
        if (direction_x == 0.0)
            return 0;
        distance = (map_x + 0.5 - x) / direction_x;
        position = y + distance * direction_y - map_y;
        hit->side = 0;
    } else {
        if (direction_y == 0.0)
            return 0;
        distance = (map_y + 0.5 - y) / direction_y;
        position = x + distance * direction_x - map_x;
        hit->side = 1;
    }
    if (distance <= 0.0 || position < 0.0 || position >= 1.0 ||
        position < door->position)
        return 0;

    hit->distance = distance;
    hit->wall_position = position - door->position;
    hit->tile = map[cell];
    hit->map_x = map_x;
    hit->map_y = map_y;
    hit->door_index = door_index;
    hit->kind = RAY_HIT_DOOR;
    return 1;
}

int raycast_hit(const uint16_t map[MAP_CELLS],
                const int16_t door_at[MAP_CELLS], const Door *doors,
                double x, double y,
                double direction_x, double direction_y, RayHit *hit)
{
    int map_x = (int)floor(x);
    int map_y = (int)floor(y);
    if (map_x < 0 || map_x >= MAP_SIDE || map_y < 0 || map_y >= MAP_SIDE ||
        (direction_x == 0.0 && direction_y == 0.0))
        return 0;
    if (hit_door(map, door_at, doors, map_x, map_y, x, y,
                 direction_x, direction_y, hit))
        return 1;

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

        const int cell = map_y * MAP_SIDE + map_x;
        const int door_index = door_at ? door_at[cell] : -1;
        if (door_index >= 0 && doors) {
            if (hit_door(map, door_at, doors, map_x, map_y, x, y,
                         direction_x, direction_y, hit))
                return 1;
            continue;
        }

        hit->tile = map[cell];
        if (hit->tile && hit->tile < 107) {
            hit->distance = hit->side ? side_y - delta_y : side_x - delta_x;
            const double position = hit->side ? x + hit->distance * direction_x
                                              : y + hit->distance * direction_y;
            hit->wall_position = position - floor(position);
            hit->map_x = map_x;
            hit->map_y = map_y;
            hit->door_index = -1;
            hit->kind = RAY_HIT_WALL;
            return hit->distance > 0.0;
        }
    }
    return 0;
}
