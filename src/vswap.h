#ifndef VSWAP_H
#define VSWAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum { WALL_SIZE = 64, WALL_PIXELS = WALL_SIZE * WALL_SIZE };

typedef struct {
    size_t count;
    uint8_t *pixels;
} VSwapWalls;

typedef struct {
    uint8_t pixels[WALL_PIXELS];
    uint8_t mask[WALL_PIXELS];
    uint16_t left;
    uint16_t right;
} VSwapSprite;

typedef struct {
    size_t chunk_count;
    size_t sprite_start;
    size_t sound_start;
    uint32_t *offsets;
    uint16_t *lengths;
    uint8_t *data;
    size_t data_size;
    VSwapWalls walls;
    size_t sprite_count;
    VSwapSprite *sprites;
} VSwap;

int vswap_load(FILE *file, VSwap *vswap, char *error, size_t error_size);
int vswap_decode_sprite(const uint8_t *data, size_t size, VSwapSprite *sprite,
                        char *error, size_t error_size);
void vswap_free(VSwap *vswap);

int vswap_read_first_wall(FILE *file, uint8_t pixels[WALL_PIXELS],
                          char *error, size_t error_size);
int vswap_load_walls(FILE *file, VSwapWalls *walls,
                     char *error, size_t error_size);
void vswap_free_walls(VSwapWalls *walls);

#endif
