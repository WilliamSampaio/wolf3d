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

    WolfMap guards = open_map();
    for (int type = 0; type < 8; ++type)
        guards.planes[1][4 * MAP_SIDE + type + 1] = 108 + type;
    game_init(&blocked, &guards, 1);
    assert(blocked.guard_count == 8);
    for (int type = 0; type < 8; ++type) {
        assert(blocked.guards[type].direction == type % 4);
        assert(blocked.guards[type].patrol == (type >= 4));
        assert(blocked.guards[type].active);
    }

    WolfMap patrol_map = open_map();
    patrol_map.planes[1][4 * MAP_SIDE + 2] = 112;
    GameState patrol;
    game_init(&patrol, &patrol_map, 1);
    for (int step = 0; step < 20; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(fabs(patrol.guards[0].x - (2.5 + 0.546875)) < 0.000001);
    assert(patrol.guards[0].frame == 3);

    WolfMap turn_map = open_map();
    turn_map.planes[1][4 * MAP_SIDE + 2] = 112;
    turn_map.planes[1][4 * MAP_SIDE + 3] = 92;
    game_init(&patrol, &turn_map, 1);
    for (int step = 0; step < 20; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].direction == 0 && patrol.guards[0].y == 4.5);
    for (int step = 0; step < 17; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].x == 3.5 && patrol.guards[0].y == 4.5);
    game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].direction == 1 && patrol.guards[0].y < 4.5);

    game_init(&patrol, &patrol_map, 1);
    patrol.map.planes[1][4 * MAP_SIDE + 2] = 92;
    game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].direction == 1 && patrol.guards[0].y < 4.5);

    patrol_map.planes[0][4 * MAP_SIDE + 3] = 1;
    game_init(&patrol, &patrol_map, 1);
    for (int step = 0; step < 40; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].x < 3.0);

    WolfMap sight_map = open_map();
    sight_map.planes[1][2 * MAP_SIDE + 7] = 108;
    GameState sight;
    game_init(&sight, &sight_map, 1);
    game_update(&sight, &(PlayerCommand){0}, 0.05);
    assert(!sight.guards[0].alerted);

    sight_map.planes[1][2 * MAP_SIDE + 7] = 110;
    game_init(&sight, &sight_map, 1);
    const double guard_start = sight.guards[0].x;
    game_update(&sight, &(PlayerCommand){0}, 0.05);
    assert(sight.guards[0].alerted && sight.guards[0].x < guard_start);

    sight_map.planes[0][2 * MAP_SIDE + 3] = 1;
    game_init(&sight, &sight_map, 1);
    game_update(&sight, &(PlayerCommand){0}, 0.05);
    assert(!sight.guards[0].alerted);

    sight_map = open_map();
    sight_map.planes[1][2 * MAP_SIDE + 3] = 108;
    game_init(&sight, &sight_map, 1);
    game_update(&sight, &(PlayerCommand){0}, 0.05);
    assert(sight.guards[0].alerted);

    sight_map = open_map();
    sight_map.planes[0][2 * MAP_SIDE + 3] = 90;
    sight_map.planes[1][2 * MAP_SIDE + 4] = 110;
    game_init(&sight, &sight_map, 1);
    game_update(&sight, &(PlayerCommand){0}, 0.05);
    assert(!sight.guards[0].alerted);

    sight_map = open_map();
    sight_map.planes[1][7 * MAP_SIDE + 7] = 110;
    game_init(&sight, &sight_map, 1);
    game_update(&sight, &(PlayerCommand){0}, 0.05);
    assert(sight.guards[0].direction == 2);
    for (int step = 0; step < 5; ++step) {
        game_update(&sight, &(PlayerCommand){0}, 0.05);
        assert(sight.guards[0].direction == 2);
    }

    WolfMap attack_map = open_map();
    attack_map.planes[1][2 * MAP_SIDE + 4] = 110;
    GameState enemy_shot;
    game_init(&enemy_shot, &attack_map, 1);
    const double shooting_x = enemy_shot.guards[0].x;
    game_update(&enemy_shot, &(PlayerCommand){0}, 0.05);
    assert(enemy_shot.guards[0].shooting);
    for (int step = 0; step < 12; ++step)
        game_update(&enemy_shot, &(PlayerCommand){0}, 0.05);
    assert(enemy_shot.health < 100 && enemy_shot.damage_seconds > 0.0);
    assert(enemy_shot.guards[0].x == shooting_x);
    for (int step = 0; step < 6; ++step)
        game_update(&enemy_shot, &(PlayerCommand){0}, 0.05);
    assert(!enemy_shot.guards[0].shooting);
    enemy_shot.player.x = 10.5;
    game_update(&enemy_shot, &(PlayerCommand){0}, 0.05);
    assert(enemy_shot.guards[0].x > shooting_x);

    game_init(&enemy_shot, &attack_map, 1);
    enemy_shot.health = 1;
    game_update(&enemy_shot, &(PlayerCommand){0}, 0.05);
    for (int step = 0; step < 12; ++step)
        game_update(&enemy_shot, &(PlayerCommand){0}, 0.05);
    assert(enemy_shot.health == 0 && enemy_shot.player_dead &&
           enemy_shot.lives == 2);
    const Player dead_player = enemy_shot.player;
    const double dead_guard_x = enemy_shot.guards[0].x;
    game_update(&enemy_shot, &(PlayerCommand){.forward = 1.0}, 0.05);
    assert(enemy_shot.player.x == dead_player.x &&
           enemy_shot.guards[0].x == dead_guard_x &&
           enemy_shot.player_death_seconds > 0.0);
    game_update(&enemy_shot, &(PlayerCommand){.use_pressed = 1}, 0.0);
    assert(!enemy_shot.player_dead && enemy_shot.health == 100 &&
           enemy_shot.lives == 2 && enemy_shot.guards[0].health == 25);

    game_init(&enemy_shot, &attack_map, 1);
    enemy_shot.lives = 1;
    enemy_shot.health = 0;
    game_update(&enemy_shot, &(PlayerCommand){0}, 0.0);
    assert(enemy_shot.player_dead && enemy_shot.lives == 0);
    game_update(&enemy_shot, &(PlayerCommand){.use_pressed = 1}, 0.0);
    assert(enemy_shot.player_dead);

    patrol_map.planes[0][4 * MAP_SIDE + 3] = 107;
    patrol_map.planes[1][4 * MAP_SIDE + 3] = 24;
    game_init(&patrol, &patrol_map, 1);
    for (int step = 0; step < 40; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].x < 3.0);

    patrol_map.planes[1][4 * MAP_SIDE + 3] = 0;
    patrol_map.planes[0][4 * MAP_SIDE + 3] = 90;
    game_init(&patrol, &patrol_map, 1);
    for (int step = 0; step < 20; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].x < 3.0);
    assert(patrol.doors[0].action == DOOR_OPENING);
    for (int step = 0; step < 25; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.guards[0].x > 3.0);

    patrol.doors[0].action = DOOR_OPEN;
    patrol.doors[0].position = 1.0;
    patrol.doors[0].open_seconds = 300.0 / 70.0;
    patrol.guards[0].x = 3.5;
    patrol.guards[0].y = 4.5;
    patrol.guards[0].patrol = 0;
    for (int step = 0; step < 10; ++step)
        game_update(&patrol, &(PlayerCommand){0}, 0.05);
    assert(patrol.doors[0].action == DOOR_OPENING);

    for (uint16_t tile = 92; tile <= 100; tile += 8) {
        patrol_map.planes[0][4 * MAP_SIDE + 3] = tile;
        game_init(&patrol, &patrol_map, 1);
        for (int step = 0; step < 40; ++step)
            game_update(&patrol, &(PlayerCommand){0}, 0.05);
        assert(patrol.guards[0].x < 3.0);
        assert(patrol.doors[0].action == DOOR_CLOSED);
    }

    GameState weapon_test;
    game_init(&weapon_test, &map, 1);
    game_update(&weapon_test, &(PlayerCommand){.requested_weapon = 3}, 0.0);
    assert(weapon_test.current_weapon == WEAPON_PISTOL);
    weapon_test.weapons |= 1u << WEAPON_MACHINE_GUN;
    game_update(&weapon_test, &(PlayerCommand){.requested_weapon = 3}, 0.0);
    assert(weapon_test.current_weapon == WEAPON_MACHINE_GUN);
    game_update(&weapon_test, &(PlayerCommand){.attack_held = 1}, 0.0);
    assert(weapon_test.ammo == 7 && weapon_test.weapon_frame == 1);
    for (int step = 0; step < 7; ++step)
        game_update(&weapon_test, &(PlayerCommand){.attack_held = 1}, 0.05);
    assert(weapon_test.ammo == 7 && weapon_test.weapon_frame == 0);
    game_update(&weapon_test, &(PlayerCommand){.attack_held = 1}, 0.0);
    assert(weapon_test.ammo == 6);
    weapon_test.ammo = 0;
    weapon_test.weapon_frame = 0;
    game_update(&weapon_test, &(PlayerCommand){.attack_held = 1}, 0.0);
    assert(weapon_test.weapon_frame == 0);

    game_init(&weapon_test, &map, 1);
    game_update(&weapon_test,
                &(PlayerCommand){.attack_pressed = 1, .attack_held = 1}, 0.0);
    for (int step = 0; step < 8; ++step)
        game_update(&weapon_test, &(PlayerCommand){.attack_held = 1}, 0.05);
    assert(weapon_test.ammo == 7 && weapon_test.weapon_frame == 0);

    WolfMap combat_map = open_map();
    combat_map.planes[1][2 * MAP_SIDE + 4] = 110;
    GameState shot_a, shot_b;
    game_init(&shot_a, &combat_map, 1);
    game_init(&shot_b, &combat_map, 1);
    shot_a.guards[0].health = 100;
    shot_b.guards[0].health = 100;
    const PlayerCommand attack = {.attack_pressed = 1};
    game_update(&shot_a, &attack, 0.0);
    game_update(&shot_b, &attack, 0.0);
    assert(shot_a.ammo == 7 && shot_a.guards[0].health < 100);
    assert(shot_a.guards[0].health == shot_b.guards[0].health);
    assert(shot_a.weapon_frame == 1);
    game_update(&shot_a, &attack, 0.0);
    assert(shot_a.ammo == 7);
    for (int step = 0; step < 7; ++step)
        game_update(&shot_a, &(PlayerCommand){0}, 0.05);
    assert(shot_a.weapon_frame == 0);
    shot_a.ammo = 0;
    const int health_before_empty = shot_a.guards[0].health;
    game_update(&shot_a, &attack, 0.0);
    assert(shot_a.guards[0].health == health_before_empty);

    combat_map.planes[0][2 * MAP_SIDE + 3] = 1;
    game_init(&shot_a, &combat_map, 1);
    game_update(&shot_a, &attack, 0.0);
    assert(shot_a.ammo == 7 && shot_a.guards[0].health == 25);

    combat_map = open_map();
    combat_map.planes[1][2 * MAP_SIDE + 4] = 110;
    combat_map.planes[1][2 * MAP_SIDE + 5] = 110;
    game_init(&shot_a, &combat_map, 1);
    game_update(&shot_a, &attack, 0.0);
    assert(shot_a.guards[0].health < 25 && shot_a.guards[1].health == 25);

    game_init(&shot_a, &combat_map, 1);
    shot_a.guards[0].health = 1;
    const double death_x = shot_a.guards[0].x;
    game_update(&shot_a, &attack, 0.05);
    assert(shot_a.guards[0].dead && shot_a.score == 100);
    assert(shot_a.static_count == 1 && shot_a.statics[0].active &&
           shot_a.statics[0].kind == STATIC_CLIP &&
           shot_a.statics[0].x == 4 && shot_a.statics[0].y == 2);
    shot_a.guards[1].active = 0;
    shot_a.player.x = 4.5;
    shot_a.player.y = 2.5;
    shot_a.ammo = 0;
    game_update(&shot_a, &(PlayerCommand){0}, 0.0);
    assert(shot_a.ammo == 8 && !shot_a.statics[0].active);
    for (int step = 0; step < 20; ++step)
        game_update(&shot_a, &(PlayerCommand){0}, 0.05);
    assert(shot_a.guards[0].x == death_x &&
           shot_a.guards[0].death_seconds > 0.5);

    game_init(&shot_a, &combat_map, 1);
    shot_a.static_count = 1;
    shot_a.statics[0] = (StaticObject){4, 2, 2, 1, STATIC_DECORATION};
    shot_a.guards[0].health = 1;
    game_update(&shot_a, &attack, 0.0);
    assert(shot_a.static_count == 2 &&
           (shot_a.statics[1].x != 4 || shot_a.statics[1].y != 2));

    game_init(&shot_a, &combat_map, 1);
    shot_a.static_count = MAP_CELLS;
    for (size_t index = 0; index < MAP_CELLS; ++index)
        shot_a.statics[index] = (StaticObject){0, 0, 2, 1, STATIC_DECORATION};
    shot_a.guards[0].health = 1;
    game_update(&shot_a, &attack, 0.0);
    assert(shot_a.static_count == MAP_CELLS);

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
