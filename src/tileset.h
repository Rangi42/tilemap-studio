#ifndef TILESET_H
#define TILESET_H

#pragma warning(push, 0)
#include <FL/Fl_Image.H>
#pragma warning(pop)

#include "utils.h"
#include "tile-buttons.h"

#define TILE_SIZE 8
#define TILE_SIZE_2X (TILE_SIZE * 2)

#define NUM_HUES 4
#define BYTES_PER_1BPP_TILE (TILE_SIZE * TILE_SIZE / TILE_SIZE)
#define BYTES_PER_2BPP_TILE (BYTES_PER_1BPP_TILE * 2)

#define NUM_TILES 256
#define TILES_PER_ROW 16

struct Tile_State;

class Tileset {
public:
	enum Result { TILESET_OK, TILESET_BAD_FILE, TILESET_BAD_EXT, TILESET_BAD_DIMS,
		TILESET_TOO_SHORT, TILESET_TOO_LARGE, TILESET_BAD_CMD, TILESET_NULL };
private:
	Fl_RGB_Image *_image, *_inactive_image;
	size_t _num_tiles;
	uint8_t _start;
	Result _result;
public:
	Tileset(uint8_t start);
	~Tileset();
	inline size_t num_tiles(void) const { return _num_tiles; }
	inline uint8_t start(void) const { return _start; }
	inline Result result(void) const { return _result; }
	void clear(void);
	bool refresh_inactive_image(void);
	bool draw_tile(const Tile_State *ts, int x, int y, bool active) const;
	Result read_tiles(const char *f);
private:
	Result read_png_graphics(const char *f);
	Result read_bmp_graphics(const char *f);
	Result read_1bpp_graphics(const char *f);
	Result read_2bpp_graphics(const char *f);
	Result read_1bpp_lz_graphics(const char *f);
	Result read_2bpp_lz_graphics(const char *f);
	Result parse_1bpp_data(size_t n, uchar *data);
	Result parse_2bpp_data(size_t n, uchar *data);
	Result postprocess_graphics(Fl_RGB_Image *img);
public:
	static const char *error_message(Result result);
};

#endif
