#ifndef TILED_IMAGE_H
#define TILED_IMAGE_H

#pragma warning(push, 0)
#include <FL/Fl_Image.H>
#pragma warning(pop)

#include "tile-buttons.h"

#define TILE_SIZE 8
#define TILE_SIZE_2X (TILE_SIZE * 2)

#define BYTES_PER_1BPP_TILE (TILE_SIZE * TILE_SIZE / 8)
#define BYTES_PER_2BPP_TILE (BYTES_PER_1BPP_TILE * 2)

struct Tile_State;

class Tiled_Image {
public:
	enum Result { IMG_OK, IMG_BAD_FILE, IMG_BAD_EXT, IMG_BAD_DIMS, IMG_TOO_SHORT, IMG_TOO_LARGE, IMG_BAD_CMD, IMG_NULL };
private:
	Fl_RGB_Image *_image, *_inactive_image;
	uint8_t _start_id;
	size_t _num_tiles;
	Result _result;
public:
	Tiled_Image(const char *f);
	~Tiled_Image();
	inline size_t num_tiles(void) const { return _num_tiles; }
	inline Result result(void) const { return _result; }
	void clear(void);
	bool refresh_inactive_image(void);
	bool draw_tile(const Tile_State *ts, int x, int y, bool active);
private:
	Result read_png_graphics(const char *f);
};

#endif
