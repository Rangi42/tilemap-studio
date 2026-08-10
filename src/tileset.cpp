#include <array>
#include <vector>
#include <cstring>
#include <zlib.h>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/fl_types.h>
#include <FL/fl_utf8.h>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "utils.h"
#include "tileset.h"
#include "tile-buttons.h"
#include "config.h"

Tileset::Tileset(int start_id, int offset, int length) : _1x_image(NULL), _2x_image(NULL), _zoomed_image(NULL),
	_num_tiles(0), _start_id(start_id), _offset(offset), _length(length), _result(Result::TILESET_NULL),
	_color_indices(), _has_color_indices(false) {}

Tileset::~Tileset() {}

void Tileset::clear() {
	delete _1x_image;
	_1x_image = NULL;
	delete _2x_image;
	_2x_image = NULL;
	delete _zoomed_image;
	_zoomed_image = NULL;
	_num_tiles = 0;
	_start_id = 0x000;
	_offset = 0;
	_length = 0;
	_result = Result::TILESET_NULL;
	_color_indices.clear();
	_color_indices.shrink_to_fit();
	_has_color_indices = false;
}

const uchar *Tileset::tile_color_indices(int index) const {
	if (!_has_color_indices || index < 0 || (size_t)index >= _color_indices.size()) { return NULL; }
	return _color_indices[index].data();
}

size_t Tileset::color_palette(Palette &pal, size_t n) const {
	pal.assign(n, FL_BLACK);
	if (!_has_color_indices || !_1x_image || n == 0) { return 0; }
	const uchar *data = (const uchar *)_1x_image->data()[0];
	int d = _1x_image->d(), ld = _1x_image->ld();
	int w = _1x_image->w(), h = _1x_image->h();
	if (!ld) { ld = w * d; }
	int wt = w / TILE_SIZE;
	if (wt <= 0) { return 0; }
	std::vector<bool> seen(n, false);
	size_t found = 0;
	for (int y = 0; y < h && found < n; y++) {
		for (int x = 0; x < w; x++) {
			size_t t = (size_t)(x / TILE_SIZE) + (size_t)(y / TILE_SIZE) * wt;
			size_t k = (size_t)(y % TILE_SIZE) * TILE_SIZE + (x % TILE_SIZE);
			if (t >= _color_indices.size()) { continue; }
			uchar ci = _color_indices[t][k];
			if (ci >= n || seen[ci]) { continue; }
			const uchar *px = data + (size_t)y * ld + (size_t)x * d;
			uchar r = px[0], g = d > 1 ? px[1] : px[0], b = d > 2 ? px[2] : px[0];
			pal[ci] = fl_rgb_color(r, g, b);
			seen[ci] = true;
			if (++found == n) { break; }
		}
	}
	return found;
}

void Tileset::update_zoom() {
	if (!_1x_image) { return; }
	int z = Config::zoom();
	_zoomed_image = (Fl_RGB_Image *)_1x_image->copy(_1x_image->w() * z, _1x_image->h() * z);
}

void Tileset::shift(int dn) {
	_start_id += dn;
}

bool Tileset::draw_tile(const Tile_State *ts, int x, int y, int z, bool active) const {
	int index = (int)ts->id - _start_id + _offset;
	int limit = (int)_num_tiles;
	if (_length > 0) { limit = std::min(limit, _length + _offset); }
	if (index < _offset || index >= limit || !_2x_image || !_zoomed_image) { return false; }

	int s = TILE_SIZE * z;
	if (!active) {
		fl_rectf(x, y, s, s, FL_INACTIVE_COLOR);
		return true;
	}

	const Palettes *ps = Tile_State::palette_set();
	if (ts->palette >= 0 && ps && (size_t)ts->palette < ps->size() && _has_color_indices) {
		const uchar *idx = tile_color_indices(index);
		if (idx) { return draw_recolored(idx, (*ps)[ts->palette], x, y, z, ts->x_flip, ts->y_flip); }
	}

	if (z == DEFAULT_ZOOM) {
		int wt = _2x_image->w() / TILE_SIZE_2X;
		int tx = index % wt * TILE_SIZE_2X, ty = index / wt * TILE_SIZE_2X;
		if (!ts->x_flip && !ts->y_flip) {
			_2x_image->draw(x, y, TILE_SIZE_2X, TILE_SIZE_2X, tx, ty);
		}
		else {
			const uchar *data = (const uchar *)_2x_image->data()[0];
			int d = _2x_image->d(), ld = _2x_image->ld();
			if (!ld) { ld = _2x_image->w() * d; }
			data += ty * ld + tx * d + (ts->y_flip ? ts->x_flip ? ld + d : ld : ts->x_flip ? d : 0) * (TILE_SIZE_2X - 1);
			int td = ts->x_flip ? -d : d;
			int tld = ts->y_flip ? -ld : ld;
			fl_draw_image(data, x, y, TILE_SIZE_2X, TILE_SIZE_2X, td, tld);
		}
	}
	else {
		int wt = _zoomed_image->w() / s;
		int tx = index % wt * s, ty = index / wt * s;
		if (!ts->x_flip && !ts->y_flip) {
			_zoomed_image->draw(x, y, s, s, tx, ty);
		}
		else {
			const uchar *data = (const uchar *)_zoomed_image->data()[0];
			int d = _zoomed_image->d(), ld = _zoomed_image->ld();
			if (!ld) { ld = _zoomed_image->w() * d; }
			data += ty * ld + tx * d + (ts->y_flip ? ts->x_flip ? ld + d : ld : ts->x_flip ? d : 0) * (s - 1);
			int td = ts->x_flip ? -d : d;
			int tld = ts->y_flip ? -ld : ld;
			fl_draw_image(data, x, y, s, s, td, tld);
		}
	}
	return true;
}

bool Tileset::print_tile(const Tile_State *ts, int x, int y, bool active) const {
	int index = (int)ts->id - _start_id + _offset;
	int limit = (int)_num_tiles;
	if (_length > 0) { limit = std::min(limit, _length + _offset); }
	if (index < _offset || index >= limit || !_1x_image) { return false; }

	if (!active) {
		fl_rectf(x, y, TILE_SIZE, TILE_SIZE, FL_INACTIVE_COLOR);
		return true;
	}

	const Palettes *ps = Tile_State::palette_set();
	if (ts->palette >= 0 && ps && (size_t)ts->palette < ps->size() && _has_color_indices) {
		const uchar *idx = tile_color_indices(index);
		if (idx) { return draw_recolored(idx, (*ps)[ts->palette], x, y, 1, ts->x_flip, ts->y_flip); }
	}

	int wt = _1x_image->w() / TILE_SIZE;
	int tx = index % wt * TILE_SIZE, ty = index / wt * TILE_SIZE;

	if (!ts->x_flip && !ts->y_flip) {
		_1x_image->draw(x, y, TILE_SIZE, TILE_SIZE, tx, ty);
	}
	else {
		const uchar *data = (const uchar *)_1x_image->data()[0];
		int d = _1x_image->d(), ld = _1x_image->ld();
		if (!ld) { ld = _1x_image->w() * d; }
		data += ty * ld + tx * d + (ts->y_flip ? ts->x_flip ? ld + d : ld : ts->x_flip ? d : 0) * (TILE_SIZE - 1);
		int td = ts->x_flip ? -d : d;
		int tld = ts->y_flip ? -ld : ld;
		fl_draw_image(data, x, y, TILE_SIZE, TILE_SIZE, td, tld);
	}
	return true;
}

// Draw a tile recolored through the given palette: look up each pixel's stored index in the
// palette, nearest-neighbor scale to zoom z, and apply flips. idx points at NUM_TILE_PIXELS bytes.
bool Tileset::draw_recolored(const uchar *idx, const Palette &palette, int x, int y, int z, bool x_flip, bool y_flip) const {
	int s = TILE_SIZE * z;
	size_t nc = palette.size();
	std::vector<uchar> buf((size_t)s * s * 3);
	for (int py = 0; py < s; py++) {
		int ty = py / z;
		int sy = y_flip ? TILE_SIZE - 1 - ty : ty;
		for (int px = 0; px < s; px++) {
			int tx = px / z;
			int sx = x_flip ? TILE_SIZE - 1 - tx : tx;
			uchar ci = idx[sy * TILE_SIZE + sx];
			uchar r = 0, g = 0, b = 0;
			if (ci < nc) { Fl::get_color(palette[ci], r, g, b); }
			size_t o = ((size_t)py * s + px) * 3;
			buf[o] = r; buf[o+1] = g; buf[o+2] = b;
		}
	}
	fl_draw_image(buf.data(), x, y, s, s, 3, 0);
	return true;
}

Tileset::Result Tileset::read_tiles(const char *f) {
	std::string s(f);
	if (ends_with_ignore_case(s, ".png")) { return read_png_graphics(f); }
	if (ends_with_ignore_case(s, ".gif")) { return read_gif_graphics(f); }
	if (ends_with_ignore_case(s, ".bmp")) { return read_bmp_graphics(f); }
	if (ends_with_ignore_case(s, ".1bpp")) { return read_1bpp_graphics(f); }
	if (ends_with_ignore_case(s, ".2bpp")) { return read_2bpp_graphics(f); }
	if (ends_with_ignore_case(s, ".4bpp")) { return read_4bpp_graphics(f); }
	if (ends_with_ignore_case(s, ".8bpp")) { return read_8bpp_graphics(f); }
	if (ends_with_ignore_case(s, ".1bpp.lz")) { return read_1bpp_lz_graphics(f); }
	if (ends_with_ignore_case(s, ".2bpp.lz")) { return read_2bpp_lz_graphics(f); }
	if (ends_with_ignore_case(s, ".rgcn")) { return read_rgcn_graphics(f); }
	if (ends_with_ignore_case(s, ".ncgr")) { return read_rgcn_graphics(f); }
	if (ends_with_ignore_case(s, ".rmp")) { return read_rts_graphics(f, true); }
	if (ends_with_ignore_case(s, ".rts")) { return read_rts_graphics(f, false); }
	return (_result = Result::TILESET_BAD_EXT);
}

// Decode an indexed (color type 3) PNG's raw palette indices, one byte per pixel, row-major.
// Returns false for truecolor, interlaced, or unsupported PNGs (they simply get no indices).
static bool read_indexed_png(const char *f, std::vector<uchar> &pixels, int &width, int &height) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return false; }
	uchar sig[8];
	if (fread(sig, 1, 8, file) != 8 || memcmp(sig, "\x89PNG\r\n\x1a\n", 8)) { fclose(file); return false; }

	uint32_t w = 0, h = 0;
	int bit_depth = 0, color_type = -1, interlace = 0;
	bool have_hdr = false;
	std::vector<uchar> idat;
	for (;;) {
		uchar len4[4], typ[4];
		if (fread(len4, 1, 4, file) != 4 || fread(typ, 1, 4, file) != 4) { break; }
		uint32_t len = ((uint32_t)len4[0] << 24) | (len4[1] << 16) | (len4[2] << 8) | len4[3];
		if (!memcmp(typ, "IHDR", 4)) {
			uchar hdr[13];
			if (len < 13 || fread(hdr, 1, 13, file) != 13) { break; }
			w = ((uint32_t)hdr[0] << 24) | (hdr[1] << 16) | (hdr[2] << 8) | hdr[3];
			h = ((uint32_t)hdr[4] << 24) | (hdr[5] << 16) | (hdr[6] << 8) | hdr[7];
			bit_depth = hdr[8]; color_type = hdr[9]; interlace = hdr[12];
			have_hdr = true;
			fseek(file, (long)(len - 13) + 4, SEEK_CUR); // rest of IHDR + CRC
		}
		else if (!memcmp(typ, "IDAT", 4)) {
			size_t off = idat.size();
			idat.resize(off + len);
			if (fread(idat.data() + off, 1, len, file) != len) { break; }
			fseek(file, 4, SEEK_CUR); // CRC
		}
		else if (!memcmp(typ, "IEND", 4)) { break; }
		else { fseek(file, (long)len + 4, SEEK_CUR); }
	}
	fclose(file);

	if (!have_hdr || color_type != 3 || interlace != 0 || idat.empty()) { return false; }
	if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 && bit_depth != 8) { return false; }

	size_t scan = ((size_t)w * bit_depth + 7) / 8; // scanline bytes, excluding the filter byte
	size_t raw_size = (scan + 1) * h;
	std::vector<uchar> raw(raw_size);
	uLongf dst = (uLongf)raw_size;
	if (uncompress(raw.data(), &dst, idat.data(), (uLong)idat.size()) != Z_OK || dst != raw_size) { return false; }

	// Reverse the PNG scanline filters (indexed <=8bpp means 1 byte per filter unit).
	std::vector<uchar> out((size_t)scan * h);
	for (uint32_t y = 0; y < h; y++) {
		uchar ft = raw[(size_t)y * (scan + 1)];
		const uchar *in = raw.data() + (size_t)y * (scan + 1) + 1;
		uchar *cur = out.data() + (size_t)y * scan;
		const uchar *prev = y ? out.data() + (size_t)(y - 1) * scan : NULL;
		for (size_t i = 0; i < scan; i++) {
			int a = i ? cur[i-1] : 0;
			int b = prev ? prev[i] : 0;
			int c = (prev && i) ? prev[i-1] : 0;
			int v = in[i];
			switch (ft) {
			case 0: break;
			case 1: v += a; break;
			case 2: v += b; break;
			case 3: v += (a + b) / 2; break;
			case 4: {
				int p = a + b - c, pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
				v += (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
				break;
			}
			default: return false;
			}
			cur[i] = (uchar)v;
		}
	}

	// Unpack indices (MSB-first within each byte for sub-8-bit depths).
	pixels.assign((size_t)w * h, 0);
	int mask = (1 << bit_depth) - 1, ppb = 8 / bit_depth;
	for (uint32_t y = 0; y < h; y++) {
		const uchar *row = out.data() + (size_t)y * scan;
		for (uint32_t x = 0; x < w; x++) {
			int val = bit_depth == 8 ? row[x] :
				(row[x / ppb] >> (8 - bit_depth * ((int)(x % ppb) + 1))) & mask;
			pixels[(size_t)y * w + x] = (uchar)val;
		}
	}
	width = (int)w;
	height = (int)h;
	return true;
}

Tileset::Result Tileset::read_png_graphics(const char *f) {
	Fl_PNG_Image *png = new Fl_PNG_Image(f);
	Result result = postprocess_graphics(png);
	if (result == Result::TILESET_OK && _1x_image) {
		std::vector<uchar> pixels;
		int w = 0, h = 0;
		if (read_indexed_png(f, pixels, w, h) && w == _1x_image->w() && h == _1x_image->h()) {
			build_color_indices_from_pixels(pixels, w, h); // exact per-pixel indices from an indexed PNG
		}
	}
	return result;
}

// Lay row-major pixel indices out tile-major to match tile_color_indices()/draw_tile().
void Tileset::build_color_indices_from_pixels(const std::vector<uchar> &pixels, int w, int h) {
	int wt = w / TILE_SIZE;
	if (wt <= 0) { return; }
	_color_indices.assign(_num_tiles, {});
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			size_t t = (size_t)(x / TILE_SIZE) + (size_t)(y / TILE_SIZE) * wt;
			size_t k = (size_t)(y % TILE_SIZE) * TILE_SIZE + (x % TILE_SIZE);
			size_t s = (size_t)y * w + x;
			if (t < _color_indices.size() && s < pixels.size()) { _color_indices[t][k] = pixels[s]; }
		}
	}
	_has_color_indices = true;
}

Tileset::Result Tileset::read_gif_graphics(const char *f) {
	Fl_GIF_Image gif(f);
	if (gif.fail()) { return (_result = Result::TILESET_BAD_FILE); }
	Fl_RGB_Image *img = new Fl_RGB_Image(&gif, FL_WHITE);
	return postprocess_graphics(img);
}

Tileset::Result Tileset::read_bmp_graphics(const char *f) {
	Fl_BMP_Image *bmp = new Fl_BMP_Image(f);
	return postprocess_graphics(bmp);
}

Tileset::Result Tileset::read_1bpp_graphics(const char *f) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return (_result = Result::TILESET_BAD_FILE); }

	size_t n = file_size(file);
	if (n % BYTES_PER_1BPP_TILE) { fclose(file); return (_result = Result::TILESET_BAD_DIMS); }

	std::vector<uchar> data(n);
	size_t r = fread(data.data(), 1, n, file);
	fclose(file);
	if (r != n) { return (_result = Result::TILESET_BAD_FILE); }

	return parse_1bpp_data(data);
}

Tileset::Result Tileset::read_2bpp_graphics(const char *f) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return (_result = Result::TILESET_BAD_FILE); }

	size_t n = file_size(file);
	if (n % BYTES_PER_2BPP_TILE) { fclose(file); return (_result = Result::TILESET_BAD_DIMS); }

	std::vector<uchar> data(n);
	size_t r = fread(data.data(), 1, n, file);
	fclose(file);
	if (r != n) { return (_result = Result::TILESET_BAD_FILE); }

	return parse_2bpp_data(data);
}

Tileset::Result Tileset::read_4bpp_graphics(const char *f) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return (_result = Result::TILESET_BAD_FILE); }

	size_t n = file_size(file);
	if (n % BYTES_PER_4BPP_TILE) { fclose(file); return (_result = Result::TILESET_BAD_DIMS); }

	std::vector<uchar> data(n);
	size_t r = fread(data.data(), 1, n, file);
	fclose(file);
	if (r != n) { return (_result = Result::TILESET_BAD_FILE); }

	return parse_4bpp_data(data);
}

Tileset::Result Tileset::read_8bpp_graphics(const char *f) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return (_result = Result::TILESET_BAD_FILE); }

	size_t n = file_size(file);
	if (n % BYTES_PER_8BPP_TILE) { fclose(file); return (_result = Result::TILESET_BAD_DIMS); }

	std::vector<uchar> data(n);
	size_t r = fread(data.data(), 1, n, file);
	fclose(file);
	if (r != n) { return (_result = Result::TILESET_BAD_FILE); }

	return parse_8bpp_data(data);
}

static Tileset::Result decompress_lz_data(const char *f, std::vector<uchar> &data);

Tileset::Result Tileset::read_1bpp_lz_graphics(const char *f) {
	std::vector<uchar> data(MAX_NUM_TILES * BYTES_PER_1BPP_TILE);
	if (decompress_lz_data(f, data) != Result::TILESET_OK) {
		return _result;
	}
	return parse_1bpp_data(data);
}

Tileset::Result Tileset::read_2bpp_lz_graphics(const char *f) {
	std::vector<uchar> data(MAX_NUM_TILES * BYTES_PER_2BPP_TILE);
	if (decompress_lz_data(f, data) != Result::TILESET_OK) {
		return _result;
	}
	return parse_2bpp_data(data);
}

enum class Hue { WHITE, DARK, LIGHT, BLACK };

static Fl_Color hue_colors[NUM_HUES] = {fl_rgb_color(0xFF), fl_rgb_color(0x55), fl_rgb_color(0xAA), fl_rgb_color(0x00)};

static void convert_1bpp_row(uchar b, Hue *row) {
	// %ABCD_EFGH -> %A %B %C %D %E %F %G %H
	for (int i = 0; i < TILE_SIZE; i++) {
		int j = TILE_SIZE - i - 1;
		row[i] = (b >> j & 1) ? Hue::BLACK : Hue::WHITE;
	}
}

static void convert_2bpp_row(uchar b1, uchar b2, Hue *row) {
	// %ABCD_EFGH %abcd_efgh -> %Aa %Bb %Cc %Dd %Ee %Ff %GG %Hh
	for (int i = 0; i < TILE_SIZE; i++) {
		int j = TILE_SIZE - i - 1;
		row[i] = (Hue)((b1 >> j & 1) * 2 + (b2 >> j & 1));
	}
}

Tileset::Result Tileset::parse_1bpp_data(const std::vector<uchar> &data) {
	_num_tiles = data.size() / BYTES_PER_1BPP_TILE;

	int limit = (int)_num_tiles - _offset;
	if (_length > 0) { limit = std::min(limit, _length + _offset); }
	if (_start_id + limit > MAX_NUM_TILES) { return (_result = Result::TILESET_TOO_LARGE); }

	Fl_Image_Surface *surface = new Fl_Image_Surface(TILE_SIZE, (int)_num_tiles * TILE_SIZE);
	surface->set_current();

	_color_indices.assign(_num_tiles, {});
	Hue row[TILE_SIZE] = {};
	for (size_t i = 0; i < _num_tiles; i++) {
		for (size_t j = 0; j < TILE_SIZE; j++) {
			uchar b = data[i * BYTES_PER_1BPP_TILE + j];
			convert_1bpp_row(b, row);
			for (int k = 0; k < TILE_SIZE; k++) {
				Hue hue = row[k];
				fl_color(hue_colors[(int)hue]);
				fl_point(k, (int)(i * TILE_SIZE + j));
				_color_indices[i][j * TILE_SIZE + k] = (uchar)((b >> (TILE_SIZE - k - 1)) & 1);
			}
		}
	}
	_has_color_indices = true;

	Fl_RGB_Image *img = surface->image();
	delete surface;
	Fl_Display_Device::display_device()->set_current();

	return postprocess_graphics(img);
}

Tileset::Result Tileset::parse_2bpp_data(const std::vector<uchar> &data) {
	_num_tiles = data.size() / BYTES_PER_2BPP_TILE;

	int limit = (int)_num_tiles - _offset;
	if (_length > 0) { limit = std::min(limit, _length + _offset); }
	if (_start_id + limit > MAX_NUM_TILES) { return (_result = Result::TILESET_TOO_LARGE); }

	Fl_Image_Surface *surface = new Fl_Image_Surface(TILE_SIZE, (int)_num_tiles * TILE_SIZE);
	surface->set_current();

	_color_indices.assign(_num_tiles, {});
	Hue row[TILE_SIZE] = {};
	for (size_t i = 0; i < _num_tiles; i++) {
		for (size_t j = 0; j < TILE_SIZE; j++) {
			uchar b1 = data[i * BYTES_PER_2BPP_TILE + j * 2];
			uchar b2 = data[i * BYTES_PER_2BPP_TILE + j * 2 + 1];
			convert_2bpp_row(b1, b2, row);
			for (int k = 0; k < TILE_SIZE; k++) {
				Hue hue = row[k];
				fl_color(hue_colors[(int)hue]);
				fl_point(k, (int)(i * TILE_SIZE + j));
				_color_indices[i][j * TILE_SIZE + k] = (uchar)(int)hue;
			}
		}
	}
	_has_color_indices = true;

	Fl_RGB_Image *img = surface->image();
	delete surface;
	Fl_Display_Device::display_device()->set_current();

	return postprocess_graphics(img);
}

static Fl_Color bpp4_colors[16] = {
	fl_rgb_color(0xFF), fl_rgb_color(0xEE), fl_rgb_color(0xDD), fl_rgb_color(0xCC),
	fl_rgb_color(0xBB), fl_rgb_color(0xAA), fl_rgb_color(0x99), fl_rgb_color(0x88),
	fl_rgb_color(0x77), fl_rgb_color(0x66), fl_rgb_color(0x55), fl_rgb_color(0x44),
	fl_rgb_color(0x33), fl_rgb_color(0x22), fl_rgb_color(0x11), fl_rgb_color(0x00)
};

Tileset::Result Tileset::parse_4bpp_data(const std::vector<uchar> &data) {
	_num_tiles = data.size() / BYTES_PER_4BPP_TILE;

	int limit = (int)_num_tiles - _offset;
	if (_length > 0) { limit = std::min(limit, _length + _offset); }
	if (_start_id + limit > MAX_NUM_TILES) { return (_result = Result::TILESET_TOO_LARGE); }

	Fl_Image_Surface *surface = new Fl_Image_Surface(TILE_SIZE, (int)_num_tiles * TILE_SIZE);
	surface->set_current();

	_color_indices.assign(_num_tiles, {});
	for (size_t i = 0; i < _num_tiles; i++) {
		for (size_t j = 0; j < TILE_SIZE; j++) {
			int py = (int)(i * TILE_SIZE + j);
			Tile_Color_Indices &tile = _color_indices[i];
			for (int k = 0; k < TILE_SIZE / 2; k++) {
				uchar b = data[i * BYTES_PER_4BPP_TILE + j * TILE_SIZE / 2 + k];
				fl_color(bpp4_colors[LO_NYB(b)]);
				fl_point(k * 2, py);
				fl_color(bpp4_colors[HI_NYB(b)]);
				fl_point(k * 2 + 1, py);
				tile[j * TILE_SIZE + k * 2] = LO_NYB(b);
				tile[j * TILE_SIZE + k * 2 + 1] = HI_NYB(b);
			}
		}
	}
	_has_color_indices = true;

	Fl_RGB_Image *img = surface->image();
	delete surface;
	Fl_Display_Device::display_device()->set_current();

	return postprocess_graphics(img);
}

Tileset::Result Tileset::parse_8bpp_data(const std::vector<uchar> &data) {
	_num_tiles = data.size() / BYTES_PER_8BPP_TILE;

	int limit = (int)_num_tiles - _offset;
	if (_length > 0) { limit = std::min(limit, _length + _offset); }
	if (_start_id + limit > MAX_NUM_TILES) { return (_result = Result::TILESET_TOO_LARGE); }

	Fl_Image_Surface *surface = new Fl_Image_Surface(TILE_SIZE, (int)_num_tiles * TILE_SIZE);
	surface->set_current();

	_color_indices.assign(_num_tiles, {});
	for (size_t i = 0; i < _num_tiles; i++) {
		for (size_t j = 0; j < TILE_SIZE; j++) {
			int py = (int)(i * TILE_SIZE + j);
			for (int k = 0; k < TILE_SIZE; k++) {
				uchar b = data[i * BYTES_PER_8BPP_TILE + j * TILE_SIZE + k];
				fl_color(0xFF-b, 0xFF-b, 0xFF-b);
				fl_point(k, py);
				_color_indices[i][j * TILE_SIZE + k] = b;
			}
		}
	}
	_has_color_indices = true;

	Fl_RGB_Image *img = surface->image();
	delete surface;
	Fl_Display_Device::display_device()->set_current();

	return postprocess_graphics(img);
}

Tileset::Result Tileset::read_rgcn_graphics(const char *f) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return (_result = Result::TILESET_BAD_FILE); }

	// <https://www.romhacking.net/documents/%5B469%5Dnds_formats.htm#NCGR>
	// <https://github.com/pleonex/tinke/blob/master/Plugins/Images/Images/NCGR.cs>
	fseek(file, 16 + 4 + 4, SEEK_CUR); // skip generic header, "RAHC", sub-section size

	uint16_t th = read_uint16(file);
	uint16_t tw = read_uint16(file);
	int depth = fgetc(file);

	// Not all possible depth values can go with tilemaps
	// <https://github.com/pleonex/tinke/blob/master/Ekona/Images/Actions.cs#:~:text=ColorFormat>
	uint8_t bpp = 0;
	if (depth == 8) { bpp = BYTES_PER_1BPP_TILE; }
	else if (depth == 2) { bpp = BYTES_PER_2BPP_TILE; }
	else if (depth == 3) { bpp = BYTES_PER_4BPP_TILE; }
	else if (depth == 4) { bpp = BYTES_PER_8BPP_TILE; }
	else { fclose(file); return (_result = Result::TILESET_BAD_FILE); }

	fseek(file, 3 + 4 + 4 + 4 + 4, SEEK_CUR); // skip padding, tile form flag, tile data size, padding

	size_t n = tw * th * bpp;
	std::vector<uchar> data(n);
	size_t r = fread(data.data(), 1, n, file);
	fclose(file);
	if (r != n) { return (_result = Result::TILESET_BAD_FILE); }

	return bpp == BYTES_PER_2BPP_TILE ? parse_2bpp_data(data) : bpp == BYTES_PER_4BPP_TILE ? parse_4bpp_data(data) :
		bpp == BYTES_PER_8BPP_TILE ? parse_8bpp_data(data) : parse_1bpp_data(data);
}

Tileset::Result Tileset::read_rts_graphics(const char *f, bool skip_rmp) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return (_result = Result::TILESET_BAD_FILE); }

	if (skip_rmp) {
		size_t n = read_rmp_size(file);
		if (n == 0) { fclose(file); return (_result = Result::TILESET_BAD_FILE); }
		fseek(file, n, SEEK_CUR);
	}

	// <https://github.com/chadaustin/sphere/blob/master/sphere/docs/internal/tileset.rts.txt>
	uchar expected_header[6] = {
		'.', 'r', 't', 's', // magic number
		LE16(1),            // version
	};
	if (!check_read(file, expected_header, sizeof(expected_header))) { fclose(file); return (_result = Result::TILESET_BAD_FILE); }

	uint16_t nt = read_uint16(file);

	uchar expected_header_2[7] = {
		LE16(TILE_SIZE), // tile width
		LE16(TILE_SIZE), // tile height
		LE16(32),        // tile bpp
		0,               // compression
	};
	if (!check_read(file, expected_header_2, sizeof(expected_header_2))) { fclose(file); return (_result = Result::TILESET_BAD_FILE); }

	fseek(file, 1 + 240, SEEK_CUR); // skip 'has obstructions' and unused

	size_t n = nt * NUM_TILE_PIXELS * 4;
	uchar *bytes = new uchar[n]();
	if (fread(bytes, 1, n, file) != n) { delete [] bytes; fclose(file); return (_result = Result::TILESET_BAD_FILE); }

	fclose(file);

	Fl_RGB_Image *img = new Fl_RGB_Image(bytes, TILE_SIZE, nt * TILE_SIZE, 4);
	img->alloc_array = 1;

	return postprocess_graphics(img);
}

Tileset::Result Tileset::postprocess_graphics(Fl_RGB_Image *img) {
	if (!img || img->fail()) { return (_result = Result::TILESET_BAD_FILE); }

	_1x_image = img;
	_2x_image = (Fl_RGB_Image *)img->copy(img->w() * DEFAULT_ZOOM, img->h() * DEFAULT_ZOOM);
	if (!_2x_image || _2x_image->fail()) { clear(); return (_result = Result::TILESET_BAD_FILE); }
	update_zoom();
	if (!_zoomed_image || _zoomed_image->fail()) { clear(); return (_result = Result::TILESET_BAD_FILE); }

	int w = _1x_image->w(), h = _1x_image->h();
	if (w % TILE_SIZE || h % TILE_SIZE) { clear(); return (_result = Result::TILESET_BAD_DIMS); }

	w /= TILE_SIZE;
	h /= TILE_SIZE;
	_num_tiles = w * h;

	int limit = (int)_num_tiles - _offset;
	if (_length > 0) { limit = std::min(limit, _length + _offset); }
	if (_start_id + limit > MAX_NUM_TILES) { clear(); return (_result = Result::TILESET_TOO_LARGE); }

	return (_result = Result::TILESET_OK);
}

const char *Tileset::error_message(Result result) {
	switch (result) {
	case Result::TILESET_OK:
		return "OK.";
	case Result::TILESET_BAD_FILE:
		return "Cannot parse file format.";
	case Result::TILESET_BAD_EXT:
		return "Unknown file extension.";
	case Result::TILESET_BAD_DIMS:
		return "Image dimensions do not fit the tile grid.";
	case Result::TILESET_TOO_SHORT:
		return "Too few bytes.";
	case Result::TILESET_TOO_LARGE:
		return "Too many pixels.";
	case Result::TILESET_BAD_CMD:
		return "Invalid LZ command.";
	case Result::TILESET_NULL:
		return "No graphics file chosen.";
	default:
		return "Unspecified error.";
	}
}

// A rundown of Pokemon Crystal's LZ compression scheme:
enum class Lz_Command {
	// Control commands occupy bits 5-7.
	// Bits 0-4 serve as the first parameter n for each command.
	LZ_LITERAL,   // n values for n bytes
	LZ_ITERATE,   // one value for n bytes
	LZ_ALTERNATE, // alternate two values for n bytes
	LZ_BLANK,     // zero for n bytes
	// Repeater commands repeat any data that was just decompressed.
	// They take an additional signed parameter s to mark a relative starting point.
	// These wrap around (positive from the start, negative from the current position).
	LZ_REPEAT,    // n bytes starting from s
	LZ_FLIP,      // n bytes in reverse bit order starting from s
	LZ_REVERSE,   // n bytes backwards starting from s
	// The long command is used when 5 bits aren't enough. Bits 2-4 contain a new control code.
	// Bits 0-1 are appended to a new byte as 8-9, allowing a 10-bit parameter.
	LZ_LONG       // n is now 10 bits for a new control code
};

// If 0xff is encountered instead of a command, decompression ends.
#define LZ_END 0xff

static auto bit_flipped = ([]() constexpr {
	std::array<uchar, 256> a{};
	for (size_t i = 0; i < a.size(); i++) {
		for (size_t b = 0; b < 8; b++) {
			a[i] += ((i >> b) & 1) << (7 - b);
		}
	}
	return a;
})();

static Tileset::Result decompress_lz_data(const char *f, std::vector<uchar> &data) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return Tileset::Result::TILESET_BAD_FILE; }

	size_t n = file_size(file);
	std::vector<uchar> lz_data(n);
	size_t r = fread(lz_data.data(), 1, n, file);
	fclose(file);
	if (r != n) { return Tileset::Result::TILESET_BAD_FILE; }

	size_t len = 0;
	for (size_t address = 0, lim = data.size();;) {
		uchar q[2];
		int offset;
		uchar b = lz_data[address++];
		if (b == LZ_END) { break; }
		if (len >= lim) { return Tileset::Result::TILESET_TOO_LARGE; }
		Lz_Command cmd = (Lz_Command)((b & 0xe0) >> 5);
		int length = 0;
		if (cmd == Lz_Command::LZ_LONG) {
			cmd = (Lz_Command)((b & 0x1c) >> 2);
			length = (int)(b & 0x03) * 0x100;
			b = lz_data[address++];
			length += (int)b + 1;
		}
		else {
			length = (int)(b & 0x1f) + 1;
		}
		switch (cmd) {
		case Lz_Command::LZ_LITERAL:
			// Copy data directly.
			for (int i = 0; i < length; i++) {
				data[len++] = lz_data[address++];
			}
			break;
		case Lz_Command::LZ_ITERATE:
			// Write one byte repeatedly.
			b = lz_data[address++];
			for (int i = 0; i < length; i++) {
				data[len++] = b;
			}
			break;
		case Lz_Command::LZ_ALTERNATE:
			// Write alternating bytes.
			q[0] = lz_data[address++];
			q[1] = lz_data[address++];
			// Copy data directly.
			for (int i = 0; i < length; i++) {
				data[len++] = q[i & 1];
			}
			break;
		case Lz_Command::LZ_BLANK:
			// Write zeros.
			for (int i = 0; i < length; i++) {
				data[len++] = 0;
			}
			break;
		case Lz_Command::LZ_REPEAT:
			// Repeat bytes from output.
			b = lz_data[address++];
			offset = b >= 0x80 ? (int)len - (int)(b & 0x7f) - 1 : (int)b * 0x100 + lz_data[address++];
			for (int i = 0; i < length; i++) {
				data[len++] = data[offset + i];
			}
			break;
		case Lz_Command::LZ_FLIP:
			// Repeat flipped bytes from output.
			b = lz_data[address++];
			offset = b >= 0x80 ? (int)len - (int)(b & 0x7f) - 1 : (int)b * 0x100 + lz_data[address++];
			for (int i = 0; i < length; i++) {
				b = data[offset + i];
				data[len++] = bit_flipped[b];
			}
			break;
		case Lz_Command::LZ_REVERSE:
			// Repeat reversed bytes from output.
			b = lz_data[address++];
			offset = b >= 0x80 ? (int)len - (int)(b & 0x7f) - 1 : (int)b * 0x100 + lz_data[address++];
			for (int i = 0; i < length; i++) {
				data[len++] = data[offset - i];
			}
			break;
		case Lz_Command::LZ_LONG:
		default:
			return Tileset::Result::TILESET_BAD_CMD;
		}
	}

	data.resize(len);
	return Tileset::Result::TILESET_OK;
}
