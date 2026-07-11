#include "map.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const uint8_t carmack[] = {
        1, 0, 2, 0, 2, 0xa7, 2,
        2, 0xa8, 0, 0, 0, 0xa7, 0x55
    };
    uint16_t words[7];
    assert(map_carmack_expand(carmack, sizeof(carmack), words, 7));
    assert(words[0] == 1 && words[1] == 2);
    assert(words[2] == 1 && words[3] == 2);
    assert(words[4] == 1 && words[5] == 2);
    assert(words[6] == 0xa755);
    assert(!map_carmack_expand(carmack, 6, words, 7));

    const uint16_t rlew[] = { 5, 0xabcd, 3, 9, 7 };
    uint16_t cells[5];
    assert(map_rlew_expand(rlew, 5, 0xabcd, cells, 5));
    assert(cells[0] == 5 && cells[1] == 9 && cells[3] == 9 && cells[4] == 7);
    assert(!map_rlew_expand(rlew, 4, 0xabcd, cells, 5));
    puts("MAP OK");
    return 0;
}
