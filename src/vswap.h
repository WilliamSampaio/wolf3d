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

int vswap_read_first_wall(FILE *file, uint8_t pixels[WALL_PIXELS],
                          char *error, size_t error_size);
int vswap_load_walls(FILE *file, VSwapWalls *walls,
                     char *error, size_t error_size);
void vswap_free_walls(VSwapWalls *walls);

#endif
