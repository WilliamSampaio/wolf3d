#include "vswap.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

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
    puts("VSWAP OK");
    return 0;
}
