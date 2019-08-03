#include <cstdio>

#pragma warning(push, 0)
#include <FL/Fl_Image_Surface.H>
#pragma warning(pop)

#include "tilemap.h"
#include "tiled-image.h"
#include "config.h"

Tilemap::Tilemap() : _size(0), _width(0), _tiles(NULL), _result(TILEMAP_NULL),
	_modified(false), _history(MAX_HISTORY_SIZE), _future(MAX_HISTORY_SIZE) {}

Tilemap::~Tilemap() {
	clear();
}

void Tilemap::size(size_t w, size_t h) {
	clear();
	_width = w;
	_size = w * h;
	_tiles = new Tile_Tessera *[_size]();
}

void Tilemap::size(size_t n) {
	clear();
	_size = n;
	_tiles = new Tile_Tessera *[_size]();
}

void Tilemap::width(size_t w) {
	_width = w;
	for (size_t i = 0; i < _size; i++) {
		Tile_Tessera *tt = _tiles[i];
		tt->coords(i / w, i % w);
	}
}

void Tilemap::resize(size_t w, size_t h, Resize_Dialog::Hor_Align ha, Resize_Dialog::Vert_Align va) {
	int dw = w - width(), dh = h - height();

	int px, py;
	switch (ha) {
	case Resize_Dialog::Hor_Align::LEFT:
		px = 0;
		break;
	case Resize_Dialog::Hor_Align::RIGHT:
		px = dw;
		break;
	case Resize_Dialog::Hor_Align::CENTER:
	default:
		px = dw / 2;
	}
	switch (va) {
	case Resize_Dialog::Vert_Align::TOP:
		py = 0;
		break;
	case Resize_Dialog::Vert_Align::BOTTOM:
		py = dh;
		break;
	case Resize_Dialog::Vert_Align::MIDDLE:
	default:
		py = dh / 2;
	}

	size_t n = w * h;
	Tile_Tessera **tiles = new Tile_Tessera *[n]();
	int mx = MAX(px, 0), my = MAX(py, 0), mw = MIN(w, width() + px), mh = MIN(h, height() + py);
	size_t i = 0, j = 0;
	uint8_t t = Config::start();
	for (int y = 0; y < py; y++) {
		for (int x = 0; x < (int)w; x++) {
			tiles[i++] = new Tile_Tessera(0, 0, 0, 0, t);
		}
	}
	for (int y = my; y < mh; y++) {
		for (int x = 0; x < px; x++) {
			tiles[i++] = new Tile_Tessera(0, 0, 0, 0, t);
		}
		for (int x = mx; x < mw; x++) {
			tiles[i++] = j < _size ? (j++, tile(x - px, y - py)) :
				new Tile_Tessera(0, 0, 0, 0, t);
		}
		for (int x = mw; x < (int)w; x++) {
			tiles[i++] = new Tile_Tessera(0, 0, 0, 0, t);
		}
	}
	for (int y = mh; y < (int)h; y++) {
		for (int x = 0; x < (int)w; x++) {
			tiles[i++] = new Tile_Tessera(0, 0, 0, 0, t);
		}
	}

	clear();
	_tiles = tiles;
	_size = n;
	width(w);
	_modified = true;
}

void Tilemap::clear() {
	delete [] _tiles;
	_tiles = NULL;
	_size = _width = 0;
	_result = TILEMAP_NULL;
	_modified = false;
	_history.clear();
	_future.clear();
}

void Tilemap::reposition_tiles(int x, int y) {
	for (size_t i = 0; i < _size; i++) {
		Tile_Tessera *tt = _tiles[i];
		int tx = x + (int)tt->col() * TILE_SIZE_2X, ty = y + (int)tt->row() * TILE_SIZE_2X;
		tt->position(tx, ty);
	}
}

void Tilemap::remember() {
	_future.clear();
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	Tilemap_State ts(_size);
	for (size_t i = 0; i < _size; i++) {
		ts.states[i] = tile(i)->state();
	}
	_history.push_back(ts);
}

void Tilemap::undo() {
	if (_history.empty()) { return; }
	while (_future.size() >= MAX_HISTORY_SIZE) { _future.pop_front(); }

	Tilemap_State ts(_size);
	for (size_t i = 0; i < _size; i++) {
		ts.states[i] = tile(i)->state();
	}
	_future.push_back(ts);

	const Tilemap_State &prev = _history.back();
	for (size_t i = 0; i < _size; i++) {
		tile(i)->state(prev.states[i]);
	}
	_history.pop_back();
}

void Tilemap::redo() {
	if (_future.empty()) { return; }
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	Tilemap_State ts(_size);
	for (size_t i = 0; i < _size; i++) {
		ts.states[i] = tile(i)->state();
	}
	_history.push_back(ts);

	const Tilemap_State &next = _future.back();
	for (size_t i = 0; i < _size; i++) {
		tile(i)->state(next.states[i]);
	}
	_future.pop_back();
}

void Tilemap::new_tiles(size_t w, size_t h) {
	size(w, h);
	for (size_t i = 0; i < _size; i++) {
		Tile_Tessera *tt = new Tile_Tessera(0, 0, 0, 0, Config::start());
		_tiles[i] = tt;
	}
	_modified = true;
}

Tilemap::Result Tilemap::read_tiles(const char *f) {
	FILE *file = fl_fopen(f, "rb");
	if (file == NULL) { return (_result = TILEMAP_BAD_FILE); } // cannot load file

	fseek(file, 0, SEEK_END);
	long c = ftell(file);
	rewind(file);
	if (c < 0) { fclose(file); return (_result = TILEMAP_BAD_FILE); } // cannot read file
	if (c == 0) { fclose(file); return (_result = TILEMAP_EMPTY); } // no content

	Tile_Tessera **tiles = new Tile_Tessera *[UINT8_MAX * UINT8_MAX]();
	size_t n = 0;
	int fmt = Config::format();

	if (fmt == Format::PLAIN) {
		if (c > UINT8_MAX * UINT8_MAX) {
			fclose(file);
			return (_result = TILEMAP_TOO_LARGE);
		}
		n = (size_t)c;
		for (size_t i = 0; i < n; i++) {
			int b = fgetc(file);
			Tile_Tessera *tt = new Tile_Tessera(0, 0, 0, 0, (uint8_t)b);
			tiles[i] = tt;
		}
	}

	else if (fmt == Format::RLE) {
		for (long s = 0; s < c; s += 2) {
			int v = fgetc(file);
			int r = fgetc(file);
			if (r == EOF) {
				fclose(file);
				for (size_t i = 0; i < n; i++) {
					delete tiles[i];
				}
				delete [] tiles;
				return (_result = TILEMAP_TOO_SHORT_RLE);
			}
			for (int i = 0; i < r; i++) {
				if (n >= UINT8_MAX * UINT8_MAX) {
					fclose(file);
					for (size_t j = 0; j < n; j++) {
						delete tiles[j];
					}
					delete [] tiles;
					return (_result = TILEMAP_TOO_LARGE);
				}
				Tile_Tessera *tt = new Tile_Tessera(0, 0, 0, 0, (uint8_t)v);
				tiles[n++] = tt;
			}
		}
	}

	else if (fmt == Format::FF_END) {
		if (c > UINT8_MAX * UINT8_MAX + 1) {
			fclose(file);
			return (_result = TILEMAP_TOO_LARGE);
		}
		n = (size_t)c - 1;
		for (size_t i = 0; i < n; i++) {
			int b = fgetc(file);
			if (b == 0xFF) {
				fclose(file);
				for (size_t j = 0; j < i; j++) {
					delete tiles[j];
				}
				delete [] tiles;
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			Tile_Tessera *tt = new Tile_Tessera(0, 0, 0, 0, (uint8_t)b);
			tiles[i] = tt;
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (size_t i = 0; i < n; i++) {
				delete tiles[i];
			}
			delete [] tiles;
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	else if (fmt == Format::RLE_FF_END) {
		for (long s = 0; s < c - 1; s += 2) {
			int v = fgetc(file);
			if (v == 0xFF) {
				fclose(file);
				for (size_t i = 0; i < n; i++) {
					delete tiles[i];
				}
				delete [] tiles;
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			int r = fgetc(file);
			if (r == 0xFF) {
				fclose(file);
				for (size_t i = 0; i < n; i++) {
					delete tiles[i];
				}
				delete [] tiles;
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			for (int i = 0; i < r; i++) {
				if (n >= UINT8_MAX * UINT8_MAX) {
					fclose(file);
					for (size_t j = 0; j < n; j++) {
						delete tiles[j];
					}
					delete [] tiles;
					return (_result = TILEMAP_TOO_LARGE);
				}
				Tile_Tessera *tt = new Tile_Tessera(0, 0, 0, 0, (uint8_t)v);
				tiles[n++] = tt;
			}
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (size_t i = 0; i < n; i++) {
				delete tiles[i];
			}
			delete [] tiles;
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	else if (fmt == Format::RLE_NYBBLES) {
		for (long s = 0; s < c - 1; s++) {
			int b = fgetc(file);
			if (b == 0x00) {
				fclose(file);
				for (size_t i = 0; i < n; i++) {
					delete tiles[i];
				}
				delete [] tiles;
				return (_result = TILEMAP_TOO_LONG_00);
			}
			int v = (b & 0xF0) >> 4;
			int r = b & 0x0F;
			for (int i = 0; i < r; i++) {
				if (n >= UINT8_MAX * UINT8_MAX) {
					fclose(file);
					for (size_t j = 0; j < n; j++) {
						delete tiles[j];
					}
					delete [] tiles;
					return (_result = TILEMAP_TOO_LARGE);
				}
				Tile_Tessera *tt = new Tile_Tessera(0, 0, 0, 0, (uint8_t)v);
				tiles[n++] = tt;
			}
		}
		int b = fgetc(file);
		if (b != 0x00) {
			fclose(file);
			for (size_t i = 0; i < n; i++) {
				delete tiles[i];
			}
			delete [] tiles;
			return (_result = TILEMAP_TOO_SHORT_00);
		}
	}

	else if (fmt == Format::XY_FLIP) {
		if (c > UINT8_MAX * UINT8_MAX + 1) {
			fclose(file);
			return (_result = TILEMAP_TOO_LARGE);
		}
		n = (size_t)c - 1;
		for (size_t i = 0; i < n; i++) {
			int b = fgetc(file);
			if (b == 0xFF) {
				fclose(file);
				for (size_t j = 0; j < i; j++) {
					delete tiles[j];
				}
				delete [] tiles;
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			bool x_flip = !!(b & 0x40), y_flip = !!(b & 0x80);
			int v = b & 0x3F;
			Tile_Tessera *tt = new Tile_Tessera(0, 0, 0, 0, (uint8_t)v, x_flip, y_flip);
			tiles[i] = tt;
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (size_t i = 0; i < n; i++) {
				delete tiles[i];
			}
			delete [] tiles;
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	fclose(file);

	size(n);
	for (size_t i = 0; i < _size; i++) {
		_tiles[i] = tiles[i];
	}
	delete [] tiles;
	if (_size == 0) { return (_result = TILEMAP_EMPTY); }

	guess_width();

	return (_result = TILEMAP_OK);
}

bool Tilemap::can_write_tiles() {
	Format fmt = Config::format();
	int n = format_tileset_size(fmt);
	for (size_t i = 0; i < _size; i++) {
		Tile_Tessera *tt = _tiles[i];
		if (tt->id() >= n) {
			return false;
		}
		if ((tt->x_flip() || tt->y_flip()) && fmt != Format::XY_FLIP) {
			return false;
		}
	}
	return true;
}

bool Tilemap::write_tiles(const char *f) {
	FILE *file = fl_fopen(f, "wb");
	if (!file) { return false; }
	int fmt = Config::format();

	if (fmt == Format::PLAIN || fmt == Format::FF_END || fmt == Format::XY_FLIP) {
		for (size_t i = 0; i < _size; i++) {
			Tile_Tessera *tt = _tiles[i];
			uint8_t v = tt->id();
			if (tt->x_flip()) { v |= 0x40; }
			if (tt->y_flip()) { v |= 0x80; }
			fputc(v, file);
		}
	}
	else if (fmt == Format::RLE || fmt == Format::RLE_FF_END) {
		for (size_t i = 0; i < _size;) {
			Tile_Tessera *tt = _tiles[i++];
			uint8_t v = tt->id();
			uint8_t r = 1;
			while (i < _size && _tiles[i]->id() == v) {
				r++;
				i++;
				if (r == 0xFF) { break; }
			}
			fputc(v, file);
			fputc(r, file);
		}
	}
	else if (fmt == Format::RLE_NYBBLES) {
		for (size_t i = 0; i < _size;) {
			Tile_Tessera *tt = _tiles[i++];
			uint8_t v = tt->id();
			uint8_t r = 1;
			while (i < _size && _tiles[i]->id() == v) {
				r++;
				i++;
				if (r == 0x0F) { break; }
			}
			uint8_t b = (v << 4) | r;
			fputc(b, file);
		}
	}

	if (fmt == Format::RLE_NYBBLES) {
		fputc(0x00, file);
	}
	else if (fmt == Format::FF_END || fmt == Format::RLE_FF_END || fmt == Format::XY_FLIP) {
		fputc(0xFF, file);
	}

	fclose(file);
	return true;
}

Fl_RGB_Image *Tilemap::print_tilemap() const {
	Fl_Image_Surface *surface = new Fl_Image_Surface((int)width() * TILE_SIZE_2X, (int)height() * TILE_SIZE_2X);
	surface->set_current();
	for (size_t i = 0; i < _size; i++) {
		Tile_Tessera *tt = _tiles[i];
		int dx = (int)tt->col() * TILE_SIZE_2X, dy = (int)tt->row() * TILE_SIZE_2X;
		surface->draw(tt, dx, dy);
	}
	Fl_RGB_Image *img = surface->image();
	delete surface;
	Fl_Display_Device::display_device()->set_current();
	return img;
}

void Tilemap::guess_width() {
	if (_size % GAME_BOY_WIDTH == 0) {
		_width = GAME_BOY_WIDTH;
	}
	else if (_size % GAME_BOY_HEIGHT == 0 && _size / GAME_BOY_HEIGHT <= GAME_BOY_WIDTH) {
		_width = _size / GAME_BOY_HEIGHT;
	}
	else if (_size % 32 == 0) {
		// Game Boy VRAM width
		_width = 32;
	}
	else if (_size % (GAME_BOY_HEIGHT - 6) == 0 && _size / (GAME_BOY_HEIGHT - 6) <= GAME_BOY_WIDTH) {
		// Game Boy screen height minus textbox height
		_width = _size / (GAME_BOY_HEIGHT - 6);
	}
	else {
		_width = 16;
	}
}

int Tilemap::format_tileset_size(Format fmt) {
	switch (fmt) {
	case Format::RLE_NYBBLES:
		// High nybble is reserved for run length
		return 0x10;
	case Format::XY_FLIP:
		// High two bits are reserved for X/Y flip
		return 0x40;
	case Format::FF_END:
	case Format::RLE_FF_END:
		// $FF is reserved for the end marker
		return 0xFF;
	default:
		return 0x100;
	}
}

const char *Tilemap::format_name(Format fmt) {
	switch (fmt) {
	case Tilemap::Format::PLAIN:
		return "Plain";
	case Tilemap::Format::RLE:
		return "Run-length encoded";
	case Tilemap::Format::FF_END:
		return "GSC Town Map";
	case Tilemap::Format::RLE_FF_END:
		return "Pok\xc3\xa9gear card";
	case Tilemap::Format::RLE_NYBBLES:
		return "RBY Town Map";
	case Tilemap::Format::XY_FLIP:
		return "PC Town Map";
	default:
		return "Any";
	}
}

const char *Tilemap::format_extension(Format fmt) {
	switch (fmt) {
	case Tilemap::Format::PLAIN:
		return ".tilemap"; // e.g. pokecrystal/gfx/card_flip/card_flip.tilemap
	case Tilemap::Format::RLE:
	case Tilemap::Format::RLE_NYBBLES:
		return ".rle"; // e.g. pokered/gfx/town_map.rle
	case Tilemap::Format::FF_END:
	case Tilemap::Format::XY_FLIP:
		return ".bin"; // e.g. pokecrystal/gfx/pokegear/*.bin, polishedcrystal/gfx/town_map/*.bin
	case Tilemap::Format::RLE_FF_END:
		return ".tilemap.rle"; // e.g. pokecrystal/gfx/pokegear/*.tilemap.bin
	default:
		return ".map"; // e.g. GSC Town Map Editor's *.map
	}
}

const char *Tilemap::error_message(Result result) {
	switch (result) {
	case TILEMAP_OK:
		return "OK.";
	case TILEMAP_BAD_FILE:
		return "Cannot open file.";
	case TILEMAP_EMPTY:
		return "Tilemap is empty.";
	case TILEMAP_TOO_LARGE:
		return "Tilemap is larger than 255x255.";
	case TILEMAP_TOO_SHORT_FF:
		return "File ends before any $FF.";
	case TILEMAP_TOO_LONG_FF:
		return "File continues after $FF.";
	case TILEMAP_TOO_SHORT_00:
		return "File ends before any $00.";
	case TILEMAP_TOO_LONG_00:
		return "File continues after $00.";
	case TILEMAP_TOO_SHORT_RLE:
		return "File ends before RLE value.";
	case TILEMAP_NULL:
		return "No file chosen.";
	default:
		return "Unspecified error.";
	}
}
