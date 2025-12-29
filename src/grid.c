

#include "grid.h"

void init_grid(struct grid_t *grid, struct graphics_context gc, vec2 position, vec2 size, uint8_t init_state, state2color cellconverter) {
    grid->position = position;
    grid->size = size;
    grid->gc = gc;
    grid->cell_converter = cellconverter;
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            grid->grid_state[i][j].private_data = init_state;
            grid->grid_state[i][j].is_dirty = 0;
        }
    }
}

void draw_grid(struct grid_t *grid) {
    int downLeftX, downLeftY, upRightX, upRightY;
    downLeftX = grid->position.x * 2 / 10;
    downLeftY = grid->position.y * 2 / 10;
    upRightX = grid->size.x * 8 / 10;
    upRightY = grid->size.y * 8 / 10;
    color pixelBackground = {100, 0, 0};

    grid->gc.draw_block(downLeftX, downLeftY, upRightX - downLeftX, upRightY - downLeftY, pixelBackground);
    int width = (upRightX - downLeftX) / (GRID_WIDTH * 12 / 10);
    int height = (upRightY - downLeftY) / (GRID_HEIGHT * 12 / 10);;
    int driftX = (upRightX - downLeftX)  / GRID_WIDTH - width;
    driftX /= 2;
    int driftY = (upRightY - downLeftY)  / GRID_WIDTH - height;
    driftY /= 2;
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            int posX = driftX + downLeftX + (upRightX - downLeftX) * i / GRID_WIDTH;
            int posY = driftY + downLeftY + (upRightY - downLeftY) * j / GRID_HEIGHT;
            color pixelCell = grid->cell_converter(grid->grid_state[i][j].private_data);
            grid->gc.draw_block(posX, posY, width, height, pixelCell);
        }
    }
}


void change_state(struct grid_t *grid, vec2 cellidx, uint8_t state) {
    grid->grid_state[cellidx.x][cellidx.y].private_data = state;
}