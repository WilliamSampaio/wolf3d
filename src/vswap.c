#include "vswap.h"

#include <stdint.h>

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

int vswap_read_first_wall(FILE *file, uint8_t pixels[WALL_PIXELS],
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
    if (!chunks || !sprite_start || sprite_start > sound_start ||
        sound_start > chunks || header_size > (uint32_t)file_size ||
        !read_u32(file, &offset) ||
        fseek(file, 6L + (long)chunks * 4L, SEEK_SET) != 0 ||
        !read_u16(file, &length)) {
        snprintf(error, error_size, "cabeçalho VSWAP inválido");
        return 0;
    }

    if (length != WALL_PIXELS || offset < header_size ||
        (uint64_t)offset + length > (uint64_t)file_size ||
        fseek(file, (long)offset, SEEK_SET) != 0) {
        snprintf(error, error_size, "primeira parede VSWAP inválida");
        return 0;
    }

    uint8_t columns[WALL_PIXELS];
    if (fread(columns, 1, sizeof(columns), file) != sizeof(columns)) {
        snprintf(error, error_size, "primeira parede VSWAP truncada");
        return 0;
    }

    for (size_t y = 0; y < WALL_SIZE; ++y)
        for (size_t x = 0; x < WALL_SIZE; ++x)
            pixels[y * WALL_SIZE + x] = columns[x * WALL_SIZE + y];
    return 1;
}
