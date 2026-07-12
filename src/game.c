#include "game.h"

#include <math.h>
#include <string.h>

#include "raycast.h"

static const double door_travel_seconds = 65535.0 / (1024.0 * 70.0);
static const double door_hold_seconds = 300.0 / 70.0;
static const double player_radius = 0.2;
static const double guard_patrol_speed = 512.0 * 70.0 / 65536.0;
static const double dog_patrol_speed = 1500.0 * 70.0 / 65536.0;
static const int guard_direction_x[4] = {1, 0, -1, 0};
static const int guard_direction_y[4] = {0, -1, 0, 1};

static const StaticKind static_kinds[48] = {
    STATIC_DECORATION, STATIC_BLOCKING, STATIC_BLOCKING, STATIC_BLOCKING,
    STATIC_DECORATION, STATIC_BLOCKING, STATIC_DOG_FOOD, STATIC_BLOCKING,
    STATIC_BLOCKING, STATIC_DECORATION, STATIC_BLOCKING, STATIC_BLOCKING,
    STATIC_BLOCKING, STATIC_BLOCKING, STATIC_DECORATION, STATIC_DECORATION,
    STATIC_BLOCKING, STATIC_BLOCKING, STATIC_BLOCKING, STATIC_DECORATION,
    STATIC_GOLD_KEY, STATIC_SILVER_KEY, STATIC_BLOCKING, STATIC_DECORATION,
    STATIC_FOOD, STATIC_FIRST_AID, STATIC_CLIP, STATIC_MACHINE_GUN,
    STATIC_CHAIN_GUN, STATIC_CROSS, STATIC_CHALICE, STATIC_BIBLE,
    STATIC_CROWN, STATIC_FULL_HEAL, STATIC_GIBS, STATIC_BLOCKING,
    STATIC_BLOCKING, STATIC_BLOCKING, STATIC_GIBS, STATIC_BLOCKING,
    STATIC_BLOCKING, STATIC_DECORATION, STATIC_DECORATION, STATIC_DECORATION,
    STATIC_DECORATION, STATIC_BLOCKING, STATIC_BLOCKING, STATIC_DECORATION
};

static int is_treasure(StaticKind kind)
{
    return kind == STATIC_CROSS || kind == STATIC_CHALICE ||
           kind == STATIC_BIBLE || kind == STATIC_CROWN ||
           kind == STATIC_FULL_HEAL;
}

static int add_limited(int value, int amount, int maximum)
{
    value += amount;
    return value > maximum ? maximum : value;
}

static int collect(GameState *game, StaticObject *object)
{
    switch (object->kind) {
    case STATIC_GIBS:
        if (game->health > 10)
            return 0;
        game->health = add_limited(game->health, 1, 100);
        break;
    case STATIC_DOG_FOOD:
        if (game->health == 100)
            return 0;
        game->health = add_limited(game->health, 4, 100);
        break;
    case STATIC_FIRST_AID:
        if (game->health == 100)
            return 0;
        game->health = add_limited(game->health, 25, 100);
        break;
    case STATIC_GOLD_KEY:
        game->keys |= 1u;
        break;
    case STATIC_SILVER_KEY:
        game->keys |= 2u;
        break;
    case STATIC_CROSS:
        game->score += 100;
        ++game->treasure_count;
        break;
    case STATIC_CHALICE:
        game->score += 500;
        ++game->treasure_count;
        break;
    case STATIC_BIBLE:
        game->score += 1000;
        ++game->treasure_count;
        break;
    case STATIC_CROWN:
        game->score += 5000;
        ++game->treasure_count;
        break;
    case STATIC_CLIP:
        if (game->ammo == 99)
            return 0;
        game->ammo = add_limited(game->ammo, 8, 99);
        break;
    case STATIC_MACHINE_GUN:
        game->ammo = add_limited(game->ammo, 6, 99);
        game->weapons |= 1u << WEAPON_MACHINE_GUN;
        game->current_weapon = WEAPON_MACHINE_GUN;
        break;
    case STATIC_CHAIN_GUN:
        game->ammo = add_limited(game->ammo, 6, 99);
        game->weapons |= 1u << WEAPON_CHAIN_GUN;
        game->current_weapon = WEAPON_CHAIN_GUN;
        break;
    case STATIC_FOOD:
        if (game->health == 100)
            return 0;
        game->health = add_limited(game->health, 10, 100);
        break;
    case STATIC_FULL_HEAL:
        game->health = add_limited(game->health, 99, 100);
        game->ammo = add_limited(game->ammo, 25, 99);
        ++game->lives;
        ++game->treasure_count;
        break;
    default:
        return 0;
    }
    object->active = 0;
    return 1;
}

static void collect_at_player(GameState *game)
{
    const int x = (int)game->player.x;
    const int y = (int)game->player.y;
    for (size_t index = 0; index < game->static_count; ++index) {
        StaticObject *object = &game->statics[index];
        if (object->active && object->x == x && object->y == y)
            collect(game, object);
    }
}

static int door_obstructed(const GameState *game, const Door *door,
                           double position)
{
    if (door_blocks_circle(door, position, game->player.x, game->player.y,
                           player_radius))
        return 1;
    for (size_t index = 0; index < game->guard_count; ++index) {
        const Guard *guard = &game->guards[index];
        if (guard->active &&
            door_blocks_circle(door, position, guard->x, guard->y,
                               player_radius))
            return 1;
    }
    return 0;
}

static void operate_door(GameState *game, Door *door)
{
    const unsigned int key = door_required_key(door->kind);
    if (key && !(game->keys & key))
        return;
    if (door->action == DOOR_CLOSED || door->action == DOOR_CLOSING) {
        door->action = DOOR_OPENING;
    } else if (!door_obstructed(game, door, door->position)) {
        door->action = DOOR_CLOSING;
    }
}

static int push_destination_open(const GameState *game, int x, int y)
{
    if (x < 0 || x >= MAP_SIDE || y < 0 || y >= MAP_SIDE)
        return 0;
    const size_t cell = (size_t)y * MAP_SIDE + x;
    if (game->blocked[cell] || game->door_at[cell] >= 0)
        return 0;
    const uint16_t tile = game->map.planes[0][cell];
    if (tile && tile < 107)
        return 0;
    if ((int)game->player.x == x && (int)game->player.y == y)
        return 0;
    for (size_t index = 0; index < game->guard_count; ++index)
        if (game->guards[index].active && !game->guards[index].dead &&
            (int)game->guards[index].x == x &&
            (int)game->guards[index].y == y)
            return 0;
    return 1;
}

static void push_wall(GameState *game, int x, int y, int direction_x,
                      int direction_y)
{
    const int first_x = x + direction_x;
    const int first_y = y + direction_y;
    if (!push_destination_open(game, first_x, first_y))
        return;
    int final_x = first_x;
    int final_y = first_y;
    const int second_x = first_x + direction_x;
    const int second_y = first_y + direction_y;
    if (push_destination_open(game, second_x, second_y)) {
        final_x = second_x;
        final_y = second_y;
    }

    const size_t source = (size_t)y * MAP_SIDE + x;
    const uint16_t wall = game->map.planes[0][source];
    const uint16_t floor = game->map.planes[0][(int)game->player.y * MAP_SIDE +
                                               (int)game->player.x];
    /* ponytail: instant tile movement; add partial geometry with animation. */
    game->map.planes[0][source] = floor;
    game->map.planes[0][(size_t)first_y * MAP_SIDE + first_x] = floor;
    game->map.planes[0][(size_t)final_y * MAP_SIDE + final_x] = wall;
    game->map.planes[1][source] = 0;
    ++game->secret_count;
}

static void use_adjacent_door(GameState *game)
{
    const double direction_x = cos(game->player.angle);
    const double direction_y = sin(game->player.angle);
    int x = (int)game->player.x;
    int y = (int)game->player.y;
    const int current_door = game->door_at[y * MAP_SIDE + x];
    if (current_door >= 0) {
        operate_door(game, &game->doors[current_door]);
        return;
    }
    const int horizontal = fabs(direction_x) >= fabs(direction_y);
    if (horizontal)
        x += direction_x < 0.0 ? -1 : 1;
    else
        y += direction_y < 0.0 ? -1 : 1;
    if (x < 0 || x >= MAP_SIDE || y < 0 || y >= MAP_SIDE)
        return;
    const size_t cell = (size_t)y * MAP_SIDE + x;
    if (game->map.planes[1][cell] == 98) {
        push_wall(game, x, y, x - (int)game->player.x,
                  y - (int)game->player.y);
        return;
    }
    if (horizontal && game->map.planes[0][cell] == 21) {
        game->map.planes[0][cell] = 22;
        game->level_complete = 1;
        return;
    }
    const int door_index = game->door_at[cell];
    if (door_index >= 0)
        operate_door(game, &game->doors[door_index]);
}

static void update_doors(GameState *game, double seconds)
{
    const double movement = seconds / door_travel_seconds;
    for (size_t index = 0; index < game->door_count; ++index) {
        Door *door = &game->doors[index];
        switch (door->action) {
        case DOOR_OPENING:
            door->position += movement;
            if (door->position >= 1.0) {
                door->position = 1.0;
                door->open_seconds = 0.0;
                door->action = DOOR_OPEN;
            }
            break;
        case DOOR_OPEN:
            door->open_seconds += seconds;
            if (door->open_seconds >= door_hold_seconds &&
                !door_obstructed(game, door, door->position))
                door->action = DOOR_CLOSING;
            break;
        case DOOR_CLOSING: {
            double next = door->position - movement;
            if (next < 0.0)
                next = 0.0;
            if (door_obstructed(game, door, next)) {
                door->action = DOOR_OPENING;
            } else {
                door->position = next;
                if (door->position <= 0.0)
                    door->action = DOOR_CLOSED;
            }
            break;
        }
        case DOOR_CLOSED:
            break;
        }
    }
}

static int guard_cell_open(const GameState *game, int x, int y)
{
    if (x < 0 || x >= MAP_SIDE || y < 0 || y >= MAP_SIDE)
        return 0;
    const size_t cell = (size_t)y * MAP_SIDE + x;
    if (game->blocked[cell])
        return 0;
    const int door = game->door_at[cell];
    if (door >= 0)
        return game->doors[door].action == DOOR_OPEN;
    const uint16_t tile = game->map.planes[0][cell];
    return !tile || tile >= 107;
}

static int guard_sees_player(const GameState *game, const Guard *guard)
{
    const double dx = game->player.x - guard->x;
    const double dy = game->player.y - guard->y;
    const double distance = sqrt(dx * dx + dy * dy);
    if (distance > 1.5 &&
        dx * guard_direction_x[guard->direction] +
        dy * guard_direction_y[guard->direction] <= 0.0)
        return 0;
    if (distance == 0.0)
        return 1;
    RayHit hit;
    return !raycast_hit(game->map.planes[0], game->door_at, game->doors,
                        guard->x, guard->y, dx / distance, dy / distance,
                        &hit) || hit.distance >= distance;
}

static int move_guard(GameState *game, Guard *guard, uint8_t direction,
                      double distance)
{
    const double next_x = guard->x + guard_direction_x[direction] * distance;
    const double next_y = guard->y + guard_direction_y[direction] * distance;
    const int tile_x = (int)next_x;
    const int tile_y = (int)next_y;
    const double player_x = next_x - game->player.x;
    const double player_y = next_y - game->player.y;
    if (tile_x >= 0 && tile_x < MAP_SIDE && tile_y >= 0 && tile_y < MAP_SIDE) {
        const int door_index = game->door_at[tile_y * MAP_SIDE + tile_x];
        if (door_index >= 0) {
            Door *door = &game->doors[door_index];
            if (door->kind == DOOR_NORMAL &&
                (door->action == DOOR_CLOSED || door->action == DOOR_CLOSING))
                door->action = DOOR_OPENING;
        }
    }
    if (player_x * player_x + player_y * player_y < 0.25 ||
        !guard_cell_open(game, tile_x, tile_y))
        return 0;
    guard->direction = direction;
    guard->x = next_x;
    guard->y = next_y;
    return 1;
}

static double distance_to_next_center(const Guard *guard, uint8_t direction)
{
    switch (direction) {
    case 0:
        return floor(guard->x) + 1.5 - guard->x;
    case 1:
        return guard->y - (floor(guard->y) - 0.5);
    case 2:
        return guard->x - (floor(guard->x) - 0.5);
    default:
        return floor(guard->y) + 1.5 - guard->y;
    }
}

static int start_guard_move(GameState *game, Guard *guard, uint8_t direction,
                            double distance)
{
    guard->move_remaining = distance_to_next_center(guard, direction);
    const double movement = distance < guard->move_remaining
                                ? distance : guard->move_remaining;
    if (!move_guard(game, guard, direction, movement)) {
        guard->move_remaining = 0.0;
        return 0;
    }
    guard->move_remaining -= movement;
    return 1;
}

static void chase_player(GameState *game, Guard *guard, double distance)
{
    if (guard->move_remaining > 0.0) {
        const double movement = distance < guard->move_remaining
                                    ? distance : guard->move_remaining;
        if (move_guard(game, guard, guard->direction, movement)) {
            guard->move_remaining -= movement;
            return;
        }
        guard->move_remaining = 0.0;
    }

    const double dx = game->player.x - guard->x;
    const double dy = game->player.y - guard->y;
    const uint8_t horizontal = dx < 0.0 ? 2 : 0;
    const uint8_t vertical = dy < 0.0 ? 1 : 3;
    uint8_t first = vertical;
    uint8_t second = horizontal;
    if (fabs(dx) >= fabs(dy)) {
        first = horizontal;
        second = vertical;
    }
    if (!start_guard_move(game, guard, first, distance))
        start_guard_move(game, guard, second, distance);
}

static int drop_cell_free(const GameState *game, int x, int y)
{
    if (x < 0 || x >= MAP_SIDE || y < 0 || y >= MAP_SIDE)
        return 0;
    const size_t cell = (size_t)y * MAP_SIDE + x;
    if (game->blocked[cell] || game->door_at[cell] >= 0)
        return 0;
    const uint16_t tile = game->map.planes[0][cell];
    if (tile && tile < 107)
        return 0;
    for (size_t index = 0; index < game->static_count; ++index)
        if (game->statics[index].active && game->statics[index].x == x &&
            game->statics[index].y == y)
            return 0;
    return 1;
}

static void place_drop(GameState *game, int x, int y, StaticKind kind)
{
    StaticObject *drop = NULL;
    for (size_t index = 0; index < game->static_count; ++index) {
        if (!game->statics[index].active) {
            drop = &game->statics[index];
            break;
        }
    }
    if (!drop) {
        if (game->static_count == MAP_CELLS)
            return;
        drop = &game->statics[game->static_count++];
    }
    const uint8_t sprite = kind == STATIC_GOLD_KEY ? 22 :
                           kind == STATIC_MACHINE_GUN ? 29 : 28;
    *drop = (StaticObject){(uint8_t)x, (uint8_t)y, sprite, 1, kind};
}

static void drop_enemy_item(GameState *game, const Guard *guard)
{
    const StaticKind kind = guard->kind == ENEMY_HANS ? STATIC_GOLD_KEY :
                            guard->kind == ENEMY_SS &&
                            !(game->weapons & (1u << WEAPON_MACHINE_GUN))
                                ? STATIC_MACHINE_GUN : STATIC_CLIP;
    const int center_x = (int)guard->x;
    const int center_y = (int)guard->y;
    if (drop_cell_free(game, center_x, center_y)) {
        place_drop(game, center_x, center_y, kind);
        return;
    }
    for (int y = center_y - 1; y <= center_y + 1; ++y) {
        for (int x = center_x - 1; x <= center_x + 1; ++x) {
            if (drop_cell_free(game, x, y)) {
                place_drop(game, x, y, kind);
                return;
            }
        }
    }
}

static void player_attack(GameState *game)
{
    const int knife = game->current_weapon == WEAPON_KNIFE;
    if (!knife && game->ammo <= 0)
        return;
    if (!knife)
        --game->ammo;
    game->weapon_frame = 1;
    game->weapon_seconds = 0.0;

    const double direction_x = cos(game->player.angle);
    const double direction_y = sin(game->player.angle);
    Guard *target = NULL;
    double target_distance = 1e30;
    for (size_t index = 0; index < game->guard_count; ++index) {
        Guard *guard = &game->guards[index];
        if (!guard->active || guard->dead || guard->kind == ENEMY_GHOST)
            continue;
        const double dx = guard->x - game->player.x;
        const double dy = guard->y - game->player.y;
        const double forward = dx * direction_x + dy * direction_y;
        const double side = fabs(dx * direction_y - dy * direction_x);
        if (forward <= 0.0 || side > forward * 0.1)
            continue;
        const double distance = sqrt(dx * dx + dy * dy);
        if (knife && distance > 1.5)
            continue;
        RayHit hit;
        if (raycast_hit(game->map.planes[0], game->door_at, game->doors,
                        game->player.x, game->player.y,
                        dx / distance, dy / distance, &hit) &&
            hit.distance < distance)
            continue;
        if (distance < target_distance) {
            target = guard;
            target_distance = distance;
        }
    }
    if (!target)
        return;

    int damage = (int)(game_random(game) & 0xffu) / (knife ? 16 : 4);
    if (!target->alerted)
        damage *= 2;
    target->alerted = 1;
    target->health -= damage;
    if (target->health <= 0) {
        target->dead = 1;
        target->death_seconds = 0.0;
        if (target->kind != ENEMY_DOG)
            drop_enemy_item(game, target);
        game->score += target->kind == ENEMY_OFFICER ? 400 :
                       target->kind == ENEMY_DOG ? 200 :
                       target->kind == ENEMY_SS ? 500 :
                       target->kind == ENEMY_HANS ? 5000 : 100;
    }
}

static void update_weapon(GameState *game, double seconds)
{
    if (!game->weapon_frame)
        return;
    game->weapon_seconds += seconds;
    const int frame = 1 + (int)(game->weapon_seconds * 70.0 / 6.0);
    if (frame > 4) {
        game->weapon_frame = 0;
        game->weapon_seconds = 0.0;
        if (game->ammo <= 0 && game->current_weapon != WEAPON_KNIFE)
            game->current_weapon = WEAPON_KNIFE;
    } else {
        game->weapon_frame = (uint8_t)frame;
    }
}

static void kill_player(GameState *game)
{
    if (game->player_dead)
        return;
    game->health = 0;
    game->player_dead = 1;
    game->player_death_seconds = 0.0;
    if (game->lives > 0)
        --game->lives;
}

static void guard_shoot(GameState *game, Guard *guard, double seconds)
{
    if (guard->kind == ENEMY_DOG) {
        const double previous = guard->shoot_seconds;
        guard->shoot_seconds += seconds;
        const double ticks = guard->shoot_seconds * 70.0;
        guard->shoot_frame = ticks < 10.0 ? 0 : ticks < 20.0 ? 1 :
                             ticks < 30.0 ? 2 : ticks < 40.0 ? 0 : 3;
        if (previous < 20.0 / 70.0 &&
            guard->shoot_seconds >= 20.0 / 70.0 &&
            fabs(game->player.x - guard->x) <= 2.0 &&
            fabs(game->player.y - guard->y) <= 2.0 &&
            (game_random(game) & 0xffu) < 180u) {
            const int damage = (int)(game_random(game) & 0xffu) / 16;
            game->health -= damage;
            if (game->health <= 0)
                kill_player(game);
            if (damage)
                game->damage_seconds = 0.15;
        }
        if (ticks >= 50.0) {
            guard->shooting = 0;
            guard->shoot_frame = 0;
            guard->shoot_seconds = 0.0;
        }
        return;
    }
    if (guard->kind == ENEMY_SS) {
        static const double shot_ticks[4] = {40.0, 60.0, 80.0, 100.0};
        const double previous = guard->shoot_seconds * 70.0;
        guard->shoot_seconds += seconds;
        const double ticks = guard->shoot_seconds * 70.0;
        guard->shoot_frame = ticks < 20.0 ? 0 :
                             ((int)((ticks - 20.0) / 10.0) & 1) ? 2 : 1;
        for (size_t shot = 0; shot < 4; ++shot) {
            if (previous < shot_ticks[shot] && ticks >= shot_ticks[shot] &&
                guard_sees_player(game, guard)) {
                const double dx = fabs(game->player.x - guard->x);
                const double dy = fabs(game->player.y - guard->y);
                const int distance = (int)ceil(dx > dy ? dx : dy) * 2 / 3;
                int chance = 256 - distance * 16;
                if ((int)(game_random(game) & 0xffu) < chance) {
                    const int divisor = distance < 2 ? 4 : distance < 4 ? 8 : 16;
                    const int damage = 1 +
                                       (int)(game_random(game) & 0xffu) / divisor;
                    game->health -= damage;
                    if (game->health <= 0)
                        kill_player(game);
                    game->damage_seconds = 0.15;
                }
            }
        }
        if (ticks >= 110.0) {
            guard->shooting = 0;
            guard->shoot_frame = 0;
            guard->shoot_seconds = 0.0;
        }
        return;
    }
    if (guard->kind == ENEMY_HANS) {
        static const double shot_ticks[6] = {
            40.0, 50.0, 60.0, 70.0, 80.0, 90.0
        };
        const double previous = guard->shoot_seconds * 70.0;
        guard->shoot_seconds += seconds;
        const double ticks = guard->shoot_seconds * 70.0;
        guard->shoot_frame = ticks < 30.0 || ticks >= 90.0 ? 0 :
                             ((int)((ticks - 30.0) / 10.0) & 1) ? 2 : 1;
        for (size_t shot = 0; shot < 6; ++shot) {
            if (previous < shot_ticks[shot] && ticks >= shot_ticks[shot] &&
                guard_sees_player(game, guard)) {
                const double dx = fabs(game->player.x - guard->x);
                const double dy = fabs(game->player.y - guard->y);
                const int distance = (int)ceil(dx > dy ? dx : dy) * 2 / 3;
                const int chance = 256 - distance * 16;
                if ((int)(game_random(game) & 0xffu) < chance) {
                    const int divisor = distance < 2 ? 4 : distance < 4 ? 8 : 16;
                    const int damage = 1 +
                                       (int)(game_random(game) & 0xffu) / divisor;
                    game->health -= damage;
                    if (game->health <= 0)
                        kill_player(game);
                    game->damage_seconds = 0.15;
                }
            }
        }
        if (ticks >= 100.0) {
            guard->shooting = 0;
            guard->shoot_frame = 0;
            guard->shoot_seconds = 0.0;
        }
        return;
    }
    const double first_end = guard->kind == ENEMY_OFFICER ? 6.0 : 20.0;
    const double shot_time = guard->kind == ENEMY_OFFICER ? 26.0 : 40.0;
    const double total_time = guard->kind == ENEMY_OFFICER ? 36.0 : 60.0;
    const double previous = guard->shoot_seconds;
    guard->shoot_seconds += seconds;
    const double ticks = guard->shoot_seconds * 70.0;
    guard->shoot_frame = ticks < first_end ? 0 : ticks < shot_time ? 1 : 2;
    if (previous < shot_time / 70.0 &&
        guard->shoot_seconds >= shot_time / 70.0 &&
        guard_sees_player(game, guard)) {
        const double dx = fabs(game->player.x - guard->x);
        const double dy = fabs(game->player.y - guard->y);
        const int distance = (int)ceil(dx > dy ? dx : dy);
        int chance = 256 - distance * 16;
        if (chance < 0)
            chance = 0;
        if ((int)(game_random(game) & 0xffu) < chance) {
            const int divisor = distance < 2 ? 4 : distance < 4 ? 8 : 16;
            const int damage = 1 + (int)(game_random(game) & 0xffu) / divisor;
            game->health -= damage;
            if (game->health <= 0)
                kill_player(game);
            game->damage_seconds = 0.15;
        }
    }
    if (ticks >= total_time) {
        guard->shooting = 0;
        guard->shoot_frame = 0;
        guard->shoot_seconds = 0.0;
    }
}

static void update_guards(GameState *game, double seconds)
{
    for (size_t index = 0; index < game->guard_count; ++index) {
        Guard *guard = &game->guards[index];
        if (!guard->active)
            continue;
        if (guard->dead) {
            guard->death_seconds += seconds;
            continue;
        }
        if (guard->kind == ENEMY_GHOST) {
            const double dx = game->player.x - guard->x;
            const double dy = game->player.y - guard->y;
            if (fabs(dx) <= 1.0 && fabs(dy) <= 1.0) {
                guard->contact_damage += seconds * 140.0;
                const int damage = (int)guard->contact_damage;
                guard->contact_damage -= damage;
                if (damage) {
                    game->health -= damage;
                    if (game->health <= 0)
                        kill_player(game);
                    game->damage_seconds = 0.15;
                }
            } else {
                chase_player(game, guard, dog_patrol_speed * seconds);
            }
            guard->animation_seconds = fmod(guard->animation_seconds + seconds,
                                            20.0 / 70.0);
            guard->frame = guard->animation_seconds * 70.0 < 10.0 ? 0 : 1;
            continue;
        }
        if (guard->shooting) {
            guard_shoot(game, guard, seconds);
            continue;
        }

        if (!guard->alerted && guard_sees_player(game, guard))
            guard->alerted = 1;
        if (!guard->alerted && !guard->patrol)
            continue;

        const double player_x = game->player.x - guard->x;
        const double player_y = game->player.y - guard->y;
        const int attack_range = guard->kind == ENEMY_DOG
                                     ? fabs(player_x) <= 1.0 &&
                                       fabs(player_y) <= 1.0
                                     : player_x * player_x + player_y * player_y <= 16.0;
        if (guard->alerted && attack_range &&
            guard_sees_player(game, guard)) {
            guard->shooting = 1;
            guard->shoot_seconds = 0.0;
            guard->shoot_frame = 0;
            continue;
        }

        if (guard->alerted) {
            const double speed = guard->kind == ENEMY_DOG
                                     ? dog_patrol_speed * 2.0
                                     : guard_patrol_speed *
                                           (guard->kind == ENEMY_OFFICER ? 5.0 :
                                            guard->kind == ENEMY_SS ? 4.0 : 3.0);
            chase_player(game, guard,
                         speed * seconds);
        } else {
            if (guard->move_remaining <= 0.0) {
                guard->x = floor(guard->x) + 0.5;
                guard->y = floor(guard->y) + 0.5;
                const size_t cell = (size_t)guard->y * MAP_SIDE +
                                    (size_t)guard->x;
                const uint16_t arrow = game->map.planes[1][cell];
                /* ponytail: cardinal paths suffice until diagonal movement exists. */
                if (arrow >= 90 && arrow <= 96 && !(arrow & 1u))
                    guard->direction = (arrow - 90) / 2;
                guard->move_remaining = 1.0;
            }
            const double distance = (guard->kind == ENEMY_DOG
                                         ? dog_patrol_speed : guard_patrol_speed) *
                                    seconds;
            const double movement = distance < guard->move_remaining
                                        ? distance : guard->move_remaining;
            if (move_guard(game, guard, guard->direction, movement))
                guard->move_remaining -= movement;
        }

        const double cycle = guard->alerted ? 42.0 : 80.0;
        guard->animation_seconds = fmod(guard->animation_seconds + seconds,
                                        cycle / 70.0);
        const double ticks = guard->animation_seconds * 70.0;
        if (guard->alerted)
            guard->frame = ticks < 13.0 ? 0 : ticks < 21.0 ? 1 :
                           ticks < 34.0 ? 2 : 3;
        else
            guard->frame = ticks < 25.0 ? 0 : ticks < 40.0 ? 1 :
                           ticks < 65.0 ? 2 : 3;
    }
}

void game_init(GameState *game, const WolfMap *map, uint32_t random_seed)
{
    memset(game, 0, sizeof(*game));
    game->map = *map;
    player_init(&game->player, &game->map);
    game->random_state = random_seed ? random_seed : 1;
    game->initial_random_seed = game->random_state;
    game->health = 100;
    game->ammo = 8;
    game->lives = 3;
    game->weapons = (1u << WEAPON_KNIFE) | (1u << WEAPON_PISTOL);
    game->current_weapon = WEAPON_PISTOL;

    for (size_t cell = 0; cell < MAP_CELLS; ++cell)
        game->door_at[cell] = -1;
    for (size_t cell = 0; cell < MAP_CELLS; ++cell) {
        int vertical;
        DoorKind kind;
        if (!door_from_tile(game->map.planes[0][cell], &vertical, &kind))
            continue;
        Door *door = &game->doors[game->door_count];
        door->x = cell % MAP_SIDE;
        door->y = cell / MAP_SIDE;
        door->vertical = vertical;
        door->kind = kind;
        door->action = DOOR_CLOSED;
        game->door_at[cell] = (int16_t)game->door_count++;
    }

    for (size_t cell = 0; cell < MAP_CELLS; ++cell) {
        const uint16_t code = game->map.planes[1][cell];
        if (code == 98) {
            ++game->secret_total;
            continue;
        }
        if (code == 214) {
            Guard *guard = &game->guards[game->guard_count++];
            guard->x = cell % MAP_SIDE + 0.5;
            guard->y = cell / MAP_SIDE + 0.5;
            guard->direction = 3;
            guard->active = 1;
            guard->kind = ENEMY_HANS;
            guard->health = 950;
            continue;
        }
        if (code >= 224 && code <= 227) {
            Guard *guard = &game->guards[game->guard_count++];
            guard->x = cell % MAP_SIDE + 0.5;
            guard->y = cell / MAP_SIDE + 0.5;
            guard->direction = 0;
            guard->active = 1;
            guard->alerted = 1;
            guard->kind = ENEMY_GHOST;
            guard->variant = code - 224;
            continue;
        }
        if ((code >= 108 && code <= 123) ||
            (code >= 126 && code <= 133) ||
            (code >= 134 && code <= 141)) {
            const int dog = code >= 134;
            const int ss = code >= 126 && code <= 133;
            const int officer = !dog && !ss && code >= 116;
            const uint16_t base = dog ? 134 : ss ? 126 : officer ? 116 : 108;
            Guard *guard = &game->guards[game->guard_count++];
            guard->x = cell % MAP_SIDE + 0.5;
            guard->y = cell / MAP_SIDE + 0.5;
            guard->direction = (code - base) % 4;
            guard->patrol = code >= base + 4;
            guard->active = 1;
            guard->kind = dog ? ENEMY_DOG :
                          ss ? ENEMY_SS : officer ? ENEMY_OFFICER : ENEMY_GUARD;
            guard->health = dog ? 1 : ss ? 100 : officer ? 50 : 25;
            continue;
        }
        if (code < 23 || code > 70)
            continue;
        StaticObject *object = &game->statics[game->static_count++];
        object->x = cell % MAP_SIDE;
        object->y = cell / MAP_SIDE;
        object->sprite = code - 21;
        object->active = 1;
        object->kind = static_kinds[code - 23];
        if (object->kind == STATIC_BLOCKING)
            game->blocked[cell] = 1;
        if (is_treasure(object->kind))
            ++game->treasure_total;
    }
}

void game_next_level(GameState *game, const WolfMap *map, uint32_t random_seed)
{
    const int health = game->health;
    const int ammo = game->ammo;
    const int lives = game->lives;
    const int score = game->score;
    const unsigned int weapons = game->weapons;
    const int current_weapon = game->current_weapon;
    game_init(game, map, random_seed);
    game->health = health;
    game->ammo = ammo;
    game->lives = lives;
    game->score = score;
    game->weapons = weapons;
    game->current_weapon = current_weapon;
}

void game_update(GameState *game, const PlayerCommand *command, double seconds)
{
    if (seconds < 0.0)
        return;
    if (seconds > 0.05)
        seconds = 0.05;
    if (game->level_complete)
        return;
    if (game->health <= 0)
        kill_player(game);
    if (game->player_dead) {
        game->player_death_seconds += seconds;
        if (command->use_pressed && game->lives > 0) {
            const WolfMap map = game->map;
            const uint32_t seed = game->initial_random_seed;
            const int lives = game->lives;
            game_init(game, &map, seed);
            game->lives = lives;
        }
        return;
    }
    player_rotate(&game->player, command->look_radians);
    if (!game->weapon_frame && game->ammo <= 0 &&
        game->current_weapon != WEAPON_KNIFE)
        game->current_weapon = WEAPON_KNIFE;
    if (!game->weapon_frame && command->requested_weapon >= 1 &&
        command->requested_weapon <= 4) {
        const int weapon = command->requested_weapon - 1;
        if (game->weapons & (1u << weapon))
            game->current_weapon = weapon;
    }
    if (command->use_pressed)
        use_adjacent_door(game);
    const int automatic = game->current_weapon == WEAPON_MACHINE_GUN ||
                          game->current_weapon == WEAPON_CHAIN_GUN;
    if (!game->weapon_frame &&
        (command->attack_pressed || (automatic && command->attack_held)))
        player_attack(game);
    update_weapon(game, seconds);
    if (game->damage_seconds > 0.0) {
        game->damage_seconds -= seconds;
        if (game->damage_seconds < 0.0)
            game->damage_seconds = 0.0;
    }
    update_doors(game, seconds);
    update_guards(game, seconds);
    if (game->player_dead)
        return;
    player_update(&game->player, game->map.planes[0], game->blocked,
                  game->door_at, game->doors, command->forward,
                  command->turn, seconds);
    collect_at_player(game);
}

uint32_t game_random(GameState *game)
{
    uint32_t value = game->random_state;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->random_state = value;
    return value;
}
