#pragma warning(push, 0)
#include <FL/fl_utf8.h>
#include <FL/Fl_PNG_Image.H>
#pragma warning(pop)

#include "utils.h"
#include "tileset.h"
#include "tile-buttons.h"
#include "tiled-image.h"
#include "config.h"

Tiled_Image::Tiled_Image(const char *f) : _image(NULL), _inactive_image(NULL), _start_id(0x00), _num_tiles(0),
	_result(IMG_NULL) {
	if (ends_with(f, ".png") || ends_with(f, ".PNG")) { read_png_graphics(f); }
	// TODO: support BMP, GIF, 1BPP, 2BPP, 1BPP.LZ, 2BPP.LZ
}

Tiled_Image::~Tiled_Image() {
	clear();
}

void Tiled_Image::clear() {
	delete _image;
	_image = NULL;
	delete _inactive_image;
	_inactive_image = NULL;
	_start_id = 0x00;
	_num_tiles = 0;
	_result = IMG_NULL;
}

bool Tiled_Image::refresh_inactive_image() {
	if (!_image || _image->fail()) { return false; }
	Fl_RGB_Image *img = (Fl_RGB_Image *)_image->copy();
	if (!img || img->fail()) { return false; }
	img->color_average(FL_GRAY, 0.33f);
	delete _inactive_image;
	_inactive_image = img;
	return true;
}

bool Tiled_Image::draw_tile(const Tile_State *ts, int x, int y, bool active) {
	uint8_t id = ts->id - _start_id;
	Fl_RGB_Image *img = active ? _image : _inactive_image;
	if (!img || id >= _num_tiles) { return false; }
	int wt = img->w() / TILE_SIZE_2X;
	int tx = id % wt * TILE_SIZE_2X, ty = id / wt * TILE_SIZE_2X;
	img->draw(x, y, TILE_SIZE_2X, TILE_SIZE_2X, tx, ty);
	return true;
}

Tiled_Image::Result Tiled_Image::read_png_graphics(const char *f) {
	Fl_PNG_Image *png = new Fl_PNG_Image(f);
	if (!png || png->fail()) { return (_result = IMG_BAD_FILE); }

	_image = png;
	if (!Config::tiles2x()) {
		_image = (Fl_RGB_Image *)png->copy(png->w() * 2, png->h() * 2);
		delete png;
		if (!_image || _image->fail()) { return (_result = IMG_BAD_FILE); }
	}

	if (!refresh_inactive_image()) {
		clear();
		return (_result = IMG_BAD_FILE);
	}

	int w = _image->w(), h = _image->h();
	if (w % TILE_SIZE_2X || h % TILE_SIZE_2X) { return (_result = IMG_BAD_DIMS); }

	w /= TILE_SIZE_2X;
	h /= TILE_SIZE_2X;
	_num_tiles = w * h;
	_start_id = Config::start();
	if (_start_id + _num_tiles > NUM_TILES) { return (_result = IMG_TOO_LARGE); }

	return (_result = IMG_OK);
}
