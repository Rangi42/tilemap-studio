#include <zlib.h>

#include "themes.h"
#include "main-window.h"
#include "tile-selection.h"
#include "tile-buttons.h"

static const Fl_Color palette_colors[MAX_NUM_PALETTES] = {
	fl_rgb_color(0xA0, 0xB0, 0xC0), fl_rgb_color(0xE6, 0x19, 0x4B),
	fl_rgb_color(0x8A, 0xE8, 0x17), fl_rgb_color(0x2B, 0x95, 0xFF),
	fl_rgb_color(0xFF, 0xE1, 0x19), fl_rgb_color(0xF5, 0x82, 0x31),
	fl_rgb_color(0x43, 0xF2, 0xF2), fl_rgb_color(0xDF, 0x32, 0xEF),
	fl_rgb_color(0x50, 0x60, 0x70), fl_rgb_color(0x8C, 0x0B, 0x00),
	fl_rgb_color(0x18, 0xB2, 0x2A), fl_rgb_color(0x43, 0x00, 0xCC),
	fl_rgb_color(0xFF, 0x77, 0xA8), fl_rgb_color(0x9A, 0x63, 0x24),
	fl_rgb_color(0x46, 0x99, 0x90), fl_rgb_color(0x7A, 0x1A, 0x5D)
};

static const uchar palette_digits_png_buffer[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x07, 0x04, 0x03, 0x00, 0x00, 0x00, 0x6d, 0xb4, 0xd7,
	0xd1, 0x00, 0x00, 0x00, 0x30, 0x50, 0x4c, 0x54, 0x45, 0xff, 0xff, 0xff, 0x18, 0xb2, 0x2a, 0x2b,
	0x95, 0xff, 0x31, 0xb2, 0xb2, 0x43, 0x00, 0xcc, 0x46, 0x99, 0x90, 0x50, 0x60, 0x70, 0x7a, 0x1a,
	0x5d, 0x8c, 0x0b, 0x00, 0x9a, 0x63, 0x24, 0xaa, 0x96, 0x11, 0xb2, 0x53, 0x76, 0xbb, 0x2a, 0xcc,
	0xcc, 0x6a, 0x28, 0xe6, 0x19, 0x4b, 0xff, 0xff, 0xff, 0x3d, 0xcc, 0x73, 0x6e, 0x00, 0x00, 0x00,
	0x01, 0x74, 0x52, 0x4e, 0x53, 0x00, 0x40, 0xe6, 0xd8, 0x66, 0x00, 0x00, 0x00, 0xaf, 0x49, 0x44,
	0x41, 0x54, 0x78, 0x5e, 0x6d, 0x8f, 0x31, 0x0e, 0x82, 0x40, 0x10, 0x45, 0xf7, 0x06, 0x1b, 0x4e,
	0x60, 0xe2, 0x05, 0x28, 0xbc, 0x02, 0x3d, 0x95, 0xb5, 0x1d, 0x57, 0xa0, 0xa4, 0xb5, 0xa3, 0xb6,
	0xa2, 0xf7, 0x0a, 0x5e, 0xc1, 0x84, 0x13, 0x18, 0x6e, 0xa0, 0xb3, 0x6e, 0xd8, 0x62, 0x21, 0x3c,
	0x27, 0x60, 0x20, 0x31, 0xfe, 0x69, 0x26, 0x7f, 0x7e, 0x5e, 0xfe, 0x18, 0x00, 0xe1, 0x8f, 0xac,
	0x8e, 0xd8, 0xf5, 0x6a, 0x42, 0xc1, 0x53, 0x5c, 0xc2, 0x3e, 0x8d, 0x91, 0x47, 0xe7, 0x0f, 0xdc,
	0x5b, 0xf5, 0xce, 0x35, 0x3b, 0x32, 0xb8, 0x71, 0xa1, 0x3f, 0x52, 0x56, 0x26, 0x04, 0x26, 0xc1,
	0x81, 0x06, 0xe9, 0xf0, 0x40, 0xab, 0x5e, 0x5d, 0x3b, 0x47, 0x9e, 0x0f, 0xd0, 0x34, 0x3d, 0x54,
	0x68, 0x50, 0x26, 0x25, 0xca, 0x3b, 0x8d, 0xd7, 0x99, 0x28, 0x2c, 0x44, 0x97, 0x90, 0x31, 0x88,
	0xd5, 0xe0, 0xc9, 0x96, 0x32, 0x13, 0x71, 0x40, 0x4a, 0x84, 0xce, 0x7b, 0x19, 0x51, 0x8f, 0x1f,
	0xa2, 0xcc, 0x1d, 0x5f, 0x73, 0x47, 0x13, 0x17, 0xe2, 0x28, 0xa1, 0x10, 0xab, 0xc1, 0xad, 0x23,
	0x95, 0x31, 0xac, 0xfa, 0xae, 0x82, 0xb0, 0xbc, 0xcb, 0xf6, 0x35, 0xe6, 0x03, 0xcf, 0xa4, 0xe5,
	0x1a, 0x38, 0x7f, 0xdd, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
	0x82
};

static Fl_PNG_Image palette_digits_image(NULL, palette_digits_png_buffer, sizeof(palette_digits_png_buffer));

static const uchar priority_png_buffer[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07, 0x02, 0x03, 0x00, 0x00, 0x00, 0xb9, 0x3c, 0xbf,
	0x40, 0x00, 0x00, 0x00, 0x09, 0x50, 0x4c, 0x54, 0x45, 0xff, 0xff, 0xff, 0xf8, 0x80, 0x08, 0xff,
	0xff, 0xff, 0xf3, 0x7c, 0x8c, 0x46, 0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x00, 0x40,
	0xe6, 0xd8, 0x66, 0x00, 0x00, 0x00, 0x1d, 0x49, 0x44, 0x41, 0x54, 0x78, 0x5e, 0x63, 0x60, 0x62,
	0x60, 0xe0, 0x6c, 0x60, 0x58, 0x9a, 0xc1, 0x30, 0x35, 0x82, 0x41, 0x35, 0x81, 0x41, 0x2d, 0x81,
	0x41, 0x63, 0x01, 0x00, 0x27, 0xf1, 0x04, 0x59, 0x6f, 0x13, 0x1c, 0xb0, 0x00, 0x00, 0x00, 0x00,
	0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static Fl_PNG_Image priority_image(NULL, priority_png_buffer, sizeof(priority_png_buffer));

static const uchar obp1_png_buffer[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07, 0x02, 0x03, 0x00, 0x00, 0x00, 0xb9, 0x3c, 0xbf,
	0x40, 0x00, 0x00, 0x00, 0x09, 0x50, 0x4c, 0x54, 0x45, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
	0xd6, 0xd6, 0x36, 0x26, 0xd6, 0x94, 0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x00, 0x40,
	0xe6, 0xd8, 0x66, 0x00, 0x00, 0x00, 0x19, 0x49, 0x44, 0x41, 0x54, 0x78, 0x5e, 0x63, 0x00, 0x02,
	0xd1, 0x00, 0x06, 0xa9, 0x09, 0x0c, 0x92, 0x13, 0x80, 0x24, 0x90, 0xcd, 0xc0, 0xc0, 0x00, 0x00,
	0x1c, 0x52, 0x02, 0xc8, 0x8d, 0xf6, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
	0xae, 0x42, 0x60, 0x82
};

static Fl_PNG_Image obp1_image(NULL, obp1_png_buffer, sizeof(obp1_png_buffer));

static const uchar swatch_backdrop_png_buffer[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x02, 0x03, 0x00, 0x00, 0x00, 0x62, 0x9d, 0x17,
	0xf2, 0x00, 0x00, 0x00, 0x0c, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0xaa,
	0xaa, 0xaa, 0xff, 0xff, 0xff, 0xc1, 0x7f, 0x62, 0xd1, 0x00, 0x00, 0x00, 0x16, 0x49, 0x44, 0x41,
	0x54, 0x78, 0x5e, 0x63, 0xf8, 0xbf, 0xea, 0xff, 0x2a, 0x74, 0x22, 0x14, 0x08, 0xd1, 0x08, 0x62,
	0xd5, 0x01, 0x00, 0xd5, 0x6b, 0x1f, 0xe1, 0x2c, 0xfd, 0xb0, 0x6a, 0x00, 0x00, 0x00, 0x00, 0x49,
	0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static Fl_PNG_Image swatch_backdrop_image(NULL, swatch_backdrop_png_buffer, sizeof(swatch_backdrop_png_buffer));

static const Fl_Color rainbow_bg_colors[16] = {
	fl_rgb_color(0x12, 0x34, 0x56), fl_rgb_color(0x12, 0x1B, 0x56), fl_rgb_color(0x23, 0x12, 0x56), fl_rgb_color(0x3C, 0x12, 0x56),
	fl_rgb_color(0x56, 0x12, 0x56), fl_rgb_color(0x56, 0x12, 0x3D), fl_rgb_color(0x56, 0x12, 0x23), fl_rgb_color(0x56, 0x1A, 0x12),
	fl_rgb_color(0x56, 0x34, 0x12), fl_rgb_color(0x56, 0x4D, 0x12), fl_rgb_color(0x45, 0x56, 0x12), fl_rgb_color(0x2C, 0x56, 0x12),
	fl_rgb_color(0x12, 0x56, 0x12), fl_rgb_color(0x12, 0x56, 0x2B), fl_rgb_color(0x12, 0x56, 0x45), fl_rgb_color(0x12, 0x4E, 0x56),
};
static const Fl_Color rainbow_fg_colors[16] = {
	fl_rgb_color(0xAB, 0xCD, 0xEF), fl_rgb_color(0xAB, 0xB4, 0xEF), fl_rgb_color(0xBC, 0xAB, 0xEF), fl_rgb_color(0xD5, 0xAB, 0xEF),
	fl_rgb_color(0xEF, 0xAB, 0xEF), fl_rgb_color(0xEF, 0xAB, 0xD6), fl_rgb_color(0xEF, 0xAB, 0xBC), fl_rgb_color(0xEF, 0xB3, 0xAB),
	fl_rgb_color(0xEF, 0xCD, 0xAB), fl_rgb_color(0xEF, 0xE6, 0xAB), fl_rgb_color(0xDE, 0xEF, 0xAB), fl_rgb_color(0xC5, 0xEF, 0xAB),
	fl_rgb_color(0xAB, 0xEF, 0xAB), fl_rgb_color(0xAB, 0xEF, 0xC4), fl_rgb_color(0xAB, 0xEF, 0xDE), fl_rgb_color(0xAB, 0xE7, 0xEF),
};

static void draw_grid(int x, int y, int z = DEFAULT_ZOOM) {
	int s = TILE_SIZE * z;
	fl_color(fl_rgb_color(0x40));
	fl_xyline(x, y+s-1, x+s-1, y);
	fl_color(fl_rgb_color(0xD0));
	char dashes[] = {2, 2, 0};
	fl_line_style(FL_DASH, 0, dashes);
	fl_xyline(x, y+s-1, x+s-1);
	fl_yxline(x+s-1, y, y+s-1);
	fl_line_style(FL_SOLID);
}

static void draw_highlight(int x, int y, int z = DEFAULT_ZOOM) {
	int s = TILE_SIZE * z;
	fl_rect(x, y, s, s, FL_DARK_YELLOW);
	fl_rect(x+1, y+1, s-2, s-2, FL_YELLOW);
}

static void draw_selection_border(int x, int y, int z = DEFAULT_ZOOM, bool highlighted = false) {
	int s = TILE_SIZE * z;
	Fl_Color c = highlighted ? FL_YELLOW : FL_WHITE;
	draw_selection_border(x, y, s, s, c, z > 5);
}

void draw_selection_border(int x, int y, int w, int h, Fl_Color c, bool zoom) {
	fl_rect(x, y, w, h, FL_BLACK);
	fl_rect(x+1, y+1, w-2, h-2, c);
	if (zoom) {
		fl_rect(x+2, y+2, w-4, h-4, c);
		fl_rect(x+3, y+3, w-6, h-6, FL_BLACK);
	}
	else {
		fl_rect(x+2, y+2, w-4, h-4, FL_BLACK);
	}
}

std::vector<Tileset> *Tile_State::_tilesets = NULL;

Fl_PNG_Image *Tile_State::_palette_bgs_image = NULL;

void Tile_State::alpha(uchar alfa) {
	const uchar trns_data[20] = {
		0x74, 0x52, 0x4e, 0x53,
		alfa, alfa, alfa, alfa, alfa, alfa, alfa, alfa,
		alfa, alfa, alfa, alfa, alfa, alfa, alfa, alfa
	};
	uLong trns_crc = crc32(0L, Z_NULL, 0);
	trns_crc = crc32(trns_crc, trns_data, sizeof(trns_data));
	uchar crc[4] = {BE32(trns_crc)};
	auto [crc1, crc2, crc3, crc4] = crc;
	const uchar palette_bgs_png_buffer[] = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
		0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x50, 0x04, 0x03, 0x00, 0x00, 0x00, 0xb4, 0x7c, 0xc5,
		0x94, 0x00, 0x00, 0x00, 0x30, 0x50, 0x4c, 0x54, 0x45, 0x18, 0xb2, 0x2a, 0x2b, 0x95, 0xff, 0x43,
		0x00, 0xcc, 0x43, 0xf2, 0xf2, 0x46, 0x99, 0x90, 0x50, 0x60, 0x70, 0x7a, 0x1a, 0x5d, 0x8a, 0xe8,
		0x17, 0x8c, 0x0b, 0x00, 0x9a, 0x63, 0x24, 0xa0, 0xb0, 0xc0, 0xdf, 0x32, 0xef, 0xe6, 0x19, 0x4b,
		0xf5, 0x82, 0x31, 0xff, 0x77, 0xa8, 0xff, 0xe1, 0x19, 0xbc, 0xc9, 0x5c, 0x8d, 0x00, 0x00, 0x00,
		0x10, 0x74, 0x52, 0x4e, 0x53, alfa, alfa, alfa, alfa, alfa, alfa, alfa, alfa, alfa, alfa, alfa,
		alfa, alfa, alfa, alfa, alfa, crc1, crc2, crc3, crc4, 0x00, 0x00, 0x00, 0xca, 0x49, 0x44, 0x41,
		0x54, 0x78, 0xda, 0xed, 0xd2, 0x51, 0x0d, 0x82, 0x00, 0x00, 0x40, 0x41, 0x46, 0x03, 0x2a, 0x50,
		0xc1, 0x0a, 0x56, 0xa0, 0x02, 0x15, 0xa8, 0x40, 0x05, 0x2a, 0x58, 0xc1, 0x0a, 0x54, 0xb0, 0x02,
		0x15, 0xb4, 0xc2, 0xfb, 0x63, 0x73, 0x77, 0x19, 0x6e, 0x7c, 0x45, 0x67, 0xb4, 0x45, 0x53, 0xf4,
		0x8d, 0x3e, 0xd1, 0x23, 0x7a, 0x47, 0x4b, 0xb4, 0x47, 0x43, 0x34, 0x47, 0x57, 0x74, 0x44, 0xcf,
		0x68, 0x8d, 0xc6, 0x01, 0x6e, 0x24, 0x20, 0x02, 0x22, 0x20, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02,
		0x82, 0x80, 0x08, 0x08, 0x02, 0x22, 0x20, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82, 0x80, 0x08,
		0x08, 0x02, 0x22, 0x20, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82, 0x80, 0x08, 0x08, 0x02, 0x22,
		0x20, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82, 0x80, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82,
		0x80, 0x08, 0x08, 0x02, 0x22, 0x20, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82, 0x80, 0x08, 0x08,
		0x02, 0x22, 0x20, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82, 0x80, 0x08, 0x08, 0x02, 0x22, 0x20,
		0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82, 0x80, 0x08, 0x08, 0x02, 0x22, 0x20, 0x08, 0x88, 0x80,
		0x08, 0x08, 0x02, 0x22, 0x20, 0x08, 0x88, 0x80, 0x20, 0x20, 0x02, 0x82, 0x80, 0x08, 0x08, 0x02,
		0x22, 0x20, 0x08, 0xc8, 0x3f, 0xf9, 0x01, 0xf4, 0x34, 0x3f, 0x70, 0x2b, 0xad, 0x13, 0x5a, 0x00,
		0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
	};
	delete _palette_bgs_image;
	_palette_bgs_image = new Fl_PNG_Image(NULL, palette_bgs_png_buffer, sizeof(palette_bgs_png_buffer));
}

void Tile_State::update_zoom() {
	if (!_tilesets) { return; }
	for (Tileset &t : *_tilesets) {
		t.update_zoom();
	}
}

static Fl_Font tile_fonts[4] = {FL_COURIER, FL_COURIER_ITALIC, FL_COURIER_BOLD, FL_COURIER_BOLD_ITALIC};

void Tile_State::draw_tile(int x, int y, int z, bool active, bool selected) {
	if (z == 1) {
		draw_tile_1x(x, y, active, selected);
		return;
	}
	if (_tilesets) {
		for (std::vector<Tileset>::reverse_iterator it = _tilesets->rbegin(); it != _tilesets->rend(); ++it) {
			if (it->draw_tile(this, x, y, z, active)) {
				return;
			}
		}
	}
	uint16_t hi = HI_NYB(id), lo = LO_NYB(id), bank = (id & 0x300) >> 8;
	char l1 = (char)(hi > 9 ? 'A' + hi - 10 : '0' + hi), l2 = (char)(lo > 9 ? 'A' + lo - 10 : '0' + lo);
	const char buffer[] = {l1, l2, '\0'};
	if (bank & 1) {
		hi ^= 8;
		lo ^= 8;
	}
	bool r = Config::rainbow_tiles();
	Fl_Color bg = rainbow_bg_colors[r ? lo : 0];
	int s = TILE_SIZE * z;
	fl_rectf(x, y, s, s, bg);
	int f = (OS::is_consolas() ? 11 : 10) + z * 2 - 4;
	fl_font(tile_fonts[bank], f);
	Fl_Color fg = selected ? FL_YELLOW : x_flip ? y_flip ? FL_YELLOW : FL_MAGENTA : y_flip ? FL_CYAN : rainbow_fg_colors[r ? hi : 0];
	fl_color(fg);
	fl_draw(buffer, x, y, s, s, FL_ALIGN_CENTER);
}

void Tile_State::draw_attributes(int x, int y, int z, int style, bool active) {
	int s = TILE_SIZE * z;
	if (!active) {
		fl_rectf(x, y, s, s, FL_INACTIVE_COLOR);
	}
	else if (palette > -1) {
		if (style > 0) {
			_palette_bgs_image->draw(x, y, s, s, TILE_SIZE * MAX_ZOOM * palette, 0);
		}
		else if (style < 0) {
			fl_rectf(x, y, s, s, palette_colors[palette]);
		}
		int dy = z > 1 || !Config::grid();
		palette_digits_image.draw(x+1, y+dy, 5, 7, 5 * palette, 0);
	}
	if (z > 1) {
		if (priority) {
			priority_image.draw(x+8, y+8);
		}
		if (obp1) {
			obp1_image.draw(x+8, y+1);
		}
	}
}

// {length*2, X,Y, ...}
static const int digit_pixels[16][1+13*2] = {
	{10*2, 1,0, 2,0, 0,1, 2,1, 0,2, 2,2, 0,3, 2,3, 0,4, 1,4},                // 0
	{ 8*2, 1,0, 0,1, 1,1, 1,2, 1,3, 0,4, 1,4, 2,4},                          // 1
	{ 8*2, 0,0, 1,0, 2,1, 1,2, 0,3, 0,4, 1,4, 2,4},                          // 2
	{ 7*2, 0,0, 1,0, 2,1, 1,2, 2,3, 0,4, 1,4},                               // 3
	{ 9*2, 0,0, 2,0, 0,1, 2,1, 0,2, 1,2, 2,2, 2,3, 2,4},                     // 4
	{ 9*2, 0,0, 1,0, 2,0, 0,1, 0,2, 1,2, 2,3, 0,4, 1,4},                     // 5
	{11*2, 1,0, 2,0, 0,1, 0,2, 1,2, 2,2, 0,3, 2,3, 0,4, 1,4, 2,4},           // 6
	{ 7*2, 0,0, 1,0, 2,0, 2,1, 2,2, 1,3, 1,4},                               // 7
	{13*2, 0,0, 1,0, 2,0, 0,1, 2,1, 0,2, 1,2, 2,2, 0,3, 2,3, 0,4, 1,4, 2,4}, // 8
	{11*2, 0,0, 1,0, 2,0, 0,1, 2,1, 0,2, 1,2, 2,2, 2,3, 0,4, 1,4},           // 9
	{10*2, 1,0, 0,1, 2,1, 0,2, 1,2, 2,2, 0,3, 2,3, 0,4, 2,4},                // A
	{10*2, 0,0, 1,0, 0,1, 2,1, 0,2, 1,2, 0,3, 2,3, 0,4, 1,4},                // B
	{ 7*2, 1,0, 2,0, 0,1, 0,2, 0,3, 1,4, 2,4},                               // C
	{10*2, 0,0, 1,0, 0,1, 2,1, 0,2, 2,2, 0,3, 2,3, 0,4, 1,4},                // D
	{11*2, 0,0, 1,0, 2,0, 0,1, 0,2, 1,2, 2,2, 0,3, 0,4, 1,4, 2,4},           // E
	{ 9*2, 0,0, 1,0, 2,0, 0,1, 0,2, 1,2, 2,2, 0,3, 0,4},                     // F
};

static void print_digit(int x, int y, uchar d) {
	const int *pixels = digit_pixels[d];
	int n = pixels[0];
	for (int i = 1; i <= n; i += 2) {
		int dx = pixels[i], dy = pixels[i+1];
		fl_point(x + dx, y + dy);
	}
}

void Tile_State::draw(int x, int y, int z, bool tile, bool attr, int style, bool active, bool selected) {
	int s = TILE_SIZE * z;
	if (tile) {
		draw_tile(x, y, z, active, selected);
	}
	else if (!attr) {
		fl_rectf(x, y, s, s, FL_WHITE);
	}
	if (attr) {
		draw_attributes(x, y, z, style, active);
	}
}

void Tile_State::draw_tile_1x(int x, int y, bool active, bool selected) {
	if (_tilesets) {
		for (std::vector<Tileset>::reverse_iterator it = _tilesets->rbegin(); it != _tilesets->rend(); ++it) {
			if (it->print_tile(this, x, y, active)) {
				return;
			}
		}
	}
	uchar hi = HI_NYB(id), lo = LO_NYB(id);
	bool r = Config::rainbow_tiles();
	Fl_Color bg = rainbow_bg_colors[r ? lo : 0];
	fl_rectf(x, y, TILE_SIZE, TILE_SIZE, bg);
	Fl_Color fg = selected ? FL_YELLOW : x_flip ? y_flip ? FL_YELLOW : FL_MAGENTA : y_flip ? FL_CYAN : rainbow_fg_colors[r ? hi : 0];
	fl_color(fg);
	print_digit(x, y+1, hi);
	print_digit(x+4, y+2, lo);
}

void Tile_State::print(int x, int y, bool active, bool selected, int palette_) {
	bool drawn = false;
	if (_tilesets) {
		for (std::vector<Tileset>::reverse_iterator it = _tilesets->rbegin(); it != _tilesets->rend(); ++it) {
			if (it->print_tile(this, x, y, active)) {
				drawn = true;
				break;
			}
		}
	}
	if (!drawn) {
		uchar hi = HI_NYB(id), lo = LO_NYB(id);
		bool r = Config::print_rainbow_tiles();
		Fl_Color bg = rainbow_bg_colors[r ? lo : 0];
		fl_rectf(x, y, TILE_SIZE, TILE_SIZE, bg);
		Fl_Color fg = selected ? FL_YELLOW : x_flip ? y_flip ? FL_YELLOW : FL_MAGENTA : y_flip ? FL_CYAN : rainbow_fg_colors[r ? hi : 0];
		fl_color(fg);
		print_digit(x, y+1, hi);
		print_digit(x+4, y+2, lo);
	}
	if (Config::print_grid()) {
		draw_grid(x, y, 1);
	}
	if (palette_ > -1) {
		if (Config::print_bold_palettes()) {
			_palette_bgs_image->draw(x, y, TILE_SIZE, TILE_SIZE, TILE_SIZE * MAX_ZOOM * palette_, 0);
		}
		if (Config::print_palettes()) {
			int dy = !Config::print_grid();
			palette_digits_image.draw(x+1, y+dy, 5, 7, 5 * palette_, 0);
		}
	}
}

void Tile_Thing::shift_id(int d, int n) {
	while (d < 0) {
		d += n;
	}
	_state.id = (uint16_t)((_state.id + d) % n);
}

Tile_Swatch::Tile_Swatch(int x, int y, int w, int h) : Tile_Thing(), Fl_Box(x, y, w, h), _attributes() {
	user_data(NULL);
	box(OS_SPACER_THIN_DOWN_FRAME);
	labeltype(FL_NO_LABEL);
}

void Tile_Swatch::draw() {
	draw_box();
	int ox = x() + Fl::box_dx(box()), oy = y() + Fl::box_dy(box());
	swatch_backdrop_image.draw(ox, oy);
	_state.draw(ox, oy, DEFAULT_ZOOM, !_attributes, _attributes, (int)Config::bold_palettes(), !!active(), false);
}

Tile_Tessera::Tile_Tessera(int x, int y, size_t row, size_t col, uint16_t id, bool x_flip, bool y_flip,
	bool priority, bool obp1, int palette) : Groupable(x, y, row, col, id, x_flip, y_flip, priority, obp1, palette) {
	user_data(NULL);
	box(FL_NO_BOX);
	labeltype(FL_NO_LABEL);
	type(TILE_TESSERA_TYPE);
}

void Tile_Tessera::draw() {
	Main_Window *mw = (Main_Window *)user_data();
	int X = x(), Y = y(), Z = Config::zoom();
	_state.draw(X, Y, Z, true, Config::show_attributes(), (int)Config::bold_palettes(), !!active(), false);
	if (Config::grid()) {
		draw_grid(X, Y, Z);
	}
	if (_state.highlighted()) {
		draw_highlight(X, Y, Z);
	}
	if (this == Fl::belowmouse() && !mw->selection().selected_multiple()) {
		draw_selection_border(X, Y, Z, _state.highlighted());
	}
}

static bool pushed_in_tileset = false;

int Tile_Tessera::handle(int event) {
	Main_Window *mw = (Main_Window *)user_data();
	Tile_Selection &ts = mw->selection();
	switch (event) {
	case FL_ENTER:
		if (ts.selecting() && !ts.from_tileset()) {
			if (Fl::event_button3()) {
				ts.continue_selecting(this);
				mw->update_selection_status();
				mw->redraw_overlay();
			}
			else {
				ts.finish_selecting();
				mw->update_selection_controls();
			}
		}
		if ((Fl::event_button1() || Fl::event_button3()) && !Fl::pushed()) {
			Fl::pushed(this);
			if (Fl::event_button1() && !ts.selecting()) {
				do_callback();
			}
		}
		mw->update_status(this);
		redraw();
		return 1;
	case FL_LEAVE:
		if (ts.selecting() && !pushed_in_tileset) {
			ts.continue_selecting(NULL);
		}
		mw->update_status(NULL);
		redraw();
		return 1;
	case FL_MOVE:
		return 1;
	case FL_PUSH:
		pushed_in_tileset = false;
		mw->map_editable(true);
		do_callback();
		return 1;
	case FL_RELEASE:
		mw->map_editable(false);
		if (ts.selecting() && !ts.from_tileset()) {
			ts.finish_selecting();
			if (ts.selected_multiple()) {
				mw->clear_flips();
			}
			mw->update_selection_status();
			mw->update_selection_controls();
		}
		return 1;
	case FL_DRAG:
		if (!Fl::event_inside(x(), y(), w(), h())) {
			Fl::pushed(NULL);
		}
		if (Fl::event_button3() && !ts.selecting() && !pushed_in_tileset) {
			ts.start_selecting(this);
			mw->redraw_overlay();
		}
		return 1;
	}
	return 0;
}

Tile_Button::Tile_Button(int x, int y, size_t row, size_t col, uint16_t id) : Groupable(x, y, row, col, id),
	_value(), _old_value() {
	user_data(NULL);
	box(FL_NO_BOX);
	labeltype(FL_NO_LABEL);
	type(FL_RADIO_BUTTON);
}

int Tile_Button::value(char v) {
	// Based on Fl_Radio_Button::value()
	_old_value = v ? 1 : 0;
	clear_changed();
	if (_value != _old_value) {
		_value = _old_value;
		redraw();
		return 1;
	}
	return 0;
}

void Tile_Button::setonly() {
	// Based on Fl_Radio_Button::setonly()
	set();
	Fl_Group *g = parent();
	Fl_Widget * const *a = g->array();
	for (int i = g->children(); i--;) {
		Fl_Widget *o = *a++;
		if (o != this && o->type() == FL_RADIO_BUTTON) {
			((Tile_Button *)o)->clear();
		}
	}
}

void Tile_Button::draw() {
	Main_Window *mw = (Main_Window *)user_data();
	int X = x(), Y = y();
	bool multi = mw->selection().selected_multiple();
	_state.draw(X, Y, DEFAULT_ZOOM, true, Config::show_attributes(), (int)Config::bold_palettes(), !!active(), !!value() && !multi);
	if (Config::grid()) {
		draw_grid(X, Y);
	}
	if (_state.highlighted()) {
		draw_highlight(X, Y);
	}
	if (value() && !multi) {
		draw_selection_border(X, Y, DEFAULT_ZOOM, _state.highlighted());
	}
}

int Tile_Button::handle(int event) {
	// Based on Fl_Button::handle()
	char new_value;
	Main_Window *mw = (Main_Window *)user_data();
	Tile_Selection &ts = mw->selection();
	switch (event) {
	case FL_ENTER:
		// Don't interfere with dragging onto the parent Droppable|Workspace
		if (mw->dropping()) { return 0; }
		if (ts.selecting() && ts.from_tileset()) {
			if (Fl::event_button1()) {
				ts.continue_selecting(this);
				mw->update_selection_status();
				mw->redraw_overlay();
			}
			else {
				ts.finish_selecting();
				mw->update_selection_controls();
			}
		}
		if ((Fl::event_button1() || Fl::event_button3()) && !Fl::pushed()) {
			Fl::pushed(this);
		}
		return 1;
	case FL_LEAVE:
		if (ts.selecting() && pushed_in_tileset) {
			ts.continue_selecting(NULL);
		}
		redraw();
		return 1;
	case FL_MOVE:
		return 1;
	case FL_PUSH:
		pushed_in_tileset = true;
		// Don't change the value on right-click
		if (Fl::event_button() == FL_RIGHT_MOUSE) {
			return 1;
		}
		if (Fl::event_inside(this)) {
			new_value = 1;
		}
		else {
			clear_changed();
			new_value = _old_value;
		}
		if (new_value != _value) {
			_value = new_value;
			set_changed();
			redraw();
			do_callback();
		}
		return 1;
	case FL_RELEASE:
		if (_value != _old_value) {
			set_changed();
			setonly();
		}
		if (!ts.selecting()) {
			do_callback();
		}
		if (ts.selecting() && ts.from_tileset()) {
			ts.finish_selecting();
			mw->update_selection_status();
			mw->update_selection_controls();
		}
		return 1;
	case FL_DRAG:
		if (!Fl::event_inside(x(), y(), w(), h())) {
			Fl::pushed(NULL);
		}
		if (Fl::event_button1() && !ts.selecting() && pushed_in_tileset) {
			ts.start_selecting(this);
			mw->redraw_overlay();
		}
		return 1;
	default:
		return 0;
	}
}

Palette_Button::Palette_Button(int x, int y, int p) : Tile_Thing(0x000, false, false, false, false, p),
	Fl_Radio_Button(x, y, TILE_SIZE_2X, TILE_SIZE_2X) {
	user_data(NULL);
	box(FL_NO_BOX);
	labeltype(FL_NO_LABEL);
	when(FL_WHEN_RELEASE);
}

void Palette_Button::draw() {
	int X = x(), Y = y();
	_state.draw(X, Y, DEFAULT_ZOOM, false, true, -1, !!active(), !!value());
	if (Config::grid()) {
		draw_grid(X, Y);
	}
	if (value()) {
		draw_selection_border(X, Y);
	}
}

int Palette_Button::handle(int event) {
	// Don't interfere with dragging onto the parent Droppable|Workpane
	return event == FL_ENTER || event == FL_LEAVE || event == FL_DRAG ? 0 : Fl_Radio_Button::handle(event);
}
