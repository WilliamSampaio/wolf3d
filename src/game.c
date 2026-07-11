#include "game.h"

void game_init(GameState *game, const WolfMap *map, uint32_t random_seed)
{
    game->map = *map;
    player_init(&game->player, &game->map);
    game->random_state = random_seed ? random_seed : 1;
}

void game_update(GameState *game, const PlayerCommand *command, double seconds)
{
    player_rotate(&game->player, command->look_radians);
    player_update(&game->player, game->map.planes[0], command->forward,
                  command->turn, seconds);
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
