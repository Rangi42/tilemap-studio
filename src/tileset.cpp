#include "tileset.h"

Tileset::Tileset() : _num_tiles(0), _result(TILESET_NULL) {}

Tileset::~Tileset() {
	clear();
}

void Tileset::clear() {
	_num_tiles = 0;
	_result = TILESET_NULL;
}

Tileset::Result Tileset::read_tiles(const char *f) {
	// TODO: read_tiles
	return (_result = TILESET_NULL);
}

const char *Tileset::error_message(Result result) {
	switch (result) {
	case TILESET_OK:
		return "OK.";
	case TILESET_BAD_FILE:
		return "Cannot parse file format.";
	case TILESET_BAD_EXT:
		return "Unknown file extension.";
	case TILESET_BAD_DIMS:
		return "Image dimensions do not fit the tile grid.";
	case TILESET_TOO_SHORT:
		return "Too few bytes.";
	case TILESET_TOO_LARGE:
		return "Too many pixels.";
	case TILESET_BAD_CMD:
		return "Invalid LZ command.";
	case TILESET_NULL:
		return "No graphics file chosen.";
	default:
		return "Unspecified error.";
	}
}
