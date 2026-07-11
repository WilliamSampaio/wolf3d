#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#include "map.h"
#include "player.h"

typedef struct {
    double forward;
    double turn;
    double look_radians;
    int use_pressed;
    int attack_pressed;
    int attack_held;
    int requested_weapon;
} PlayerCommand;

typedef struct {
    WolfMap map;
    Player player;
    uint32_t random_state;
} GameState;

void game_init(GameState *game, const WolfMap *map, uint32_t random_seed);
void game_update(GameState *game, const PlayerCommand *command, double seconds);
uint32_t game_random(GameState *game);

#endif
