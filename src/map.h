#ifndef MAP_H
#define MAP_H

#include <stddef.h>
#include <stdint.h>

enum { MAP_SIDE = 64, MAP_CELLS = MAP_SIDE * MAP_SIDE };

typedef struct {
    uint16_t planes[2][MAP_CELLS];
    int player_x;
    int player_y;
    int player_direction;
    char name[17];
} WolfMap;

int map_carmack_expand(const uint8_t *input, size_t input_size,
                       uint16_t *output, size_t output_words);
int map_rlew_expand(const uint16_t *input, size_t input_words, uint16_t tag,
                    uint16_t *output, size_t output_words);
int map_load(const char *directory, const char *edition, size_t map_index,
             WolfMap *map, char *error, size_t error_size);

#endif
