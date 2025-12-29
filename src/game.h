#ifndef GAME_H
#define GAME_H
#include "graphics.h"
#include <stdint.h>

typedef uint16_t read_key_blocking_t(void);

void start_game(struct graphics_context ctx, read_key_blocking_t readkey);

#endif