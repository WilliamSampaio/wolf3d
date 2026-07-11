#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"

typedef struct {
    double x;
    double y;
    double angle;
} Player;

void player_init(Player *player, const WolfMap *map);
void player_rotate(Player *player, double radians);
void player_update(Player *player, const uint16_t map[MAP_CELLS],
                   double forward, double turn, double seconds);

#endif
