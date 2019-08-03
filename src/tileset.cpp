#include "tileset.h"

Tileset::Tileset() : _image(NULL), _result(TILESET_NULL) {}

Tileset::~Tileset() {
	clear();
}

void Tileset::clear() {
	delete _image;
	_image = NULL;
	_result = TILESET_NULL;
}

Tileset::Result Tileset::read_tiles(const char *f) {
	_image = new Tiled_Image(f);
	if (!_image) { return (_result = TILESET_NULL); }
	switch (_image->result()) {
	case Tiled_Image::IMG_OK: break;
	case Tiled_Image::IMG_NULL: return (_result = TILESET_BAD_FILE);
	case Tiled_Image::IMG_BAD_FILE: return (_result = TILESET_BAD_FILE);
	case Tiled_Image::IMG_BAD_EXT: return (_result = TILESET_BAD_EXT);
	case Tiled_Image::IMG_BAD_DIMS: return (_result = TILESET_BAD_DIMS);
	case Tiled_Image::IMG_TOO_SHORT: return (_result = TILESET_TOO_SHORT);
	case Tiled_Image::IMG_TOO_LARGE: return (_result = TILESET_TOO_LARGE);
	case Tiled_Image::IMG_BAD_CMD: return (_result = TILESET_BAD_CMD);
	default: return (_result = TILESET_BAD_FILE);
	}
	return (_result = TILESET_OK);
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
