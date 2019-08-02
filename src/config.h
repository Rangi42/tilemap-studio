#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include "tilemap.h"

class Config {
private:
	static Tilemap::Format _format;
	static uint8_t _start;
	static bool _2x_tiles;
	static bool _rainbow_tiles;
public:
	inline static Tilemap::Format format(void) { return _format; }
	inline static void format(Tilemap::Format f) { _format = f; }
	inline static uint8_t start(void) { return _start; }
	inline static void start(uint8_t s) { _start = s; }
	inline static bool tiles2x(void) { return _2x_tiles; }
	inline static void tiles2x(bool t) { _2x_tiles = t; }
	inline static bool rainbow_tiles(void) { return _rainbow_tiles; }
	inline static void rainbow_tiles(bool r) { _rainbow_tiles = r; }
};

#endif
