#include "data_files.h"

#include <stdio.h>
#include <string.h>

static const char *files[] = {
    "VSWAP", "GAMEMAPS", "MAPHEAD", "VGADICT",
    "VGAHEAD", "VGAGRAPH", "AUDIOHED", "AUDIOT"
};

static int file_is_not_empty(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        return 0;
    const int result = fgetc(file) != EOF;
    fclose(file);
    return result;
}

const char *data_validate(const char *directory, char *error, size_t error_size)
{
    static const char *editions[] = { "WL6", "WL1" };
    size_t best_missing = sizeof(files) / sizeof(*files) + 1;
    const char *best_edition = editions[0];

    for (size_t edition = 0; edition < sizeof(editions) / sizeof(*editions); ++edition) {
        size_t missing = 0;
        for (size_t i = 0; i < sizeof(files) / sizeof(*files); ++i) {
            char path[4096];
            const int length = snprintf(path, sizeof(path), "%s/%s.%s",
                                        directory, files[i], editions[edition]);
            if (length < 0 || (size_t)length >= sizeof(path) || !file_is_not_empty(path))
                ++missing;
        }
        if (!missing)
            return editions[edition];
        if (missing < best_missing) {
            best_missing = missing;
            best_edition = editions[edition];
        }
    }

    snprintf(error, error_size,
             "dados %s incompletos em '%s': esperados VSWAP, GAMEMAPS, "
             "MAPHEAD, VGADICT, VGAHEAD, VGAGRAPH, AUDIOHED e AUDIOT",
             best_edition, directory);
    return NULL;
}
