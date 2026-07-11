#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#include "door.h"
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

typedef enum {
    STATIC_DECORATION,
    STATIC_BLOCKING,
    STATIC_GIBS,
    STATIC_DOG_FOOD,
    STATIC_FIRST_AID,
    STATIC_GOLD_KEY,
    STATIC_SILVER_KEY,
    STATIC_CROSS,
    STATIC_CHALICE,
    STATIC_BIBLE,
    STATIC_CROWN,
    STATIC_CLIP,
    STATIC_MACHINE_GUN,
    STATIC_CHAIN_GUN,
    STATIC_FOOD,
    STATIC_FULL_HEAL
} StaticKind;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t sprite;
    uint8_t active;
    StaticKind kind;
} StaticObject;

typedef struct {
    double x;
    double y;
    uint8_t direction;
    uint8_t patrol;
    uint8_t active;
} Guard;

enum {
    WEAPON_KNIFE,
    WEAPON_PISTOL,
    WEAPON_MACHINE_GUN,
    WEAPON_CHAIN_GUN
};

typedef struct {
    WolfMap map;
    Player player;
    uint32_t random_state;
    StaticObject statics[MAP_CELLS];
    size_t static_count;
    Guard guards[MAP_CELLS];
    size_t guard_count;
    uint8_t blocked[MAP_CELLS];
    int health;
    int ammo;
    int lives;
    int score;
    unsigned int keys;
    unsigned int weapons;
    int current_weapon;
    int treasure_count;
    int treasure_total;
    Door doors[MAP_CELLS];
    size_t door_count;
    int16_t door_at[MAP_CELLS];
} GameState;

void game_init(GameState *game, const WolfMap *map, uint32_t random_seed);
void game_update(GameState *game, const PlayerCommand *command, double seconds);
uint32_t game_random(GameState *game);

#endif
