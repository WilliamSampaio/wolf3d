#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>

#include "map.h"
#include "door.h"

typedef enum {
    RAY_HIT_WALL,
    RAY_HIT_DOOR
} RayHitKind;

typedef struct {
    double distance;
    double wall_position;
    uint16_t tile;
    int side;
    int map_x;
    int map_y;
    int door_index;
    RayHitKind kind;
} RayHit;

int raycast_hit(const uint16_t map[MAP_CELLS],
                const int16_t door_at[MAP_CELLS], const Door *doors,
                double x, double y,
                double direction_x, double direction_y, RayHit *hit);

#endif
