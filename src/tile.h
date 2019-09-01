#ifndef TILE_H
#define TILE_H

#pragma warning(push, 0)
#include <FL/Fl_RGB_Image.H>
#pragma warning(pop)

#include "config.h"
#include "tileset.h"

#define NUM_TILE_PIXELS (TILE_SIZE * TILE_SIZE)

#define SPACE_TILE_ID 0x7F
#define SPACE_COLOR 0xFFFFFF00 // white

typedef Fl_Color Tile[NUM_TILE_PIXELS];

bool is_blank_tile(Tile &tile);
bool are_identical_tiles(Tile &t1, Tile &t2, Tilemap_Format fmt, bool &x_flip, bool &y_flip);
Tile *get_image_tiles(Fl_RGB_Image *img, size_t &n);

#endif
