#include "player.h"

#include <math.h>

enum { PLAYER_RADIUS_PERCENT = 20 };

static int is_open(const uint16_t map[MAP_CELLS],
                   const uint8_t blocked[MAP_CELLS],
                   const int16_t door_at[MAP_CELLS], double x, double y)
{
    const int tile_x = (int)floor(x);
    const int tile_y = (int)floor(y);
    if (tile_x < 0 || tile_x >= MAP_SIDE || tile_y < 0 || tile_y >= MAP_SIDE)
        return 0;
    if (blocked && blocked[tile_y * MAP_SIDE + tile_x])
        return 0;
    if (door_at && door_at[tile_y * MAP_SIDE + tile_x] >= 0)
        return 1;
    const uint16_t tile = map[tile_y * MAP_SIDE + tile_x];
    return !tile || tile >= 107;
}

static int fits(const uint16_t map[MAP_CELLS],
                const uint8_t blocked[MAP_CELLS],
                const int16_t door_at[MAP_CELLS],
                const Door *doors, double x, double y)
{
    const double radius = PLAYER_RADIUS_PERCENT / 100.0;
    if (!is_open(map, blocked, door_at, x - radius, y - radius) ||
        !is_open(map, blocked, door_at, x + radius, y - radius) ||
        !is_open(map, blocked, door_at, x - radius, y + radius) ||
        !is_open(map, blocked, door_at, x + radius, y + radius))
        return 0;
    if (!door_at || !doors)
        return 1;
    const int center_x = (int)floor(x);
    const int center_y = (int)floor(y);
    for (int tile_y = center_y - 1; tile_y <= center_y + 1; ++tile_y) {
        for (int tile_x = center_x - 1; tile_x <= center_x + 1; ++tile_x) {
            if (tile_x < 0 || tile_x >= MAP_SIDE ||
                tile_y < 0 || tile_y >= MAP_SIDE)
                continue;
            const int door = door_at[tile_y * MAP_SIDE + tile_x];
            if (door >= 0 && door_blocks_circle(&doors[door],
                                                doors[door].position,
                                                x, y, radius))
                return 0;
        }
    }
    return 1;
}

void player_init(Player *player, const WolfMap *map)
{
    const double pi = 3.14159265358979323846;
    player->x = map->player_x + 0.5;
    player->y = map->player_y + 0.5;
    player->angle = -pi / 2.0 + map->player_direction * pi / 2.0;
}

void player_rotate(Player *player, double radians)
{
    const double pi = 3.14159265358979323846;
    player->angle = fmod(player->angle + radians + pi, 2.0 * pi);
    if (player->angle < 0.0)
        player->angle += 2.0 * pi;
    player->angle -= pi;
}

void player_update(Player *player, const uint16_t map[MAP_CELLS],
                   const uint8_t blocked[MAP_CELLS],
                   const int16_t door_at[MAP_CELLS], const Door *doors,
                   double forward, double turn, double seconds)
{
    if (seconds < 0.0)
        return;
    if (seconds > 0.05)
        seconds = 0.05;

    player_rotate(player, turn * 2.0 * seconds);

    const double distance = forward * 2.5 * seconds;
    const double next_x = player->x + cos(player->angle) * distance;
    const double next_y = player->y + sin(player->angle) * distance;
    if (fits(map, blocked, door_at, doors, next_x, player->y))
        player->x = next_x;
    if (fits(map, blocked, door_at, doors, player->x, next_y))
        player->y = next_y;
}
