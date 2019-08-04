#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include "tilemap.h"

class Config {
private:
	static Tilemap::Format _format;
	static uint8_t _start;
	static bool _16px_tiles;
	static bool _rainbow_tiles;
	static bool _attributes;
public:
	inline static Tilemap::Format format(void) { return _format; }
	inline static void format(Tilemap::Format f) { _format = f; }
	inline static uint8_t start(void) { return _start; }
	inline static void start(uint8_t s) { _start = s; }
	inline static bool tiles_16px(void) { return _16px_tiles; }
	inline static void tiles_16px(bool t) { _16px_tiles = t; }
	inline static bool rainbow_tiles(void) { return _rainbow_tiles; }
	inline static void rainbow_tiles(bool r) { _rainbow_tiles = r; }
	inline static bool attributes(void) { return _attributes; }
	inline static void attributes(bool a) { _attributes = a; }
};

#endif
