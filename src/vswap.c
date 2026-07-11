#include "vswap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int read_u16(FILE *file, uint16_t *value)
{
    uint8_t bytes[2];
    if (fread(bytes, 1, sizeof(bytes), file) != sizeof(bytes))
        return 0;
    *value = (uint16_t)(bytes[0] | (uint16_t)bytes[1] << 8);
    return 1;
}

static int read_u32(FILE *file, uint32_t *value)
{
    uint8_t bytes[4];
    if (fread(bytes, 1, sizeof(bytes), file) != sizeof(bytes))
        return 0;
    *value = (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 |
             (uint32_t)bytes[2] << 16 | (uint32_t)bytes[3] << 24;
    return 1;
}

static int read_wall(FILE *file, size_t page, uint8_t pixels[WALL_PIXELS],
                     char *error, size_t error_size)
{
    uint16_t chunks, sprite_start, sound_start, length;
    uint32_t offset;

    if (fseek(file, 0, SEEK_END) != 0) {
        snprintf(error, error_size, "não foi possível medir VSWAP");
        return 0;
    }
    const long file_size = ftell(file);
    rewind(file);

    if (file_size < 0 || !read_u16(file, &chunks) ||
        !read_u16(file, &sprite_start) || !read_u16(file, &sound_start)) {
        snprintf(error, error_size, "cabeçalho VSWAP truncado");
        return 0;
    }

    const uint32_t header_size = 6u + (uint32_t)chunks * 6u;
    if (!chunks || !sprite_start || page >= sprite_start || sprite_start > sound_start ||
        sound_start > chunks || header_size > (uint32_t)file_size ||
        fseek(file, 6L + (long)page * 4L, SEEK_SET) != 0 ||
        !read_u32(file, &offset) ||
        fseek(file, 6L + (long)chunks * 4L + (long)page * 2L, SEEK_SET) != 0 ||
        !read_u16(file, &length)) {
        snprintf(error, error_size, "cabeçalho VSWAP inválido");
        return 0;
    }

    if (!offset && !length) {
        memset(pixels, 0, WALL_PIXELS);
        return 1;
    }
    if (length != WALL_PIXELS || offset < header_size ||
        (uint64_t)offset + length > (uint64_t)file_size ||
        fseek(file, (long)offset, SEEK_SET) != 0) {
        snprintf(error, error_size, "parede %zu do VSWAP inválida", page);
        return 0;
    }

    uint8_t columns[WALL_PIXELS];
    if (fread(columns, 1, sizeof(columns), file) != sizeof(columns)) {
        snprintf(error, error_size, "parede %zu do VSWAP truncada", page);
        return 0;
    }

    for (size_t y = 0; y < WALL_SIZE; ++y)
        for (size_t x = 0; x < WALL_SIZE; ++x)
            pixels[y * WALL_SIZE + x] = columns[x * WALL_SIZE + y];
    return 1;
}

int vswap_read_first_wall(FILE *file, uint8_t pixels[WALL_PIXELS],
                          char *error, size_t error_size)
{
    return read_wall(file, 0, pixels, error, error_size);
}

int vswap_load_walls(FILE *file, VSwapWalls *walls,
                     char *error, size_t error_size)
{
    uint16_t chunks, sprite_start, sound_start;
    rewind(file);
    if (!read_u16(file, &chunks) || !read_u16(file, &sprite_start) ||
        !read_u16(file, &sound_start) || !sprite_start ||
        sprite_start > sound_start || sound_start > chunks) {
        snprintf(error, error_size, "cabeçalho VSWAP inválido");
        return 0;
    }

    walls->pixels = malloc((size_t)sprite_start * WALL_PIXELS);
    if (!walls->pixels) {
        snprintf(error, error_size, "memória insuficiente para paredes VSWAP");
        return 0;
    }
    walls->count = sprite_start;
    for (size_t page = 0; page < walls->count; ++page) {
        if (!read_wall(file, page, walls->pixels + page * WALL_PIXELS,
                       error, error_size)) {
            vswap_free_walls(walls);
            return 0;
        }
    }
    return 1;
}

void vswap_free_walls(VSwapWalls *walls)
{
    free(walls->pixels);
    walls->pixels = NULL;
    walls->count = 0;
}
