#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>
#pragma warning(pop)

#include "themes.h"
#include "main-window.h"
#include "tile-buttons.h"

static const uchar palette0_png_buffer[96] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0x3d, 0x6d,
	0x22, 0x00, 0x00, 0x00, 0x03, 0x50, 0x4c, 0x54, 0x45, 0x80, 0x80, 0x80, 0x90, 0x74, 0x3d, 0x31,
	0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x20, 0x7b, 0x88, 0xf8, 0xae, 0x00, 0x00, 0x00,
	0x0b, 0x49, 0x44, 0x41, 0x54, 0x78, 0x5e, 0x63, 0x20, 0x11, 0x00, 0x00, 0x00, 0x30, 0x00, 0x01,
	0x81, 0x48, 0xa6, 0x44, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

static const uchar palette1_png_buffer[96] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0x3d, 0x6d,
	0x22, 0x00, 0x00, 0x00, 0x03, 0x50, 0x4c, 0x54, 0x45, 0xee, 0x00, 0x00, 0x04, 0x06, 0xc0, 0x70,
	0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x20, 0x7b, 0x88, 0xf8, 0xae, 0x00, 0x00, 0x00,
	0x0b, 0x49, 0x44, 0x41, 0x54, 0x78, 0x5e, 0x63, 0x20, 0x11, 0x00, 0x00, 0x00, 0x30, 0x00, 0x01,
	0x81, 0x48, 0xa6, 0x44, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

static const uchar palette2_png_buffer[96] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0x3d, 0x6d,
	0x22, 0x00, 0x00, 0x00, 0x03, 0x50, 0x4c, 0x54, 0x45, 0x00, 0xa3, 0x00, 0x22, 0x50, 0xd2, 0xf0,
	0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x20, 0x7b, 0x88, 0xf8, 0xae, 0x00, 0x00, 0x00,
	0x0b, 0x49, 0x44, 0x41, 0x54, 0x78, 0x5e, 0x63, 0x20, 0x11, 0x00, 0x00, 0x00, 0x30, 0x00, 0x01,
	0x81, 0x48, 0xa6, 0x44, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

static const uchar palette3_png_buffer[96] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0x3d, 0x6d,
	0x22, 0x00, 0x00, 0x00, 0x03, 0x50, 0x4c, 0x54, 0x45, 0x43, 0x43, 0xff, 0x23, 0xf0, 0xfd, 0x08,
	0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x20, 0x7b, 0x88, 0xf8, 0xae, 0x00, 0x00, 0x00,
	0x0b, 0x49, 0x44, 0x41, 0x54, 0x78, 0x5e, 0x63, 0x20, 0x11, 0x00, 0x00, 0x00, 0x30, 0x00, 0x01,
	0x81, 0x48, 0xa6, 0x44, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

static Fl_PNG_Image palette_images[NUM_SGB_PALETTES] = {
	{NULL, palette0_png_buffer, 96},
	{NULL, palette1_png_buffer, 96},
	{NULL, palette2_png_buffer, 96},
	{NULL, palette3_png_buffer, 96},
};

static const Fl_Color palettes[NUM_SGB_PALETTES] = {
	fl_rgb_color(0x20, 0x20, 0x20),
	fl_rgb_color(0xee, 0x00, 0x00),
	fl_rgb_color(0x00, 0xa3, 0x00),
	fl_rgb_color(0x43, 0x43, 0xff),
};

static const Fl_Color bg_colors[16] = {
	fl_rgb_color(0x12, 0x34, 0x56), fl_rgb_color(0x12, 0x1B, 0x56), fl_rgb_color(0x23, 0x12, 0x56), fl_rgb_color(0x3C, 0x12, 0x56),
	fl_rgb_color(0x56, 0x12, 0x56), fl_rgb_color(0x56, 0x12, 0x3D), fl_rgb_color(0x56, 0x12, 0x23), fl_rgb_color(0x56, 0x1A, 0x12),
	fl_rgb_color(0x56, 0x34, 0x12), fl_rgb_color(0x56, 0x4D, 0x12), fl_rgb_color(0x45, 0x56, 0x12), fl_rgb_color(0x2C, 0x56, 0x12),
	fl_rgb_color(0x12, 0x56, 0x12), fl_rgb_color(0x12, 0x56, 0x2B), fl_rgb_color(0x12, 0x56, 0x45), fl_rgb_color(0x12, 0x4E, 0x56),
};
static const Fl_Color fg_colors[16] = {
	fl_rgb_color(0xAB, 0xCD, 0xEF), fl_rgb_color(0xAB, 0xB4, 0xEF), fl_rgb_color(0xBC, 0xAB, 0xEF), fl_rgb_color(0xD5, 0xAB, 0xEF),
	fl_rgb_color(0xEF, 0xAB, 0xEF), fl_rgb_color(0xEF, 0xAB, 0xD6), fl_rgb_color(0xEF, 0xAB, 0xBC), fl_rgb_color(0xEF, 0xB3, 0xAB),
	fl_rgb_color(0xEF, 0xCD, 0xAB), fl_rgb_color(0xEF, 0xE6, 0xAB), fl_rgb_color(0xDE, 0xEF, 0xAB), fl_rgb_color(0xC5, 0xEF, 0xAB),
	fl_rgb_color(0xAB, 0xEF, 0xAB), fl_rgb_color(0xAB, 0xEF, 0xC4), fl_rgb_color(0xAB, 0xEF, 0xDE), fl_rgb_color(0xAB, 0xE7, 0xEF),
};

static void draw_outlined_text(const char *l, int x, int y, int w, int h, Fl_Align a, Fl_Color c) {
	fl_color(FL_WHITE);
	fl_draw(l, x-1, y-1, w, h, a);
	fl_draw(l, x-1, y+1, w, h, a);
	fl_draw(l, x+1, y-1, w, h, a);
	fl_draw(l, x+1, y+1, w, h, a);
	fl_color(c);
	fl_draw(l, x, y, w, h, a);
}

static void draw_grid(int x, int y) {
	fl_color(fl_rgb_color(0x40));
	for (int i = 1; i < TILE_SIZE_2X; i += 4) {
		fl_point(x+i, y+TILE_SIZE_2X-1);
		fl_point(x+i+1, y+TILE_SIZE_2X-1);
		fl_point(x+TILE_SIZE_2X-1, y+i);
		fl_point(x+TILE_SIZE_2X-1, y+i+1);
	}
	fl_color(fl_rgb_color(0xC0));
	for (int i = 0; i < TILE_SIZE_2X; i += 4) {
		fl_point(x+i, y+TILE_SIZE_2X-1);
		fl_point(x+i+3, y+TILE_SIZE_2X-1);
		fl_point(x+TILE_SIZE_2X-1, y+i);
		fl_point(x+TILE_SIZE_2X-1, y+i+3);
	}
}

static void draw_selection_border(int x, int y) {
	fl_rect(x, y, TILE_SIZE_2X, TILE_SIZE_2X, FL_BLACK);
	fl_rect(x+1, y+1, TILE_SIZE_2X-2, TILE_SIZE_2X-2, FL_WHITE);
	fl_rect(x+2, y+2, TILE_SIZE_2X-4, TILE_SIZE_2X-4, FL_BLACK);
}

std::vector<Tileset> *Tile_State::_tilesets = NULL;

void Tile_State::draw_tile(int x, int y, bool active, bool selected) {
	if (_tilesets) {
		for (std::vector<Tileset>::reverse_iterator it = _tilesets->rbegin(); it != _tilesets->rend(); ++it) {
			if (it->draw_tile(this, x, y, active)) {
				return;
			}
		}
	}
	uint16_t hi = (id & 0xF0) >> 4, lo = id & 0x0F;
	char l1 = (char)(hi > 9 ? 'A' + hi - 10 : '0' + hi), l2 = (char)(lo > 9 ? 'A' + lo - 10 : '0' + lo);
	const char buffer[3] = {l1, l2, '\0'};
	bool r = Config::rainbow_tiles();
	Fl_Color bg = bg_colors[r ? lo : 0];
	if (!active) { bg = fl_inactive(bg); }
	fl_rectf(x, y, TILE_SIZE_2X, TILE_SIZE_2X, bg);
	int s = OS::is_consolas() ? 11 : 10;
	fl_font(x_flip || y_flip ? FL_COURIER_ITALIC : FL_COURIER, s);
	Fl_Color fg = selected ? FL_YELLOW : x_flip ? y_flip ? FL_YELLOW : FL_MAGENTA : y_flip ? FL_CYAN : fg_colors[r ? hi : 0];
	if (!active) { fg = fl_inactive(fg); }
	fl_color(fg);
	fl_draw(buffer, x, y, TILE_SIZE_2X, TILE_SIZE_2X, FL_ALIGN_CENTER);
}

void Tile_State::draw_attributes(int x, int y) {
	if (palette > -1) {
		palette_images[palette].draw(x, y, TILE_SIZE_2X, TILE_SIZE_2X);
		const char buffer[2] = {(char)('0' + palette), '\0'};
		int s = OS::is_consolas() ? 11 : 10;
		fl_font(FL_COURIER_BOLD, s);
		draw_outlined_text(buffer, x, y, TILE_SIZE_2X, TILE_SIZE_2X, FL_ALIGN_CENTER, palettes[palette]);
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

void Tile_State::print(int x, int y) {
	if (_tilesets) {
		for (std::vector<Tileset>::reverse_iterator it = _tilesets->rbegin(); it != _tilesets->rend(); ++it) {
			if (it->print_tile(this, x, y)) {
				return;
			}
		}
	}
	char hi = (char)((id & 0xF0) >> 4), lo = (char)(id & 0x0F);
	bool r = Config::rainbow_tiles();
	Fl_Color bg = bg_colors[r ? lo : 0];
	fl_rectf(x, y, TILE_SIZE, TILE_SIZE, bg);
	Fl_Color fg = x_flip ? y_flip ? FL_YELLOW : FL_MAGENTA : y_flip ? FL_CYAN : fg_colors[r ? hi : 0];
	fl_color(fg);
	print_digit(x, y+1, hi);
	print_digit(x+4, y+2, lo);
}

Tile_Swatch::Tile_Swatch(int x, int y, int w, int h) : Tile_Thing(), Fl_Box(x, y, w, h), _attributes() {
	user_data(NULL);
	box(OS_SPACER_THIN_DOWN_FRAME);
	labeltype(FL_NO_LABEL);
}

void Tile_Swatch::draw() {
	draw_box();
	_state.draw(x() + Fl::box_dx(box()), y() + Fl::box_dy(box()), !_attributes, _attributes, !!active());
}

Tile_Tessera::Tile_Tessera(int x, int y, size_t row, size_t col, uint16_t id, bool x_flip, bool y_flip,
	bool priority, bool obp1, int palette) :
	Tile_Thing(id, x_flip, y_flip, priority, obp1, palette), Fl_Box(x, y, TILE_SIZE_2X, TILE_SIZE_2X),
	_row(row), _col(col) {
	user_data(NULL);
	box(FL_NO_BOX);
	labeltype(FL_NO_LABEL);
	when(FL_WHEN_RELEASE);
}

void Tile_Tessera::draw() {
	_state.draw(x(), y(), true, Config::attributes(), !!active());
	if (Config::grid()) {
		draw_grid(x(), y());
	}
	if (this == Fl::belowmouse()) {
		draw_selection_border(x(), y());
	}
}

int Tile_Tessera::handle(int event) {
	Main_Window *mw = (Main_Window *)user_data();
	switch (event) {
	case FL_ENTER:
		if (Fl::event_button1() && !Fl::pushed()) {
			Fl::pushed(this);
			do_callback();
		}
		mw->update_status(this);
		redraw();
		return 1;
	case FL_LEAVE:
		mw->update_status(NULL);
		// fallthrough
	case FL_MOVE:
		redraw();
		return 1;
	case FL_PUSH:
		mw->map_editable(true);
		do_callback();
		return 1;
	case FL_RELEASE:
		mw->map_editable(false);
		return 1;
	case FL_DRAG:
		if (!Fl::event_inside(x(), y(), w(), h())) {
			Fl::pushed(NULL);
		}
		return 1;
	}
	return Fl_Box::handle(event);
}

Tile_Button::Tile_Button(int x, int y, uint16_t id) : Tile_Thing(id), Fl_Radio_Button(x, y, TILE_SIZE_2X, TILE_SIZE_2X) {
	user_data(NULL);
	box(FL_NO_BOX);
	labeltype(FL_NO_LABEL);
	when(FL_WHEN_RELEASE);
}

void Tile_Button::draw() {
	_state.draw(x(), y(), true, Config::attributes(), !!active(), !!value());
	if (Config::grid()) {
		draw_grid(x(), y());
	}
	if (value()) {
		draw_selection_border(x(), y());
	}
}

int Tile_Button::handle(int event) {
	// Don't interfere with dragging onto the parent Droppable|Workpane
	return event == FL_ENTER || event == FL_LEAVE || event == FL_DRAG ? 0 : Fl_Radio_Button::handle(event);
}
