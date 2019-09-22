#ifndef TILEMAP_FORMAT_H
#define TILEMAP_FORMAT_H

#define NUM_FORMATS 7

enum Tilemap_Format { PLAIN, TILE_ATTR, TEN_BIT, RLE_NYBBLES, FF_END, XY_FLIP, RLE_FF_END };

int format_tileset_size(Tilemap_Format fmt);
const char *format_name(Tilemap_Format fmt);
const char *format_extension(Tilemap_Format fmt);

#endif
