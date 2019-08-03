#ifndef TILESET_H
#define TILESET_H

#include "utils.h"

#define NUM_TILES 256
#define TILES_PER_ROW 16
#define TILES_PER_COL (MAX_NUM_TILES / TILES_PER_ROW)

class Tileset {
public:
	enum Result { TILESET_OK, TILESET_BAD_FILE, TILESET_BAD_EXT, TILESET_BAD_DIMS,
		TILESET_TOO_SHORT, TILESET_TOO_LARGE, TILESET_BAD_CMD, TILESET_NULL };
private:
	size_t _num_tiles;
	uint8_t _first_tile;
	Result _result;
public:
	Tileset();
	~Tileset();
	inline size_t num_tiles(void) const { return _num_tiles; }
	inline uint8_t first_tile(void) const { return _first_tile; }
	inline Result result(void) const { return _result; }
public:
	void clear(void);
	Result read_tiles(const char *f);
public:
	static const char *error_message(Result result);
};

#endif
