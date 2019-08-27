#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"

#define NUM_FORMATS 7

enum Tilemap_Format { PLAIN, RLE, FF_END, RLE_FF_END, RLE_NYBBLES, XY_FLIP, TILE_ATTR };

class Config {
private:
	static Tilemap_Format _format;
	static bool _rainbow_tiles;
	static bool _attributes;
public:
	inline static Tilemap_Format format(void) { return _format; }
	inline static void format(Tilemap_Format f) { _format = f; }
	inline static bool rainbow_tiles(void) { return _rainbow_tiles; }
	inline static void rainbow_tiles(bool r) { _rainbow_tiles = r; }
	inline static bool attributes(void) { return _attributes; }
	inline static void attributes(bool a) { _attributes = a; }
};

#endif
