#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>

#include "game.h"
#include "vswap.h"

enum { RENDER_WIDTH = 320, RENDER_HEIGHT = 200 };

void render_scene(uint32_t pixels[RENDER_WIDTH * RENDER_HEIGHT],
                  const GameState *game, const VSwap *vswap);

#endif
