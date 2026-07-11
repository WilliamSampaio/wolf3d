#include "game.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static WolfMap open_map(void)
{
    WolfMap map;
    memset(&map, 0, sizeof(map));
    map.player_x = 2;
    map.player_y = 2;
    map.player_direction = 1;
    for (int y = 0; y < MAP_SIDE; ++y)
        for (int x = 0; x < MAP_SIDE; ++x)
            map.planes[0][y * MAP_SIDE + x] = x == 0 || y == 0 ||
                                               x == MAP_SIDE - 1 || y == MAP_SIDE - 1
                                                   ? 1 : 107;
    return map;
}

static GameState game_with_object(uint16_t code)
{
    WolfMap map = open_map();
    map.planes[1][2 * MAP_SIDE + 2] = code;
    GameState game;
    game_init(&game, &map, 1);
    return game;
}

static void update_once(GameState *game)
{
    const PlayerCommand idle = {0};
    game_update(game, &idle, 0.0);
}

int main(void)
{
    const WolfMap map = open_map();
    GameState first, second;
    game_init(&first, &map, 1234);
    game_init(&second, &map, 1234);

    for (int i = 0; i < 8; ++i)
        assert(game_random(&first) == game_random(&second));

    const PlayerCommand command = {
        .forward = 1.0,
        .turn = 1.0,
        .look_radians = -0.05,
        .use_pressed = 1,
        .attack_pressed = 1
    };
    game_update(&first, &command, 0.05);
    assert(first.player.x > 2.5);
    assert(fabs(first.player.angle - 0.05) < 0.000001);

    WolfMap collision_map = open_map();
    collision_map.planes[1][2 * MAP_SIDE + 3] = 24;
    GameState blocked;
    game_init(&blocked, &collision_map, 1);
    const PlayerCommand forward = {.forward = 1.0};
    for (int i = 0; i < 10; ++i)
        game_update(&blocked, &forward, 0.05);
    assert(blocked.player.x < 2.81);
    collision_map.planes[1][2 * MAP_SIDE + 3] = 23;
    game_init(&blocked, &collision_map, 1);
    for (int i = 0; i < 10; ++i)
        game_update(&blocked, &forward, 0.05);
    assert(blocked.player.x > 3.0);

    WolfMap all_statics = open_map();
    for (int type = 0; type < 48; ++type)
        all_statics.planes[1][3 * MAP_SIDE + type + 1] = 23 + type;
    game_init(&blocked, &all_statics, 1);
    assert(blocked.static_count == 48);
    int blocking_count = 0;
    for (int x = 1; x <= 48; ++x)
        blocking_count += blocked.blocked[3 * MAP_SIDE + x] != 0;
    assert(blocking_count == 21);

    GameState pickup = game_with_object(47);
    pickup.health = 90;
    update_once(&pickup);
    assert(pickup.health == 100 && !pickup.statics[0].active);
    pickup = game_with_object(48);
    pickup.health = 50;
    update_once(&pickup);
    assert(pickup.health == 75);
    pickup = game_with_object(49);
    pickup.ammo = 95;
    update_once(&pickup);
    assert(pickup.ammo == 99);
    pickup = game_with_object(49);
    pickup.ammo = 99;
    update_once(&pickup);
    assert(pickup.statics[0].active);
    pickup = game_with_object(43);
    update_once(&pickup);
    assert(pickup.keys == 1u);
    pickup = game_with_object(50);
    update_once(&pickup);
    assert(pickup.weapons & (1u << WEAPON_MACHINE_GUN));
    pickup = game_with_object(52);
    update_once(&pickup);
    assert(pickup.score == 100 && pickup.treasure_count == 1 &&
           pickup.treasure_total == 1);

    for (uint16_t tile = 90; tile <= 101; ++tile) {
        int vertical;
        DoorKind kind;
        assert(door_from_tile(tile, &vertical, &kind));
        assert(vertical == !(tile & 1u));
        assert(kind == (DoorKind)((tile - 90) / 2));
    }
    Door geometry = {3, 2, 1, DOOR_NORMAL, DOOR_CLOSED, 0.0, 0.0};
    assert(door_blocks_circle(&geometry, 0.0, 3.5, 2.5, 0.2));
    assert(!door_blocks_circle(&geometry, 0.8, 3.5, 2.5, 0.2));
    geometry.vertical = 0;
    assert(door_blocks_circle(&geometry, 0.0, 3.5, 2.5, 0.2));
    assert(!door_blocks_circle(&geometry, 0.8, 3.5, 2.5, 0.2));

    WolfMap door_map = open_map();
    door_map.planes[0][2 * MAP_SIDE + 3] = 90;
    GameState door_game;
    game_init(&door_game, &door_map, 1);
    assert(door_game.door_count == 1);
    assert(door_game.doors[0].action == DOOR_CLOSED);
    const PlayerCommand use = {.use_pressed = 1};
    game_update(&door_game, &use, 0.0);
    assert(door_game.doors[0].action == DOOR_OPENING);
    game_update(&door_game, &(PlayerCommand){0}, 0.05);
    game_update(&door_game, &use, 0.0);
    assert(door_game.doors[0].action == DOOR_CLOSING);
    game_update(&door_game, &use, 0.0);
    assert(door_game.doors[0].action == DOOR_OPENING);
    for (int i = 0; i < 19; ++i)
        game_update(&door_game, &(PlayerCommand){0}, 0.05);
    assert(door_game.doors[0].action == DOOR_OPEN);
    for (int i = 0; i < 10; ++i)
        game_update(&door_game, &forward, 0.05);
    assert(door_game.player.x > 3.5);

    door_map.planes[0][2 * MAP_SIDE + 3] = 92;
    game_init(&door_game, &door_map, 1);
    game_update(&door_game, &use, 0.0);
    assert(door_game.doors[0].action == DOOR_CLOSED);
    door_game.keys = 1u;
    game_update(&door_game, &use, 0.0);
    assert(door_game.doors[0].action == DOOR_OPENING);

    door_game.doors[0].position = 1.0;
    door_game.doors[0].action = DOOR_OPEN;
    door_game.doors[0].open_seconds = 300.0 / 70.0;
    door_game.player = (Player){3.5, 2.8, 0.0};
    game_update(&door_game, &(PlayerCommand){0}, 0.05);
    assert(door_game.doors[0].action == DOOR_CLOSING);
    game_update(&door_game, &(PlayerCommand){0}, 0.05);
    assert(door_game.doors[0].action == DOOR_OPENING);

    game_init(&door_game, &door_map, 1);
    door_game.keys = 1u;
    door_game.player = (Player){3.25, 2.5, 0.0};
    game_update(&door_game, &use, 0.0);
    assert(door_game.doors[0].action == DOOR_OPENING);
    puts("GAME OK");
    return 0;
}
