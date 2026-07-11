#include "vswap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint16_t get_u16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | (uint16_t)bytes[1] << 8);
}

static uint32_t get_u32(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 |
           (uint32_t)bytes[2] << 16 | (uint32_t)bytes[3] << 24;
}

static int read_file(FILE *file, uint8_t **data, size_t *size)
{
    if (fseek(file, 0, SEEK_END) != 0)
        return 0;
    const long length = ftell(file);
    if (length <= 0 || fseek(file, 0, SEEK_SET) != 0)
        return 0;
    *data = malloc((size_t)length);
    if (!*data)
        return 0;
    if (fread(*data, 1, (size_t)length, file) != (size_t)length) {
        free(*data);
        *data = NULL;
        return 0;
    }
    *size = (size_t)length;
    return 1;
}

int vswap_decode_sprite(const uint8_t *data, size_t size, VSwapSprite *sprite,
                        char *error, size_t error_size)
{
    if (size < 4) {
        snprintf(error, error_size, "sprite VSWAP truncado");
        return 0;
    }
    const uint16_t left = get_u16(data);
    const uint16_t right = get_u16(data + 2);
    if (left > right || right >= WALL_SIZE ||
        4 + (size_t)(right - left + 1) * 2 > size) {
        snprintf(error, error_size, "limites do sprite VSWAP inválidos");
        return 0;
    }

    memset(sprite, 0, sizeof(*sprite));
    sprite->left = left;
    sprite->right = right;
    for (size_t column = left; column <= right; ++column) {
        size_t command = get_u16(data + 4 + (column - left) * 2);
        if (command >= size) {
            snprintf(error, error_size, "offset de coluna do sprite inválido");
            return 0;
        }
        for (;;) {
            if (command + 2 > size) {
                snprintf(error, error_size, "comando do sprite truncado");
                return 0;
            }
            const uint16_t end_twice = get_u16(data + command);
            if (!end_twice)
                break;
            if (command + 6 > size) {
                snprintf(error, error_size, "post do sprite truncado");
                return 0;
            }
            const size_t source = get_u16(data + command + 2);
            const uint16_t start_twice = get_u16(data + command + 4);
            if ((start_twice & 1) || (end_twice & 1) ||
                start_twice >= end_twice || end_twice > WALL_SIZE * 2) {
                snprintf(error, error_size, "post do sprite inválido");
                return 0;
            }
            const size_t start = start_twice / 2;
            const size_t end = end_twice / 2;
            for (size_t y = start; y < end; ++y) {
                const size_t pixel_offset = (uint16_t)(source + y);
                if (pixel_offset >= size) {
                    snprintf(error, error_size, "pixels do sprite fora da página");
                    return 0;
                }
                const size_t output = y * WALL_SIZE + column;
                sprite->pixels[output] = data[pixel_offset];
                sprite->mask[output] = 1;
            }
            command += 6;
        }
    }
    return 1;
}

static int decode_wall(const uint8_t *data, size_t size, uint8_t *pixels)
{
    if (size != WALL_PIXELS)
        return 0;
    for (size_t y = 0; y < WALL_SIZE; ++y)
        for (size_t x = 0; x < WALL_SIZE; ++x)
            pixels[y * WALL_SIZE + x] = data[x * WALL_SIZE + y];
    return 1;
}

int vswap_load(FILE *file, VSwap *vswap, char *error, size_t error_size)
{
    memset(vswap, 0, sizeof(*vswap));
    if (!read_file(file, &vswap->data, &vswap->data_size) || vswap->data_size < 6) {
        snprintf(error, error_size, "não foi possível ler VSWAP");
        return 0;
    }

    vswap->chunk_count = get_u16(vswap->data);
    vswap->sprite_start = get_u16(vswap->data + 2);
    vswap->sound_start = get_u16(vswap->data + 4);
    const size_t header_size = 6 + vswap->chunk_count * 6;
    if (!vswap->chunk_count || !vswap->sprite_start ||
        vswap->sprite_start > vswap->sound_start ||
        vswap->sound_start > vswap->chunk_count || header_size > vswap->data_size) {
        snprintf(error, error_size, "cabeçalho VSWAP inválido");
        vswap_free(vswap);
        return 0;
    }

    vswap->offsets = malloc(vswap->chunk_count * sizeof(*vswap->offsets));
    vswap->lengths = malloc(vswap->chunk_count * sizeof(*vswap->lengths));
    if (!vswap->offsets || !vswap->lengths) {
        snprintf(error, error_size, "memória insuficiente para índice VSWAP");
        vswap_free(vswap);
        return 0;
    }
    for (size_t page = 0; page < vswap->chunk_count; ++page) {
        const uint32_t offset = get_u32(vswap->data + 6 + page * 4);
        const uint16_t length = get_u16(
            vswap->data + 6 + vswap->chunk_count * 4 + page * 2);
        if ((!offset != !length) ||
            (offset && (offset < header_size || offset > vswap->data_size ||
                        length > vswap->data_size - offset))) {
            snprintf(error, error_size, "chunk %zu do VSWAP inválido", page);
            vswap_free(vswap);
            return 0;
        }
        vswap->offsets[page] = offset;
        vswap->lengths[page] = length;
    }

    vswap->walls.count = vswap->sprite_start;
    vswap->walls.pixels = calloc(vswap->walls.count, WALL_PIXELS);
    vswap->sprite_count = vswap->sound_start - vswap->sprite_start;
    vswap->sprites = calloc(vswap->sprite_count, sizeof(*vswap->sprites));
    if (!vswap->walls.pixels || (vswap->sprite_count && !vswap->sprites)) {
        snprintf(error, error_size, "memória insuficiente para recursos VSWAP");
        vswap_free(vswap);
        return 0;
    }

    for (size_t page = 0; page < vswap->walls.count; ++page) {
        if (!vswap->offsets[page])
            continue;
        if (!decode_wall(vswap->data + vswap->offsets[page], vswap->lengths[page],
                         vswap->walls.pixels + page * WALL_PIXELS)) {
            snprintf(error, error_size, "parede %zu do VSWAP inválida", page);
            vswap_free(vswap);
            return 0;
        }
    }
    for (size_t sprite = 0; sprite < vswap->sprite_count; ++sprite) {
        const size_t page = vswap->sprite_start + sprite;
        if (!vswap->offsets[page])
            continue;
        if (!vswap_decode_sprite(vswap->data + vswap->offsets[page],
                                 vswap->lengths[page], &vswap->sprites[sprite],
                                 error, error_size)) {
            vswap_free(vswap);
            return 0;
        }
    }
    return 1;
}

void vswap_free(VSwap *vswap)
{
    free(vswap->offsets);
    free(vswap->lengths);
    free(vswap->data);
    free(vswap->walls.pixels);
    free(vswap->sprites);
    memset(vswap, 0, sizeof(*vswap));
}

int vswap_read_first_wall(FILE *file, uint8_t pixels[WALL_PIXELS],
                          char *error, size_t error_size)
{
    VSwap vswap;
    if (!vswap_load(file, &vswap, error, error_size))
        return 0;
    memcpy(pixels, vswap.walls.pixels, WALL_PIXELS);
    vswap_free(&vswap);
    return 1;
}

int vswap_load_walls(FILE *file, VSwapWalls *walls,
                     char *error, size_t error_size)
{
    VSwap vswap;
    if (!vswap_load(file, &vswap, error, error_size))
        return 0;
    *walls = vswap.walls;
    vswap.walls = (VSwapWalls){0};
    vswap_free(&vswap);
    return 1;
}

void vswap_free_walls(VSwapWalls *walls)
{
    free(walls->pixels);
    walls->pixels = NULL;
    walls->count = 0;
}
