#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include "tilemap-format.h"

class Config {
private:
	static Tilemap_Format _format;
	static bool _grid, _rainbow_tiles, _bold_palettes;
	static uint16_t _highlight_id;
	static bool _show_attributes;
public:
	inline static Tilemap_Format format(void) { return _format; }
	inline static void format(Tilemap_Format fmt) { _format = fmt; }
	inline static bool grid(void) { return _grid; }
	inline static void grid(bool g) { _grid = g; }
	inline static bool rainbow_tiles(void) { return _rainbow_tiles; }
	inline static void rainbow_tiles(bool r) { _rainbow_tiles = r; }
	inline static bool bold_palettes(void) { return _bold_palettes; }
	inline static void bold_palettes(bool b) { _bold_palettes = b; }
	inline static uint16_t highlight_id(void) { return _highlight_id; }
	inline static void highlight_id(uint16_t id) { _highlight_id = id; }
	inline static bool show_attributes(void) { return _show_attributes; }
	inline static void show_attributes(bool a) { _show_attributes = a; }
};

#endif
