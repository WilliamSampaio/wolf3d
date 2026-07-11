#include "vswap.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "data/shareware-v1.4"
#endif

static void write_u16(FILE *file, uint16_t value)
{
    fputc(value & 0xff, file);
    fputc(value >> 8, file);
}

static void write_u32(FILE *file, uint32_t value)
{
    write_u16(file, value & 0xffff);
    write_u16(file, value >> 16);
}

int main(void)
{
    FILE *file = tmpfile();
    assert(file);
    write_u16(file, 1);
    write_u16(file, 1);
    write_u16(file, 1);
    write_u32(file, 12);
    write_u16(file, WALL_PIXELS);
    for (int x = 0; x < WALL_SIZE; ++x)
        for (int y = 0; y < WALL_SIZE; ++y)
            fputc(x + y, file);
    rewind(file);

    uint8_t pixels[WALL_PIXELS];
    char error[128];
    assert(vswap_read_first_wall(file, pixels, error, sizeof(error)));
    assert(pixels[7 * WALL_SIZE + 3] == 10);
    fclose(file);

    file = tmpfile();
    assert(file);
    write_u16(file, 2);
    write_u16(file, 2);
    write_u16(file, 2);
    write_u32(file, 18);
    write_u32(file, 0);
    write_u16(file, WALL_PIXELS);
    write_u16(file, 0);
    for (int i = 0; i < WALL_PIXELS; ++i)
        fputc(i, file);
    rewind(file);
    VSwapWalls walls = {0};
    assert(vswap_load_walls(file, &walls, error, sizeof(error)));
    assert(walls.count == 2 && walls.pixels[WALL_PIXELS] == 0);
    vswap_free_walls(&walls);
    fclose(file);

    file = tmpfile();
    assert(file);
    fputc(1, file);
    rewind(file);
    assert(!vswap_read_first_wall(file, pixels, error, sizeof(error)));
    fclose(file);

    const uint8_t sprite_data[] = {
        3, 0, 3, 0, 6, 0,
        16, 0, 10, 0, 12, 0,
        0, 0, 0, 0, 42, 43
    };
    VSwapSprite sprite;
    assert(vswap_decode_sprite(sprite_data, sizeof(sprite_data), &sprite,
                               error, sizeof(error)));
    assert(sprite.left == 3 && sprite.right == 3);
    assert(!sprite.mask[5 * WALL_SIZE + 3]);
    assert(sprite.mask[6 * WALL_SIZE + 3] && sprite.pixels[6 * WALL_SIZE + 3] == 42);
    assert(sprite.mask[7 * WALL_SIZE + 3] && sprite.pixels[7 * WALL_SIZE + 3] == 43);
    uint8_t invalid_sprite[sizeof(sprite_data)];
    for (size_t i = 0; i < sizeof(sprite_data); ++i)
        invalid_sprite[i] = sprite_data[i];
    invalid_sprite[4] = 0xff;
    invalid_sprite[5] = 0xff;
    assert(!vswap_decode_sprite(invalid_sprite, sizeof(invalid_sprite), &sprite,
                                error, sizeof(error)));

    char path[4096];
    assert(snprintf(path, sizeof(path), "%s/VSWAP.WL1", TEST_DATA_DIR) > 0);
    file = fopen(path, "rb");
    assert(file);
    VSwap vswap;
    assert(vswap_load(file, &vswap, error, sizeof(error)));
    assert(vswap.walls.count == 106);
    assert(vswap.sprite_count > 48);
    vswap_free(&vswap);
    fclose(file);
    puts("VSWAP OK");
    return 0;
}
