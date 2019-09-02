#ifndef TILEMAP_FORMAT_H
#define TILEMAP_FORMAT_H

#define NUM_FORMATS 7

enum Tilemap_Format { PLAIN, RLE, FF_END, RLE_FF_END, RLE_NYBBLES, XY_FLIP, TILE_ATTR };

int format_tileset_size(Tilemap_Format fmt);
const char *format_name(Tilemap_Format fmt);
const char *format_extension(Tilemap_Format fmt);

#endif
