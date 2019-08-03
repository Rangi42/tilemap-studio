#ifndef TILESET_H
#define TILESET_H

#include "utils.h"
#include "tiled-image.h"
#include "tile-buttons.h"

#define NUM_TILES 256
#define TILES_PER_ROW 16
#define TILES_PER_COL (MAX_NUM_TILES / TILES_PER_ROW)

struct Tile_State;

class Tileset {
public:
	enum Result { TILESET_OK, TILESET_BAD_FILE, TILESET_BAD_EXT, TILESET_BAD_DIMS,
		TILESET_TOO_SHORT, TILESET_TOO_LARGE, TILESET_BAD_CMD, TILESET_NULL };
private:
	Tiled_Image *_image;
	Result _result;
public:
	Tileset();
	~Tileset();
	inline Tiled_Image *image(void) const { return _image; }
	inline size_t num_tiles(void) const { return _image ? _image->num_tiles() : 0; }
	inline Result result(void) const { return _result; }
public:
	void clear(void);
	inline bool refresh_inactive_image(void) { return _image ? _image->refresh_inactive_image() : false; }
	inline bool draw_tile(const Tile_State *ts, int x, int y, bool active) const {
		return _image && _image->draw_tile(ts, x, y, active);
	}
	Result read_tiles(const char *f);
public:
	static const char *error_message(Result result);
};

#endif
