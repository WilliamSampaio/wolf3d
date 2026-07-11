#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { NEAR_TAG = 0xa7, FAR_TAG = 0xa8, MAP_HEADER_SIZE = 38 };

static uint16_t get_u16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | (uint16_t)bytes[1] << 8);
}

static uint32_t get_u32(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 |
           (uint32_t)bytes[2] << 16 | (uint32_t)bytes[3] << 24;
}

int map_carmack_expand(const uint8_t *input, size_t input_size,
                       uint16_t *output, size_t output_words)
{
    size_t in = 0, out = 0;
    while (out < output_words) {
        if (in + 2 > input_size)
            return 0;
        const uint16_t word = get_u16(input + in);
        const uint8_t tag = word >> 8;
        const size_t count = word & 0xff;
        in += 2;

        if ((tag == NEAR_TAG || tag == FAR_TAG) && !count) {
            if (in >= input_size)
                return 0;
            output[out++] = (uint16_t)(word | input[in++]);
        } else if (tag == NEAR_TAG || tag == FAR_TAG) {
            size_t offset;
            if (tag == NEAR_TAG) {
                if (in >= input_size)
                    return 0;
                const size_t distance = input[in++];
                if (!distance || distance > out)
                    return 0;
                offset = out - distance;
            } else {
                if (in + 2 > input_size)
                    return 0;
                offset = get_u16(input + in);
                in += 2;
                if (offset >= out)
                    return 0;
            }
            if (count > output_words - out)
                return 0;
            for (size_t i = 0; i < count; ++i)
                output[out++] = output[offset++];
        } else {
            output[out++] = word;
        }
    }
    return in == input_size;
}

int map_rlew_expand(const uint16_t *input, size_t input_words, uint16_t tag,
                    uint16_t *output, size_t output_words)
{
    size_t in = 0, out = 0;
    while (out < output_words) {
        if (in >= input_words)
            return 0;
        const uint16_t value = input[in++];
        if (value != tag) {
            output[out++] = value;
            continue;
        }
        if (in + 2 > input_words)
            return 0;
        const size_t count = input[in++];
        const uint16_t repeated = input[in++];
        if (!count || count > output_words - out)
            return 0;
        for (size_t i = 0; i < count; ++i)
            output[out++] = repeated;
    }
    return in == input_words;
}

static uint8_t *read_file(const char *path, size_t *size)
{
    FILE *file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0) {
        if (file)
            fclose(file);
        return NULL;
    }
    const long length = ftell(file);
    if (length <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    uint8_t *bytes = malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *size = (size_t)length;
    return bytes;
}

static int load_plane(const uint8_t *data, size_t data_size, uint32_t start,
                      uint16_t length, uint16_t tag, uint16_t output[MAP_CELLS])
{
    if (length < 4 || start > data_size || length > data_size - start)
        return 0;
    const uint8_t *compressed = data + start;
    const uint16_t expanded_bytes = get_u16(compressed);
    if (expanded_bytes < 4 || expanded_bytes % 2)
        return 0;
    const size_t expanded_words = expanded_bytes / 2;
    uint16_t *expanded = malloc(expanded_words * sizeof(*expanded));
    if (!expanded)
        return 0;
    const int valid = map_carmack_expand(compressed + 2, length - 2,
                                         expanded, expanded_words) &&
                      expanded[0] == MAP_CELLS * 2 &&
                      map_rlew_expand(expanded + 1, expanded_words - 1, tag,
                                      output, MAP_CELLS);
    free(expanded);
    return valid;
}

int map_load_first(const char *directory, const char *edition, WolfMap *map,
                   char *error, size_t error_size)
{
    char path[4096];
    size_t head_size, data_size;
    snprintf(path, sizeof(path), "%s/MAPHEAD.%s", directory, edition);
    uint8_t *head = read_file(path, &head_size);
    snprintf(path, sizeof(path), "%s/GAMEMAPS.%s", directory, edition);
    uint8_t *data = read_file(path, &data_size);
    if (!head || !data || head_size < 6) {
        snprintf(error, error_size, "não foi possível ler MAPHEAD/GAMEMAPS");
        free(head);
        free(data);
        return 0;
    }

    const uint16_t tag = get_u16(head);
    const uint32_t header_offset = get_u32(head + 2);
    if (header_offset == UINT32_MAX || header_offset > data_size ||
        MAP_HEADER_SIZE > data_size - header_offset) {
        snprintf(error, error_size, "cabeçalho do mapa 0 inválido");
        free(head);
        free(data);
        return 0;
    }

    const uint8_t *header = data + header_offset;
    const uint16_t width = get_u16(header + 18);
    const uint16_t height = get_u16(header + 20);
    int valid = width == MAP_SIDE && height == MAP_SIDE;
    for (size_t plane = 0; valid && plane < 2; ++plane)
        valid = load_plane(data, data_size, get_u32(header + plane * 4),
                           get_u16(header + 12 + plane * 2), tag,
                           map->planes[plane]);

    if (!valid) {
        snprintf(error, error_size, "planos do mapa 0 inválidos");
        free(head);
        free(data);
        return 0;
    }

    memcpy(map->name, header + 22, 16);
    map->name[16] = '\0';
    int players = 0;
    for (size_t i = 0; i < MAP_CELLS; ++i) {
        const uint16_t object = map->planes[1][i];
        if (object >= 19 && object <= 22) {
            map->player_x = i % MAP_SIDE;
            map->player_y = i / MAP_SIDE;
            map->player_direction = object - 19;
            ++players;
        }
    }
    free(head);
    free(data);
    if (players != 1) {
        snprintf(error, error_size, "mapa 0 contém %d inícios de jogador", players);
        return 0;
    }
    return 1;
}
