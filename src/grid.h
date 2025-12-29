
#ifndef GRID_H
#define GRID_H

#ifndef GRID_WIDTH
#define GRID_WIDTH 40
#endif

#ifndef GRID_HEIGHT
#define GRID_HEIGHT 40
#endif

#include <stdint.h>

#include "graphics.h"
#include "utils.h"

typedef color state2color(uint8_t state);

struct grid_state_t{
    unsigned int is_dirty : 1;
    // allocate half a byte of state info for every cell. Each cell can have 2^4 individual states.
    unsigned int private_data : 4;
};

struct grid_t {
    struct graphics_context gc;
    state2color *cell_converter;
    struct grid_state_t grid_state[GRID_WIDTH][GRID_HEIGHT];
    vec2 position;
    vec2 size;
};

void init_grid(struct grid_t *grid, struct graphics_context gc, vec2 position, vec2 size, uint8_t init_state, state2color cell_converter);

void draw_grid(struct grid_t *grid);

void change_state(struct grid_t *grid, vec2 cellidx, uint8_t state);

#endif