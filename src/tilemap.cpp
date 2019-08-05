#include <cstdio>

#pragma warning(push, 0)
#include <FL/Fl_Image_Surface.H>
#pragma warning(pop)

#include "tilemap.h"
#include "tileset.h"
#include "config.h"

Tilemap::Tilemap() : _tiles(), _width(0), _result(TILEMAP_NULL), _modified(false), _history(), _future() {}

Tilemap::~Tilemap() {
	clear();
}

void Tilemap::width(size_t w) {
	_width = w;
	size_t n = size();
	for (size_t i = 0; i < n; i++) {
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
	std::vector<Tile_Tessera *> tiles;
	tiles.reserve(n);
	int mx = MAX(px, 0), my = MAX(py, 0), mw = MIN(w, width() + px), mh = MIN(h, height() + py);
	size_t i = 0;
	for (int y = 0; y < py; y++) {
		for (int x = 0; x < (int)w; x++) {
			tiles.emplace_back(new Tile_Tessera(0, 0));
		}
	}
	for (int y = my; y < mh; y++) {
		for (int x = 0; x < px; x++) {
			tiles.emplace_back(new Tile_Tessera(0, 0));
		}
		for (int x = mx; x < mw; x++) {
			tiles.emplace_back(i < size() ? (i++, tile(x - px, y - py)) : new Tile_Tessera(0, 0));
		}
		for (int x = mw; x < (int)w; x++) {
			tiles.emplace_back(new Tile_Tessera(0, 0));
		}
	}
	for (int y = mh; y < (int)h; y++) {
		for (int x = 0; x < (int)w; x++) {
			tiles.emplace_back(new Tile_Tessera(0, 0));
		}
	}

	clear();
	_tiles.swap(tiles);
	width(w);
	_modified = true;
}

void Tilemap::clear() {
	_tiles.clear();
	_width = 0;
	_result = TILEMAP_NULL;
	_modified = false;
	_history.clear();
	_future.clear();
}

void Tilemap::reposition_tiles(int x, int y) {
	for (Tile_Tessera *tt : _tiles) {
		int tx = x + (int)tt->col() * TILE_SIZE_2X, ty = y + (int)tt->row() * TILE_SIZE_2X;
		tt->position(tx, ty);
	}
}

void Tilemap::remember() {
	_future.clear();
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	size_t n = size();
	Tilemap_State ts(n);
	for (size_t i = 0; i < n; i++) {
		ts.states[i] = _tiles[i]->state();
	}
	_history.push_back(ts);
}

void Tilemap::undo() {
	if (_history.empty()) { return; }
	while (_future.size() >= MAX_HISTORY_SIZE) { _future.pop_front(); }

	size_t n = size();
	Tilemap_State ts(n);
	for (size_t i = 0; i < n; i++) {
		ts.states[i] = _tiles[i]->state();
	}
	_future.push_back(ts);

	const Tilemap_State &prev = _history.back();
	for (size_t i = 0; i < n; i++) {
		_tiles[i]->state(prev.states[i]);
	}
	_history.pop_back();
}

void Tilemap::redo() {
	if (_future.empty()) { return; }
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	size_t n = size();
	Tilemap_State ts(n);
	for (size_t i = 0; i < n; i++) {
		ts.states[i] = _tiles[i]->state();
	}
	_history.push_back(ts);

	const Tilemap_State &next = _future.back();
	for (size_t i = 0; i < n; i++) {
		_tiles[i]->state(next.states[i]);
	}
	_future.pop_back();
}

void Tilemap::new_tiles(size_t w, size_t h) {
	clear();
	size_t n = w * h;
	_tiles.reserve(n);
	for (size_t i = 0; i < n; i++) {
		_tiles.emplace_back(new Tile_Tessera(0, 0));
	}
	_width = w;
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

	std::vector<Tile_Tessera *> tiles;
	int fmt = Config::format();

	if (fmt == Format::PLAIN) {
		for (long i = 0; i < c; i++) {
			int b = fgetc(file);
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint8_t)b));
		}
	}

	else if (fmt == Format::RLE) {
		if (c % 2) {
			fclose(file);
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
		for (long i = 0; i < c; i += 2) {
			int v = fgetc(file);
			int r = fgetc(file);
			if (r == EOF) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_SHORT_RLE);
			}
			for (int j = 0; j < r; j++) {
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint8_t)v));
			}
		}
	}

	else if (fmt == Format::FF_END) {
		for (long i = 0; i < c - 1; i++) {
			int b = fgetc(file);
			if (b == 0xFF) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint8_t)b));
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	else if (fmt == Format::RLE_FF_END) {
		if (!(c % 2)) {
			fclose(file);
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
		for (long i = 0; i < c - 1; i += 2) {
			int v = fgetc(file);
			if (v == 0xFF) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			int r = fgetc(file);
			if (r == 0xFF) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			for (int j = 0; j < r; j++) {
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint8_t)v));
			}
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	else if (fmt == Format::RLE_NYBBLES) {
		for (long i = 0; i < c - 1; i++) {
			int b = fgetc(file);
			if (b == 0x00) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_00);
			}
			int v = (b & 0xF0) >> 4, r = b & 0x0F;
			for (int j = 0; j < r; j++) {
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint8_t)v));
			}
		}
		int b = fgetc(file);
		if (b != 0x00) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_00);
		}
	}

	else if (fmt == Format::XY_FLIP) {
		for (long i = 0; i < c - 1; i++) {
			int b = fgetc(file);
			if (b == 0xFF) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			bool x_flip = !!(b & 0x40), y_flip = !!(b & 0x80);
			int v = b & 0x3F;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint8_t)v, x_flip, y_flip));
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	else if (fmt == Format::TILE_ATTR) {
		if (c % 2) {
			fclose(file);
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
		for (long i = 0; i < c; i += 2) {
			int v = fgetc(file);
			int a = fgetc(file);
			bool x_flip = !!(a & 0x40), y_flip = !!(a & 0x80);
			int color = (a & 0xC) >> 2;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint8_t)v, x_flip, y_flip, color));
		}
	}

	fclose(file);

	_tiles.swap(tiles);
	if (size() == 0) { return (_result = TILEMAP_EMPTY); }

	guess_width();

	return (_result = TILEMAP_OK);
}

bool Tilemap::can_write_tiles() {
	Format fmt = Config::format();
	int m = format_tileset_size(fmt);
	for (Tile_Tessera *tt : _tiles) {
		if (tt->id() >= m) {
			return false;
		}
		if ((tt->x_flip() || tt->y_flip()) && fmt != Format::XY_FLIP && fmt != Format::TILE_ATTR) {
			return false;
		}
		if (tt->sgb_color() > -1 && fmt != Format::TILE_ATTR) {
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
		for (Tile_Tessera *tt : _tiles) {
			uint8_t v = tt->id();
			if (tt->x_flip()) { v |= 0x40; }
			if (tt->y_flip()) { v |= 0x80; }
			fputc(v, file);
		}
	}
	else if (fmt == Format::RLE || fmt == Format::RLE_FF_END) {
		size_t n = size();
		for (size_t i = 0; i < n;) {
			Tile_Tessera *tt = _tiles[i++];
			uint8_t v = tt->id(), r = 1;
			while (i < n && _tiles[i++]->id() == v) {
				if (++r == 0xFF) { break; } // maximum byte
			}
			fputc(v, file);
			fputc(r, file);
		}
	}
	else if (fmt == Format::RLE_NYBBLES) {
		size_t n = size();
		for (size_t i = 0; i < n;) {
			Tile_Tessera *tt = _tiles[i++];
			uint8_t v = tt->id(), r = 1;
			while (i < n && _tiles[i++]->id() == v) {
				if (++r == 0x0F) { break; } // maximum nybble
			}
			uint8_t b = (v << 4) | r;
			fputc(b, file);
		}
	}
	else if (fmt == Format::TILE_ATTR) {
		for (Tile_Tessera *tt : _tiles) {
			uint8_t v = tt->id();
			fputc(v, file);
			uint8_t a = 0x10;
			if (tt->x_flip()) { v |= 0x40; }
			if (tt->y_flip()) { v |= 0x80; }
			if (tt->sgb_color() > -1) { v |= tt->sgb_color() << 2; }
			fputc(a, file);
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
	for (Tile_Tessera *tt : _tiles) {
		int dx = (int)tt->col() * TILE_SIZE_2X, dy = (int)tt->row() * TILE_SIZE_2X;
		surface->draw(tt, dx, dy);
	}
	Fl_RGB_Image *img = surface->image();
	delete surface;
	Fl_Display_Device::display_device()->set_current();
	return img;
}

void Tilemap::guess_width() {
	size_t n = size();
	if (n % GAME_BOY_WIDTH == 0 && n / GAME_BOY_WIDTH <= GAME_BOY_HEIGHT) {
		_width = GAME_BOY_WIDTH;
	}
	else if (n % GAME_BOY_HEIGHT == 0 && n / GAME_BOY_HEIGHT <= GAME_BOY_WIDTH) {
		_width = n / GAME_BOY_HEIGHT;
	}
	else if (n % 32 == 0) {
		// Game Boy VRAM width
		_width = 32;
	}
	else if (n % (GAME_BOY_HEIGHT - 6) == 0 && n / (GAME_BOY_HEIGHT - 6) <= GAME_BOY_WIDTH) {
		// Game Boy screen height minus textbox height
		_width = n / (GAME_BOY_HEIGHT - 6);
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
	case Format::PLAIN:
		return "Plain";
	case Format::RLE:
		return "Run-length encoded";
	case Format::FF_END:
		return "GSC Town Map";
	case Format::RLE_FF_END:
		return "Pok\xc3\xa9gear card";
	case Format::RLE_NYBBLES:
		return "RBY Town Map";
	case Format::XY_FLIP:
		return "PC Town Map";
	case Format::TILE_ATTR:
		return "SGB border";
	default:
		return "Any";
	}
}

const char *Tilemap::format_extension(Format fmt) {
	switch (fmt) {
	case Format::PLAIN:
	default:
		return ".tilemap"; // e.g. pokecrystal/gfx/card_flip/card_flip.tilemap
	case Format::RLE:
	case Format::RLE_NYBBLES:
		return ".rle"; // e.g. pokered/gfx/town_map.rle
	case Format::FF_END:
	case Format::XY_FLIP:
		return ".bin"; // e.g. pokecrystal/gfx/pokegear/*.bin, polishedcrystal/gfx/town_map/*.bin
	case Format::RLE_FF_END:
		return ".tilemap.rle"; // e.g. pokecrystal/gfx/pokegear/*.tilemap.bin
	case Format::TILE_ATTR:
		return ".map"; // e.g. pokered/gfx/{red|blue}/sgbborder.map
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
