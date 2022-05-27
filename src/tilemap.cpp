#include <cstdio>
#include <cctype>

#pragma warning(push, 0)
#include <FL/filename.H>
#pragma warning(pop)

#include "tilemap.h"
#include "tileset.h"
#include "config.h"
#include "version.h"

Tilemap::Tilemap() : _tiles(), _width(0), _result(Result::TILEMAP_NULL), _modified(false), _history(), _future() {}

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

void Tilemap::resize(size_t w, size_t h, int px, int py) {
	size_t n = w * h;
	std::vector<Tile_Tessera *> tiles;
	tiles.reserve(n);
	int mx = std::max(px, 0), my = std::max(py, 0), mw = std::min(w, width() + px), mh = std::min(h, height() + py);
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
			Tile_Tessera *tt = tile(x - px, y - py);
			tiles.emplace_back(tt ? tt : new Tile_Tessera());
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

	if (format_can_edit_palettes(Config::format())) {
		for (Tile_Tessera *tt : tiles) {
			if (tt->palette() == -1) {
				tt->palette(0);
			}
		}
	}

	clear();
	_tiles.swap(tiles);
	width(w);
	_modified = true;
}

void Tilemap::shift(int dx, int dy) {
	if (!is_rectangular()) { return; }

	size_t n = size();
	std::vector<Tile_Tessera *> tiles;
	tiles.reserve(n);

	int w = (int)width(), h = (int)height();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			tiles.emplace_back(tile((x + w - dx) % w, (y + h - dy) % h));
		}
	}

	_tiles.swap(tiles);
	_modified = true;
}

void Tilemap::transpose() {
	if (!is_rectangular()) { return; }

	size_t n = size();
	std::vector<Tile_Tessera *> tiles;
	tiles.reserve(n);

	size_t w = width(), h = height();
	for (size_t x = 0; x < w; x++) {
		for (size_t y = 0; y < h; y++) {
			tiles.emplace_back(tile(x, y));
		}
	}

	clear();
	_tiles.swap(tiles);
	width(h);
	_modified = true;
}

void Tilemap::clear() {
	_tiles.clear();
	_width = 0;
	_result = Result::TILEMAP_NULL;
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

bool Tilemap::can_format_as(Tilemap_Format fmt) {
	int n = format_tileset_size(fmt), m = format_palettes_size(fmt);
	bool can_flip = format_can_flip(fmt), has_priority = format_has_priority(fmt), has_obp1 = format_has_obp1(fmt);
	return std::all_of(RANGE(_tiles), [&](const Tile_Tessera *tt) {
		return tt->id() < n && tt->palette() < m
			&& (can_flip || (!tt->x_flip() && !tt->y_flip()))
			&& (has_priority || !tt->priority())
			&& (has_obp1 || !tt->obp1());
	});
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
	_modified = true;
}

void Tilemap::new_tiles(size_t w, size_t h) {
	clear();
	size_t n = w * h;
	_tiles.reserve(n);
	for (size_t i = 0; i < n; i++) {
		_tiles.emplace_back(new Tile_Tessera());
	}
	if (format_can_edit_palettes(Config::format())) {
		for (Tile_Tessera *tt : _tiles) {
			tt->palette(0);
		}
	}
	_width = w;
	_modified = true;
}

Tilemap::Result Tilemap::make_tiles(const std::vector<uchar> &tbytes, const std::vector<uchar> &abytes) {
	size_t c = tbytes.size();
	if (c == 0) { return (_result = Result::TILEMAP_EMPTY); }

	std::vector<Tile_Tessera *> tiles;
	size_t width = 0;
	Tilemap_Format fmt = Config::format();

	if (fmt == Tilemap_Format::PLAIN) {
		tiles.reserve(c);
		for (size_t i = 0; i < c; i++) {
			uint16_t b = tbytes[i];
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, b));
		}
	}

	else if (fmt == Tilemap_Format::GBC_ATTRS) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve(c / 2);
		for (size_t i = 0; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			if (!!(a & 0x08)) { v |= 0x100; }
			bool x_flip = !!(a & 0x20), y_flip = !!(a & 0x40), priority = !!(a & 0x80), obp1 = !!(a & 0x10);
			int palette = a & 0x07;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, priority, obp1, palette));
		}
	}

	else if (fmt == Tilemap_Format::GBC_ATTRMAP) {
		size_t ac = abytes.size();
		if (ac != c) { return (_result = ac < c ? Result::ATTRMAP_TOO_SHORT : Result::ATTRMAP_TOO_LONG); }
		tiles.reserve(c);
		for (size_t i = 0; i < c; i++) {
			uint16_t v = tbytes[i];
			uchar a = abytes[i];
			if (!!(a & 0x08)) { v |= 0x100; }
			bool x_flip = !!(a & 0x20), y_flip = !!(a & 0x40), priority = !!(a & 0x80), obp1 = !!(a & 0x10);
			int palette = a & 0x07;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, priority, obp1, palette));
		}
	}

	else if (fmt == Tilemap_Format::GBA_4BPP) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve(c / 2);
		for (size_t i = 0; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			v = v | ((a & 0x03) << 8);
			bool x_flip = !!(a & 0x04), y_flip = !!(a & 0x08);
			int palette = HI_NYB(a);
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, false, false, palette));
		}
	}

	else if (fmt == Tilemap_Format::GBA_8BPP) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve(c / 2);
		for (size_t i = 0; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			v = v | ((a & 0x03) << 8);
			bool x_flip = !!(a & 0x04), y_flip = !!(a & 0x08);
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, false, false, 0));
		}
	}

	else if (fmt == Tilemap_Format::NDS_4BPP) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve((c - NDS_HEADER_SIZE) / 2);
		for (size_t i = NDS_HEADER_SIZE; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			v = v | ((a & 0x03) << 8);
			bool x_flip = !!(a & 0x04), y_flip = !!(a & 0x08);
			int palette = HI_NYB(a);
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, false, false, palette));
		}
		width = NDS_WIDTH;
	}

	else if (fmt == Tilemap_Format::NDS_8BPP) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve((c - NDS_HEADER_SIZE) / 2);
		for (size_t i = NDS_HEADER_SIZE; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			v = v | ((a & 0x03) << 8);
			bool x_flip = !!(a & 0x04), y_flip = !!(a & 0x08);
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, false, false, 0));
		}
		width = NDS_WIDTH;
	}

	else if (fmt == Tilemap_Format::SGB_BORDER) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve(c / 2);
		for (size_t i = 0; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			bool x_flip = !!(a & 0x40), y_flip = !!(a & 0x80);
			int palette = (a & 0x0C) >> 2;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, false, false, palette));
		}
		width = SGB_WIDTH;
	}

	else if (fmt == Tilemap_Format::SNES_ATTRS) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve(c / 2);
		for (size_t i = 0; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			v = v | ((a & 0x03) << 8);
			bool x_flip = !!(a & 0x40), y_flip = !!(a & 0x80), priority = !!(a & 0x20);
			int palette = (a & 0x1C) >> 2;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, priority, false, palette));
		}
	}

	else if (fmt == Tilemap_Format::TG16) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve(c / 2);
		for (size_t i = 0; i < c; i += 2) {
			uint16_t v = tbytes[i];
			uchar a = tbytes[i+1];
			v = v | ((a & 0x07) << 8);
			int palette = HI_NYB(a);
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, false, false, false, false, palette));
		}
	}

	else if (fmt == Tilemap_Format::GENESIS) {
		if (c % 2) { return (_result = Result::TILEMAP_TOO_SHORT_ATTRS); }
		tiles.reserve(c / 2);
		for (size_t i = 0; i < c; i += 2) {
			uchar a = tbytes[i];
			uint16_t v = tbytes[i+1];
			v = v | ((a & 0x07) << 8);
			bool x_flip = !!(a & 0x08), y_flip = !!(a & 0x10), priority = !!(a & 0x80);
			int palette = (a & 0x60) >> 5;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip, priority, false, palette));
		}
	}

	else if (fmt == Tilemap_Format::RBY_TOWN_MAP) {
		tiles.reserve(c);
		for (size_t i = 0; i < c - 1; i++) {
			uchar b = tbytes[i];
			if (b == 0x00) {
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = Result::TILEMAP_TOO_LONG_00);
			}
			uint16_t v = HI_NYB(b), r = LO_NYB(b);
			for (uint16_t j = 0; j < r; j++) {
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v));
			}
		}
		if (tbytes[c-1] != 0x00) {
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = Result::TILEMAP_TOO_SHORT_00);
		}
		width = GAME_BOY_WIDTH;
	}

	else if (fmt == Tilemap_Format::GSC_TOWN_MAP) {
		tiles.reserve(c);
		for (size_t i = 0; i < c - 1; i++) {
			uint16_t b = tbytes[i];
			if (b == 0xFF) {
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = Result::TILEMAP_TOO_LONG_FF);
			}
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, b));
		}
		if (tbytes[c-1] != 0xFF) {
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = Result::TILEMAP_TOO_SHORT_FF);
		}
		width = GAME_BOY_WIDTH;
	}

	else if (fmt == Tilemap_Format::PC_TOWN_MAP) {
		tiles.reserve(c);
		for (size_t i = 0; i < c - 1; i++) {
			uchar b = tbytes[i];
			if (b == 0xFF) {
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = Result::TILEMAP_TOO_LONG_FF);
			}
			bool x_flip = !!(b & 0x40), y_flip = !!(b & 0x80);
			uint16_t v = b & 0x3F;
			tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v, x_flip, y_flip));
		}
		if (tbytes[c-1] != 0xFF) {
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = Result::TILEMAP_TOO_SHORT_FF);
		}
		width = GAME_BOY_WIDTH;
	}

	else if (fmt == Tilemap_Format::SW_TOWN_MAP) {
		tiles.reserve(c);
		if (!(c % 2)) { return (_result = Result::TILEMAP_TOO_SHORT_00); }
		for (size_t i = 0; i < c - 1; i += 2) {
			uint16_t v = tbytes[i];
			if (v == 0x00) {
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = Result::TILEMAP_TOO_LONG_00);
			}
			uint16_t r = tbytes[i+1];
			if (r == 0x00) {
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = Result::TILEMAP_TOO_LONG_00);
			}
			for (uint16_t j = 0; j < r; j++) {
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v));
			}
		}
		if (tbytes[c-1] != 0x00) {
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = Result::TILEMAP_TOO_SHORT_00);
		}
		width = GAME_BOY_WIDTH;
	}

	else if (fmt == Tilemap_Format::POKEGEAR_CARD) {
		tiles.reserve(c);
		if (!(c % 2)) { return (_result = Result::TILEMAP_TOO_SHORT_FF); }
		for (size_t i = 0; i < c - 1; i += 2) {
			uint16_t v = tbytes[i];
			if (v == 0xFF) {
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = Result::TILEMAP_TOO_LONG_FF);
			}
			uint16_t r = tbytes[i+1];
			if (r == 0xFF) {
				for (Tile_Tessera *tt : tiles) { delete tt; }
				return (_result = Result::TILEMAP_TOO_LONG_FF);
			}
			for (uint16_t j = 0; j < r; j++) {
				tiles.emplace_back(new Tile_Tessera(0, 0, 0, 0, v));
			}
		}
		if (tbytes[c-1] != 0xFF) {
			for (Tile_Tessera *tt : tiles) { delete tt; }
			return (_result = Result::TILEMAP_TOO_SHORT_FF);
		}
		width = GAME_BOY_WIDTH;
	}

	if (tiles.empty()) { return (_result = Result::TILEMAP_EMPTY); }

	_tiles.swap(tiles);
	if (width > 0) { _width = width; }
	else { guess_width(); }

	return (_result = Result::TILEMAP_OK);
}

static bool read_file_bytes(const char *f, std::vector<uchar> &bytes) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return false; }
	size_t n = file_size(file);
	bytes.reserve(n);
	for (int b = fgetc(file); b != EOF; b = fgetc(file)) {
		bytes.push_back((uchar)b);
	}
	fclose(file);
	return true;
}

Tilemap::Result Tilemap::read_tiles(const char *tf, const char *af) {
	std::vector<uchar> tbytes, abytes;
	if (!read_file_bytes(tf, tbytes)) { return (_result = Result::TILEMAP_BAD_FILE); }
	if (af && af[0] && !read_file_bytes(af, abytes)) { return (_result = Result::ATTRMAP_BAD_FILE); }
	return make_tiles(tbytes, abytes);
}

bool Tilemap::write_tiles(const char *tf, const char *af, Tilemap_Format fmt) {
	FILE *file = fl_fopen(tf, "wb");
	if (!file) { return false; }

	std::vector<uchar> bytes = make_tilemap_bytes(_tiles, fmt, width(), height());
	if (fmt == Tilemap_Format::GBC_ATTRMAP) {
		FILE *attr_file = fl_fopen(af, "wb");
		if (!attr_file) { fclose(file); return false; }

		size_t nb = bytes.size() / 2;
		fwrite(bytes.data(), 1, nb, file);
		fwrite(bytes.data() + nb, 1, nb, attr_file);

		fclose(attr_file);
	}
	else {
		fwrite(bytes.data(), 1, bytes.size(), file);
	}

	fclose(file);
	return true;
}

bool Tilemap::export_tiles(const char *f) const {
	FILE *file = fl_fopen(f, "wb");
	if (!file) { return false; }

	Tilemap_Format fmt = Config::format();
	std::vector<uchar> bytes = make_tilemap_bytes(_tiles, fmt, width(), height());
	if (ends_with_ignore_case(f, ".csv")) {
		export_csv_tiles(file, bytes, fmt);
	}
	else if (ends_with_ignore_case(f, ".c") || ends_with_ignore_case(f, ".h")) {
		export_c_tiles(file, bytes, fmt, f);
	}
	else {
		export_asm_tiles(file, bytes, fmt, f);
	}

	fclose(file);
	return true;
}

static void escape_filename(char *name, size_t len, const char *f) {
	const char *basename = fl_filename_name(f);
	strcpy(name, basename);
	fl_filename_setext(name, len, NULL);
	size_t n = strlen(name);
	for (size_t i = 0; i < n; i++) {
		if (!isalnum(name[i])) {
			name[i] = '_';
		}
	}
	if (isdigit(name[0])) {
		for (size_t i = 0; i < n; i++) {
			name[i+1] = name[i];
		}
		name[0] = '_';
	}
}

void Tilemap::export_c_tiles(FILE *file, const std::vector<uchar> &bytes, Tilemap_Format fmt, const char *f) const {
	char name[FL_PATH_MAX] = {};
	escape_filename(name, sizeof(name), f);
	fprintf(file, "/*\n Tilemap: %zu x %zu, %s\n Exported by " PROGRAM_NAME "\n*/\n\n",
		width(), height(), format_name(fmt));
	if (fmt == Tilemap_Format::GBC_ATTRMAP) {
		size_t nb = bytes.size() / 2;
		fprintf(file, "unsigned char %s_tilemap[] = {", name);
		for (size_t i = 0; i < nb; i++) {
			if (i % 12 == 0) fputs("\n ", file);
			fprintf(file, " 0x%02x", bytes[i]);
			if (i < nb - 1) fputc(',', file);
		}
		fputs("\n};\n\n", file);
		fprintf(file, "unsigned char %s_attrmap[] = {", name);
		for (size_t i = 0; i < nb; i++) {
			if (i % 12 == 0) fputs("\n ", file);
			fprintf(file, " 0x%02x", bytes[nb+i]);
			if (i < nb - 1) fputc(',', file);
		}
		fputs("\n};\n\n", file);
		fprintf(file, "unsigned int %s_len = %zu;\n", name, nb);
	}
	else {
		size_t nb = bytes.size();
		fprintf(file, "unsigned char %s_tilemap[] = {", name);
		for (size_t i = 0; i < nb; i++) {
			if (i % 12 == 0) fputs("\n ", file);
			fprintf(file, " 0x%02x", bytes[i]);
			if (i < nb - 1) fputc(',', file);
		}
		fputs("\n};\n\n", file);
		fprintf(file, "unsigned int %s_len = %zu;\n", name, nb);
	}
}

void Tilemap::export_asm_tiles(FILE *file, const std::vector<uchar> &bytes, Tilemap_Format fmt, const char *f) const {
	char name[FL_PATH_MAX] = {};
	escape_filename(name, sizeof(name), f);
	fprintf(file, "; Tilemap: %zu x %zu, %s\n; Exported by " PROGRAM_NAME "\n\n",
		width(), height(), format_name(fmt));
	size_t rw = width() * format_bytes_per_tile(fmt);
	if (rw == 0) { rw = 16; }
	if (fmt == Tilemap_Format::GBC_ATTRMAP) {
		size_t nb = bytes.size() / 2;
		fprintf(file, "%s_Tilemap::", name);
		for (size_t i = 0; i < nb; i++) {
			if (i % rw == 0) fputs("\n\tdb", file);
			fprintf(file, " $%02x", bytes[i]);
			if (i < nb - 1 && i % rw != rw - 1) fputc(',', file);
		}
		fputs("\n.end::\n\n", file);
		fprintf(file, "%s_Attrmap::", name);
		for (size_t i = 0; i < nb; i++) {
			if (i % rw == 0) fputs("\n\tdb", file);
			fprintf(file, " $%02x", bytes[nb+i]);
			if (i < nb - 1 && i % rw != rw - 1) fputc(',', file);
		}
		fputs("\n.end::\n\n", file);
		fprintf(file, "%s_LEN EQU %zu\n", name, nb);
	}
	else {
		size_t nb = bytes.size();
		fprintf(file, "%s_Tilemap::", name);
		for (size_t i = 0; i < nb; i++) {
			if (i % rw == 0) fputs("\n\tdb ", file);
			fprintf(file, " $%02x", bytes[i]);
			if (i < nb - 1 && i % rw != rw - 1) fputc(',', file);
		}
		fputs("\n\n", file);
		fprintf(file, "%s_LEN EQU %zu\n", name, nb);
	}
}

void Tilemap::export_csv_tiles(FILE *file, const std::vector<uchar> &bytes, Tilemap_Format fmt) const {
	size_t rw = width() * format_bytes_per_tile(fmt);
	size_t nb = bytes.size();
	for (size_t i = 0; i < nb; i++) {
		fprintf(file, "%d", bytes[i]);
		fputc(i < nb - 1 && (rw == 0 || i % rw != rw - 1) ? ',' : '\n', file);
	}
}

void Tilemap::print_tilemap() const {
	for (Tile_Tessera *tt : _tiles) {
		int dx = (int)tt->col() * TILE_SIZE, dy = (int)tt->row() * TILE_SIZE;
		tt->print(dx, dy, true, false);
	}
}

static size_t sqrt(size_t n) {
	for (size_t r = 1; r <= n / 2; r++) {
		if (r * r == n) { return r; }
	}
	return 0;
}

void Tilemap::guess_width() {
	size_t n = size();
#define N_FITS_SIZE(w, h) n % (w) == 0 && n / (w) <= (h)
	if (N_FITS_SIZE(GAME_BOY_WIDTH, GAME_BOY_HEIGHT)) {
		_width = GAME_BOY_WIDTH;
	}
	else if (N_FITS_SIZE(GAME_BOY_HEIGHT, GAME_BOY_WIDTH)) {
		_width = n / GAME_BOY_HEIGHT;
	}
	else if (N_FITS_SIZE(GBA_WIDTH, GBA_HEIGHT)) {
		_width = GBA_WIDTH;
	}
	else if (N_FITS_SIZE(GAME_BOY_VRAM_SIZE, GAME_BOY_VRAM_SIZE)) {
		_width = GAME_BOY_VRAM_SIZE;
	}
	else if (N_FITS_SIZE(GAME_BOY_HEIGHT - 6, GAME_BOY_WIDTH)) {
		// Game Boy screen height minus textbox height
		_width = n / (GAME_BOY_HEIGHT - 6);
	}
	else if (N_FITS_SIZE(64, 64)) {
		_width = 64;
	}
	else if (size_t r = sqrt(n); r > 0) {
		_width = r;
	}
	else {
		_width = 16;
	}
#undef N_FITS_SIZE
}

const char *Tilemap::error_message(Result result) {
	switch (result) {
	case Result::TILEMAP_OK:
		return "OK.";
	case Result::TILEMAP_BAD_FILE:
	case Result::ATTRMAP_BAD_FILE:
		return "Cannot open file.";
	case Result::TILEMAP_EMPTY:
		return "Tilemap is empty.";
	case Result::TILEMAP_TOO_SHORT_FF:
		return "File ends before any $FF.";
	case Result::TILEMAP_TOO_LONG_FF:
		return "File continues after $FF.";
	case Result::TILEMAP_TOO_SHORT_00:
		return "File ends before any $00.";
	case Result::TILEMAP_TOO_LONG_00:
		return "File continues after $00.";
	case Result::TILEMAP_TOO_SHORT_RLE:
		return "File ends before RLE value.";
	case Result::TILEMAP_TOO_SHORT_ATTRS:
		return "File ends before attribute value.";
	case Result::TILEMAP_INVALID:
	case Result::ATTRMAP_INVALID:
		return "Cannot parse file format.";
	case Result::TILEMAP_NULL:
		return "No file chosen.";
	case Result::ATTRMAP_TOO_SHORT:
		return "Attrmap is shorter than tilemap.";
	case Result::ATTRMAP_TOO_LONG:
		return "Attrmap is longer than tilemap.";
	default:
		return "Unspecified error.";
	}
}
