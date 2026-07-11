#ifndef RAYCAST_H
#define RAYCAST_H

#include <stdint.h>

#include "map.h"

typedef struct {
    double distance;
    double wall_position;
    uint16_t tile;
    int side;
} RayHit;

int raycast_hit(const uint16_t map[MAP_CELLS], double x, double y,
                double direction_x, double direction_y, RayHit *hit);

#endif
