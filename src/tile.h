#ifndef TILE_H
#define TILE_H

#pragma warning(push, 0)
#include <FL/Fl_RGB_Image.H>
#pragma warning(pop)

#include "config.h"
#include "tileset.h"

#define NUM_TILE_PIXELS (TILE_SIZE * TILE_SIZE)

#define NORMRGB(c) (uchar)(((c) & 0xF8) | (((c) & 0xF8) >> 5))

typedef Fl_Color Tile[NUM_TILE_PIXELS];

bool is_blank_tile(const Tile &tile, Fl_Color blank_color);
bool are_identical_tiles(const Tile &t1, const Tile &t2, Tilemap_Format fmt, bool &x_flip, bool &y_flip);
Tile *get_image_tiles(Fl_RGB_Image *img, size_t &n, size_t &iw);

#endif
