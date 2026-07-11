#ifndef VSWAP_H
#define VSWAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum { WALL_SIZE = 64, WALL_PIXELS = WALL_SIZE * WALL_SIZE };

int vswap_read_first_wall(FILE *file, uint8_t pixels[WALL_PIXELS],
                          char *error, size_t error_size);

#endif
