#pragma warning(push, 0)
#include <FL/fl_utf8.h>
#include <FL/Fl_PNG_Image.H>
#pragma warning(pop)

#include "utils.h"
#include "tileset.h"
#include "tile-buttons.h"
#include "config.h"

Tileset::Tileset() : _image(NULL), _inactive_image(NULL), _start_id(0x00), _num_tiles(0), _result(TILESET_NULL) {}

Tileset::~Tileset() {
	clear();
}

void Tileset::clear() {
	delete _image;
	_image = NULL;
	delete _inactive_image;
	_inactive_image = NULL;
	_start_id = 0x00;
	_num_tiles = 0;
	_result = TILESET_NULL;
}

bool Tileset::refresh_inactive_image() {
	if (!_image || _image->fail()) { return false; }
	Fl_RGB_Image *img = (Fl_RGB_Image *)_image->copy();
	if (!img || img->fail()) { return false; }
	img->color_average(FL_GRAY, 0.33f);
	delete _inactive_image;
	_inactive_image = img;
	return true;
}

bool Tileset::draw_tile(const Tile_State *ts, int x, int y, bool active) const {
	uint8_t id = ts->id - _start_id;
	Fl_RGB_Image *img = active ? _image : _inactive_image;
	if (!img || id >= _num_tiles) { return false; }
	// TODO: support flipped graphics
	int wt = img->w() / TILE_SIZE_2X;
	int tx = id % wt * TILE_SIZE_2X, ty = id / wt * TILE_SIZE_2X;
	img->draw(x, y, TILE_SIZE_2X, TILE_SIZE_2X, tx, ty);
	return true;
}

Tileset::Result Tileset::read_tiles(const char *f) {
	if (ends_with(f, ".png") || ends_with(f, ".PNG")) { return read_png_graphics(f); }
	// TODO: support BMP, GIF, 1BPP, 2BPP, 1BPP.LZ, 2BPP.LZ
	return (_result = TILESET_BAD_EXT);
}

Tileset::Result Tileset::read_png_graphics(const char *f) {
	Fl_PNG_Image *png = new Fl_PNG_Image(f);
	if (!png || png->fail()) { return (_result = TILESET_BAD_FILE); }

	_image = png;
	if (!Config::tiles2x()) {
		_image = (Fl_RGB_Image *)png->copy(png->w() * 2, png->h() * 2);
		delete png;
		if (!_image || _image->fail()) { return (_result = TILESET_BAD_FILE); }
	}

	if (!refresh_inactive_image()) {
		clear();
		return (_result = TILESET_BAD_FILE);
	}

	int w = _image->w(), h = _image->h();
	if (w % TILE_SIZE_2X || h % TILE_SIZE_2X) { return (_result = TILESET_BAD_DIMS); }

	w /= TILE_SIZE_2X;
	h /= TILE_SIZE_2X;
	_num_tiles = w * h;
	_start_id = Config::start();
	if (_start_id + _num_tiles > NUM_TILES) { return (_result = TILESET_TOO_LARGE); }

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
