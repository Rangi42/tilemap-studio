#ifndef TILE_H
#define TILE_H

#pragma warning(push, 0)
#include <FL/Fl_RGB_Image.H>
#pragma warning(pop)

#include "config.h"
#include "tileset.h"

#define TILE_SIZE 8
#define NUM_TILE_PIXELS (TILE_SIZE * TILE_SIZE)

#define NORMRGB(c) (uchar)(((c) & 0xF8) | (((c) & 0xF8) >> 5))
#define ALT_NORM_MASK 0xF8F8F800 // clear the low 3 bits of each color channel

typedef Fl_Color Tile[NUM_TILE_PIXELS];

bool is_blank_tile(const Tile &tile, Fl_Color blank_color);
bool are_identical_tiles(const Tile &t1, const Tile &t2, bool allow_flip, bool &x_flip, bool &y_flip);
Tile *get_image_tiles(Fl_RGB_Image *img, size_t &n, size_t &iw, bool alt_norm, Fl_Color blank_color);

#endif
