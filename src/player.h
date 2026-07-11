#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"
#include "door.h"

typedef struct {
    double x;
    double y;
    double angle;
} Player;

void player_init(Player *player, const WolfMap *map);
void player_rotate(Player *player, double radians);
void player_update(Player *player, const uint16_t map[MAP_CELLS],
                   const uint8_t blocked[MAP_CELLS],
                   const int16_t door_at[MAP_CELLS], const Door *doors,
                   double forward, double turn, double seconds);

#endif
