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
			tiles.emplace_back(new Tile_Tessera());
		}
	}
	for (int y = my; y < mh; y++) {
		for (int x = 0; x < px; x++) {
			tiles.emplace_back(new Tile_Tessera());
		}
		for (int x = mx; x < mw; x++) {
			if (i < size()) {
				i++;
				tiles.emplace_back(tile(x - px, y - py));
			}
			else {
				tiles.emplace_back(new Tile_Tessera());
			}
		}
		for (int x = mw; x < (int)w; x++) {
			tiles.emplace_back(new Tile_Tessera());
		}
	}
	for (int y = mh; y < (int)h; y++) {
		for (int x = 0; x < (int)w; x++) {
			tiles.emplace_back(new Tile_Tessera());
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
	int s = TILE_SIZE * Config::zoom();
	for (Tile_Tessera *tt : _tiles) {
		int tx = x + (int)tt->col() * s, ty = y + (int)tt->row() * s;
		tt->resize(tx, ty, s, s);
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
		_tiles.emplace_back(new Tile_Tessera());
	}
	if (format_has_palettes(Config::format())) {
		for (size_t i = 0; i < n; i++) {
			_tiles[i]->palette(0);
		}
	}
	_width = w;
	_modified = true;
}

Tilemap::Result Tilemap::read_tiles(const char *tf, const char *af) {
	FILE *file = fl_fopen(tf, "rb");
	if (file == NULL) { return (_result = TILEMAP_BAD_FILE); }

	fseek(file, 0, SEEK_END);
	long c = ftell(file);
	rewind(file);
	if (c < 0) { fclose(file); return (_result = TILEMAP_BAD_FILE); }
	if (c == 0) { fclose(file); return (_result = TILEMAP_EMPTY); }

	std::vector<Tile_Tessera *> tiles;
	int fmt = Config::format();

	if (fmt == Tilemap_Format::PLAIN) {
		for (long i = 0; i < c; i++) {
			int b = fgetc(file);
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)b));
		}
	}

	else if (fmt == Tilemap_Format::GBC_ATTRS) {
		if (c % 2) {
			fclose(file);
			return (_result = TILEMAP_TOO_SHORT_ATTRS);
		}
		for (long i = 0; i < c; i += 2) {
			int v = fgetc(file);
			int a = fgetc(file);
			if (!!(a & 0x08)) { v |= 0x100; }
			bool x_flip = !!(a & 0x20), y_flip = !!(a & 0x40), priority = !!(a & 0x80), obp1 = !!(a & 0x10);
			int palette = a & 0x07;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v, x_flip, y_flip, priority, obp1, palette));
		}
	}

	else if (fmt == Tilemap_Format::GBC_ATTRMAP) {
		FILE *attr_file = fl_fopen(af, "rb");
		if (attr_file == NULL) {
			fclose(file);
			return (_result = ATTRMAP_BAD_FILE);
		}

		fseek(attr_file, 0, SEEK_END);
		long ac = ftell(attr_file);
		rewind(attr_file);
		if (ac != c) {
			fclose(attr_file);
			fclose(file);
			return (_result = ac < 0 ? ATTRMAP_BAD_FILE : ac < c ? ATTRMAP_TOO_SHORT : ATTRMAP_TOO_LONG);
		}

		for (long i = 0; i < c; i++) {
			int v = fgetc(file);
			int a = fgetc(attr_file);
			if (!!(a & 0x08)) { v |= 0x100; }
			bool x_flip = !!(a & 0x20), y_flip = !!(a & 0x40), priority = !!(a & 0x80), obp1 = !!(a & 0x10);
			int palette = a & 0x07;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v, x_flip, y_flip, priority, obp1, palette));
		}

		fclose(attr_file);
	}

	else if (fmt == Tilemap_Format::GBA_PALETTES) {
		if (c % 2) {
			fclose(file);
			return (_result = TILEMAP_TOO_SHORT_ATTRS);
		}
		for (long i = 0; i < c; i += 2) {
			int v = fgetc(file);
			int a = fgetc(file);
			v = v | ((a & 0x03) << 8);
			bool x_flip = !!(a & 0x04), y_flip = !!(a & 0x08);
			int palette = (a & 0xF0) >> 4;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v, x_flip, y_flip, false, false, palette));
		}
	}

	else if (fmt == Tilemap_Format::SGB_BORDER) {
		if (c % 2) {
			fclose(file);
			return (_result = TILEMAP_TOO_SHORT_ATTRS);
		}
		for (long i = 0; i < c; i += 2) {
			int v = fgetc(file);
			int a = fgetc(file);
			bool x_flip = !!(a & 0x40), y_flip = !!(a & 0x80);
			int palette = (a & 0x0C) >> 2;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v, x_flip, y_flip, false, false, palette));
		}
	}

	else if (fmt == Tilemap_Format::SNES_ATTRS) {
		if (c % 2) {
			fclose(file);
			return (_result = TILEMAP_TOO_SHORT_ATTRS);
		}
		for (long i = 0; i < c; i += 2) {
			int v = fgetc(file);
			int a = fgetc(file);
			v = v | ((a & 0x03) << 8);
			bool x_flip = !!(a & 0x40), y_flip = !!(a & 0x80), priority = !!(a & 0x20);
			int palette = (a & 0x1C) >> 2;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v, x_flip, y_flip, priority, false, palette));
		}
	}

	else if (fmt == Tilemap_Format::RBY_TOWN_MAP) {
		for (long i = 0; i < c - 1; i++) {
			int b = fgetc(file);
			if (b == 0x00) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_00);
			}
			int v = (b & 0xF0) >> 4, r = b & 0x0F;
			for (int j = 0; j < r; j++) {
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v));
			}
		}
		int b = fgetc(file);
		if (b != 0x00) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_00);
		}
	}

	else if (fmt == Tilemap_Format::GSC_TOWN_MAP) {
		for (long i = 0; i < c - 1; i++) {
			int b = fgetc(file);
			if (b == 0xFF) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)b));
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	else if (fmt == Tilemap_Format::PC_TOWN_MAP) {
		for (long i = 0; i < c - 1; i++) {
			int b = fgetc(file);
			if (b == 0xFF) {
				fclose(file);
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = TILEMAP_TOO_LONG_FF);
			}
			bool x_flip = !!(b & 0x40), y_flip = !!(b & 0x80);
			int v = b & 0x3F;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v, x_flip, y_flip));
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	else if (fmt == Tilemap_Format::POKEGEAR_CARD) {
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
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, (uint16_t)v));
			}
		}
		int b = fgetc(file);
		if (b != 0xFF) {
			fclose(file);
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = TILEMAP_TOO_SHORT_FF);
		}
	}

	fclose(file);

	_tiles.swap(tiles);
	if (size() == 0) { return (_result = TILEMAP_EMPTY); }

	guess_width();

	return (_result = TILEMAP_OK);
}

bool Tilemap::can_format_as(Tilemap_Format fmt) {
	int n = format_tileset_size(fmt), m = format_palettes_size(fmt);
	bool can_flip = format_can_flip(fmt), has_priority = format_has_priority(fmt), has_obp1 = format_has_obp1(fmt);
	for (Tile_Tessera *tt : _tiles) {
		if (tt->id() >= n) {
			return false;
		}
		if (tt->palette() >= m) {
			return false;
		}
		if ((tt->x_flip() || tt->y_flip()) && !can_flip) {
			return false;
		}
		if (tt->priority() && !has_priority) {
			return false;
		}
		if (tt->obp1() && !has_obp1) {
			return false;
		}
	}
	return true;
}

void Tilemap::limit_to_format(Tilemap_Format fmt) {
	int n = format_tileset_size(fmt), m = format_palettes_size(fmt);
	bool can_flip = format_can_flip(fmt), has_priority = format_has_priority(fmt), has_obp1 = format_has_obp1(fmt);
	for (Tile_Tessera *tt : _tiles) {
		if (tt->id() >= n) {
			tt->id((uint16_t)(n - 1));
		}
		if (tt->palette() == -1 && m > 0) {
			tt->palette(0);
		}
		else if (tt->palette() >= m) {
			tt->palette(m - 1);
		}
		if (!can_flip) {
			tt->x_flip(false);
			tt->y_flip(false);
		}
		if (!has_priority) {
			tt->priority(false);
		}
		if (!has_obp1) {
			tt->obp1(false);
		}
	}
}

bool Tilemap::write_tiles(const char *tf, const char *af, std::vector<Tile_Tessera *> &tiles, Tilemap_Format fmt) {
	FILE *file = fl_fopen(tf, "wb");
	if (!file) { return false; }

	if (fmt == Tilemap_Format::PLAIN || fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP) {
		for (Tile_Tessera *tt : tiles) {
			int v = (int)tt->id();
			if (tt->x_flip()) { v |= 0x40; }
			if (tt->y_flip()) { v |= 0x80; }
			fputc(v, file);
		}
	}
	else if (fmt == Tilemap_Format::GBC_ATTRS) {
		for (Tile_Tessera *tt : tiles) {
			int v = (int)(tt->id() & 0xFF);
			fputc(v, file);
			int a = 0;
			if (tt->id() & 0x100) { a |= 0x08; }
			if (tt->obp1())     { a |= 0x10; }
			if (tt->priority()) { a |= 0x80; }
			if (tt->x_flip())   { a |= 0x20; }
			if (tt->y_flip())   { a |= 0x40; }
			if (tt->palette() > -1) { a |= tt->palette() & 0x07; }
			fputc(a, file);
		}
	}
	else if (fmt == Tilemap_Format::GBC_ATTRMAP) {
		FILE *attr_file = fl_fopen(af, "wb");
		if (!attr_file) { fclose(file); return false; }

		for (Tile_Tessera *tt : tiles) {
			int v = (int)(tt->id() & 0xFF);
			fputc(v, file);
			int a = 0;
			if (tt->id() & 0x100) { a |= 0x08; }
			if (tt->obp1())     { a |= 0x10; }
			if (tt->priority()) { a |= 0x80; }
			if (tt->x_flip())   { a |= 0x20; }
			if (tt->y_flip())   { a |= 0x40; }
			if (tt->palette() > -1) { a |= tt->palette() & 0x07; }
			fputc(a, attr_file);
		}

		fclose(attr_file);
	}
	else if (fmt == Tilemap_Format::GBA_PALETTES) {
		for (Tile_Tessera *tt : tiles) {
			int v = (int)(tt->id() & 0xFF);
			fputc(v, file);
			int a = (tt->id() >> 8) & 0x03;
			if (tt->x_flip()) { a |= 0x04; }
			if (tt->y_flip()) { a |= 0x08; }
			if (tt->palette() > -1) { a |= (tt->palette() << 4) & 0xF0; }
			fputc(a, file);
		}
	}
	else if (fmt == Tilemap_Format::SGB_BORDER) {
		for (Tile_Tessera *tt : tiles) {
			int v = (int)(tt->id() & 0xFF);
			fputc(v, file);
			int a = 0x10;
			if (tt->x_flip()) { a |= 0x40; }
			if (tt->y_flip()) { a |= 0x80; }
			if (tt->palette() > -1) { a |= (tt->palette() << 2) & 0x0C; }
			fputc(a, file);
		}
	}
	else if (fmt == Tilemap_Format::SNES_ATTRS) {
		for (Tile_Tessera *tt : tiles) {
			int v = (int)(tt->id() & 0xFF);
			fputc(v, file);
			int a = (tt->id() >> 8) & 0x03;
			if (tt->priority()) { a |= 0x20; }
			if (tt->x_flip())   { a |= 0x40; }
			if (tt->y_flip())   { a |= 0x80; }
			if (tt->palette() > -1) { a |= (tt->palette() << 2) & 0x1C; }
			fputc(a, file);
		}
	}
	else if (fmt == Tilemap_Format::RBY_TOWN_MAP) {
		size_t n = tiles.size();
		for (size_t i = 0; i < n;) {
			Tile_Tessera *tt = tiles[i++];
			int v = (int)tt->id(), r = 1;
			while (i < n && (int)tiles[i]->id() == v) {
				i++;
				if (++r == 0x0F) { break; } // maximum nybble
			}
			int b = (v << 4) | r;
			fputc(b, file);
		}
	}
	else if (fmt == Tilemap_Format::POKEGEAR_CARD) {
		size_t n = tiles.size();
		for (size_t i = 0; i < n;) {
			Tile_Tessera *tt = tiles[i++];
			int v = (int)tt->id(), r = 1;
			while (i < n && (int)tiles[i]->id() == v) {
				i++;
				if (++r == 0xFF) { break; } // maximum byte
			}
			fputc(v, file);
			fputc(r, file);
		}
	}

	if (fmt == Tilemap_Format::RBY_TOWN_MAP) {
		fputc(0x00, file);
	}
	else if (fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP || fmt == Tilemap_Format::POKEGEAR_CARD) {
		fputc(0xFF, file);
	}

	fclose(file);
	return true;
}

Fl_RGB_Image *Tilemap::print_tilemap() const {
	Fl_Image_Surface *surface = new Fl_Image_Surface((int)width() * TILE_SIZE, (int)height() * TILE_SIZE);
	surface->set_current();
	for (Tile_Tessera *tt : _tiles) {
		int dx = (int)tt->col() * TILE_SIZE, dy = (int)tt->row() * TILE_SIZE;
		tt->print(dx, dy, true, false);
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
	else if (n % GBA_WIDTH == 0 && n / GBA_WIDTH <= GBA_HEIGHT) {
		_width = GBA_WIDTH;
	}
	else if (n % GAME_BOY_VRAM_SIZE == 0) {
		_width = GAME_BOY_VRAM_SIZE;
	}
	else if (n % (GAME_BOY_HEIGHT - 6) == 0 && n / (GAME_BOY_HEIGHT - 6) <= GAME_BOY_WIDTH) {
		// Game Boy screen height minus textbox height
		_width = n / (GAME_BOY_HEIGHT - 6);
	}
	else {
		_width = 16;
	}
}

const char *Tilemap::error_message(Result result) {
	switch (result) {
	case TILEMAP_OK:
		return "OK.";
	case TILEMAP_BAD_FILE:
	case ATTRMAP_BAD_FILE:
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
	case TILEMAP_TOO_SHORT_ATTRS:
		return "File ends before attribute value.";
	case TILEMAP_NULL:
		return "No file chosen.";
	case ATTRMAP_TOO_SHORT:
		return "Attrmap is shorter than tilemap.";
	case ATTRMAP_TOO_LONG:
		return "Attrmap is longer than tilemap.";
	default:
		return "Unspecified error.";
	}
}
