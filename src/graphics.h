#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

typedef struct {uint8_t red; uint8_t green; uint8_t blue;} color;

typedef void draw_block(int width, int height, int leftX, int lowerY, color color);

struct graphics_context {
    draw_block *draw_block;
    int width;
    int height;
};

#endif