#include "render.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#include "game_palette.h"
#include "raycast.h"

typedef struct {
    double x;
    double y;
    double distance_squared;
    size_t sprite;
} VisibleSprite;

static uint32_t palette_color(uint8_t index, int brightness)
{
    const uint8_t *rgb = &game_palette[index * 3];
    return 0xff000000u | (uint32_t)(rgb[0] * 255 / 63 * brightness / 4) << 16 |
           (uint32_t)(rgb[1] * 255 / 63 * brightness / 4) << 8 |
           (uint32_t)(rgb[2] * 255 / 63 * brightness / 4);
}

static int compare_distance(const void *left, const void *right)
{
    const VisibleSprite *a = left;
    const VisibleSprite *b = right;
    if (a->distance_squared < b->distance_squared)
        return 1;
    if (a->distance_squared > b->distance_squared)
        return -1;
    return 0;
}

static size_t guard_sprite(const Guard *guard, const Player *player)
{
    static const size_t walking_sprite[4] = {58, 66, 74, 82};
    if (guard->dead) {
        const int frame = (int)(guard->death_seconds * 70.0 / 15.0);
        return frame < 3 ? 91 + frame : 95;
    }
    if (guard->shooting)
        return 96 + guard->shoot_frame;
    const double pi = 3.14159265358979323846;
    const double facing = -guard->direction * pi / 2.0;
    double relative = facing - atan2(player->y - guard->y,
                                     player->x - guard->x);
    while (relative < 0.0)
        relative += 2.0 * pi;
    while (relative >= 2.0 * pi)
        relative -= 2.0 * pi;
    const size_t base = guard->patrol || guard->alerted
                            ? walking_sprite[guard->frame] : 50;
    return base + ((int)(relative / (pi / 4.0) + 0.5) & 7);
}

static size_t collect_sprites(VisibleSprite sprites[MAP_CELLS * 2],
                              const GameState *game, const VSwap *vswap)
{
    size_t count = 0;
    for (size_t index = 0; index < game->static_count; ++index) {
        const StaticObject *object = &game->statics[index];
        if (!object->active || object->sprite >= vswap->sprite_count)
            continue;
        const double x = object->x + 0.5;
        const double y = object->y + 0.5;
        const double dx = x - game->player.x;
        const double dy = y - game->player.y;
        sprites[count++] = (VisibleSprite){
            x, y, dx * dx + dy * dy, object->sprite
        };
    }
    for (size_t index = 0; index < game->guard_count; ++index) {
        const Guard *guard = &game->guards[index];
        const size_t sprite = guard_sprite(guard, &game->player);
        if (!guard->active || sprite >= vswap->sprite_count)
            continue;
        const double dx = guard->x - game->player.x;
        const double dy = guard->y - game->player.y;
        sprites[count++] = (VisibleSprite){
            guard->x, guard->y, dx * dx + dy * dy, sprite
        };
    }
    qsort(sprites, count, sizeof(*sprites), compare_distance);
    return count;
}

static void draw_sprite(uint32_t pixels[RENDER_WIDTH * RENDER_HEIGHT],
                         const VSwapSprite *sprite, const VisibleSprite *object,
                        const Player *player, const double depth[RENDER_WIDTH])
{
    const double direction_x = cos(player->angle);
    const double direction_y = sin(player->angle);
    const double plane_x = -direction_y * 0.66;
    const double plane_y = direction_x * 0.66;
    const double relative_x = object->x - player->x;
    const double relative_y = object->y - player->y;
    const double determinant = plane_x * direction_y - direction_x * plane_y;
    const double transformed_x =
        (direction_y * relative_x - direction_x * relative_y) / determinant;
    const double transformed_y =
        (-plane_y * relative_x + plane_x * relative_y) / determinant;
    if (transformed_y <= 0.01)
        return;

    const int screen_x = (int)(RENDER_WIDTH / 2.0 *
                               (1.0 + transformed_x / transformed_y));
    const int sprite_height = abs((int)(RENDER_HEIGHT / transformed_y));
    if (!sprite_height)
        return;
    const int sprite_width = sprite_height;
    const int left = screen_x - sprite_width / 2;
    const int top = (RENDER_HEIGHT - sprite_height) / 2;

    for (int x = left; x < left + sprite_width; ++x) {
        if (x < 0 || x >= RENDER_WIDTH || transformed_y >= depth[x])
            continue;
        const int texture_x = (x - left) * WALL_SIZE / sprite_width;
        if (texture_x < sprite->left || texture_x > sprite->right)
            continue;
        for (int y = top; y < top + sprite_height; ++y) {
            if (y < 0 || y >= RENDER_HEIGHT)
                continue;
            const int texture_y = (y - top) * WALL_SIZE / sprite_height;
            const size_t source = (size_t)texture_y * WALL_SIZE + texture_x;
            if (sprite->mask[source])
                pixels[y * RENDER_WIDTH + x] =
                    palette_color(sprite->pixels[source], 4);
        }
    }
}

static void draw_weapon(uint32_t pixels[RENDER_WIDTH * RENDER_HEIGHT],
                        const GameState *game, const VSwap *vswap)
{
    static const size_t ready_sprite[4] = {416, 421, 426, 431};
    if (game->current_weapon < WEAPON_KNIFE ||
        game->current_weapon > WEAPON_CHAIN_GUN)
        return;
    const size_t index = ready_sprite[game->current_weapon] + game->weapon_frame;
    if (index >= vswap->sprite_count)
        return;
    const VSwapSprite *sprite = &vswap->sprites[index];
    const int size = 160;
    const int left = (RENDER_WIDTH - size) / 2;
    const int top = RENDER_HEIGHT - size;
    for (int y = 0; y < size; ++y) {
        const int texture_y = y * WALL_SIZE / size;
        for (int x = 0; x < size; ++x) {
            const int texture_x = x * WALL_SIZE / size;
            const size_t source = (size_t)texture_y * WALL_SIZE + texture_x;
            if (texture_x >= sprite->left && texture_x <= sprite->right &&
                sprite->mask[source])
                pixels[(top + y) * RENDER_WIDTH + left + x] =
                    palette_color(sprite->pixels[source], 4);
        }
    }
}

void render_scene(uint32_t pixels[RENDER_WIDTH * RENDER_HEIGHT],
                  const GameState *game, const VSwap *vswap)
{
    const WolfMap *map = &game->map;
    const Player *player = &game->player;
    for (int y = 0; y < RENDER_HEIGHT; ++y)
        for (int x = 0; x < RENDER_WIDTH; ++x)
            pixels[y * RENDER_WIDTH + x] =
                palette_color(y < RENDER_HEIGHT / 2 ? 0x1d : 0x19, 4);

    double depth[RENDER_WIDTH];
    const double direction_x = cos(player->angle);
    const double direction_y = sin(player->angle);
    const double plane_x = -direction_y * 0.66;
    const double plane_y = direction_x * 0.66;
    for (int x = 0; x < RENDER_WIDTH; ++x) {
        depth[x] = 1e30;
        const double camera_x = 2.0 * x / RENDER_WIDTH - 1.0;
        const double ray_x = direction_x + plane_x * camera_x;
        const double ray_y = direction_y + plane_y * camera_x;
        RayHit hit;
        if (!raycast_hit(map->planes[0], game->door_at, game->doors,
                         player->x, player->y, ray_x, ray_y, &hit))
            continue;
        depth[x] = hit.distance;

        size_t page = 0;
        if (hit.kind == RAY_HIT_DOOR && vswap->walls.count >= 8) {
            const Door *door = &game->doors[hit.door_index];
            size_t base = vswap->walls.count - 8;
            if (door->kind == DOOR_ELEVATOR)
                base += 4;
            else if (door->kind != DOOR_NORMAL)
                base += 6;
            page = base + (door->vertical != 0);
        } else if (hit.tile < 64)
            page = (hit.tile - 1) * 2 + (hit.side == 0);
        else if (hit.tile >= 90 && hit.tile <= 101 && vswap->walls.count >= 8)
            page = vswap->walls.count - 8 + (hit.side == 0);
        if (page >= vswap->walls.count)
            page = 0;

        int texture_x = (int)(hit.wall_position * WALL_SIZE);
        if ((hit.side == 0 && ray_x > 0.0) || (hit.side == 1 && ray_y < 0.0))
            texture_x = WALL_SIZE - 1 - texture_x;
        const int line_height = (int)(RENDER_HEIGHT / hit.distance);
        const int line_start = (RENDER_HEIGHT - line_height) / 2;
        const int draw_start = line_start < 0 ? 0 : line_start;
        int draw_end = line_start + line_height;
        if (draw_end > RENDER_HEIGHT)
            draw_end = RENDER_HEIGHT;
        const uint8_t *texture = vswap->walls.pixels + page * WALL_PIXELS;
        for (int y = draw_start; y < draw_end; ++y) {
            const int texture_y = (y - line_start) * WALL_SIZE / line_height;
            pixels[y * RENDER_WIDTH + x] = palette_color(
                texture[texture_y * WALL_SIZE + texture_x], hit.side ? 3 : 4);
        }
    }

    VisibleSprite sprites[MAP_CELLS * 2];
    const size_t count = collect_sprites(sprites, game, vswap);
    for (size_t index = 0; index < count; ++index)
        draw_sprite(pixels, &vswap->sprites[sprites[index].sprite],
                    &sprites[index], player, depth);
    draw_weapon(pixels, game, vswap);
    if (game->damage_seconds > 0.0) {
        for (size_t index = 0; index < RENDER_WIDTH * RENDER_HEIGHT; ++index) {
            const uint32_t color = pixels[index];
            const uint32_t red = ((color >> 16) & 0xffu) + 255u;
            pixels[index] = 0xff000000u | (red / 2) << 16 |
                            (((color >> 8) & 0xffu) / 2) << 8 |
                            (color & 0xffu) / 2;
        }
    }
}
