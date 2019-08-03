#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "themes.h"
#include "config.h"
#include "main-window.h"
#include "tiled-image.h"
#include "tile-buttons.h"

Tileset *Tile_State::_tileset = NULL;

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

static void draw_selection_border(int x, int y) {
	fl_rect(x, y, TILE_SIZE_2X, TILE_SIZE_2X, FL_BLACK);
	fl_rect(x+1, y+1, TILE_SIZE_2X-2, TILE_SIZE_2X-2, FL_WHITE);
	fl_rect(x+2, y+2, TILE_SIZE_2X-4, TILE_SIZE_2X-4, FL_BLACK);
}

Tile_State::Tile_State(uint8_t id_, bool x_flip_, bool y_flip_) : id(id_), x_flip(x_flip_), y_flip(y_flip_) {}

void Tile_State::draw(int x, int y, bool active, bool selected) {
	if (_tileset->draw_tile(this, x, y, active)) { return; }
	char hi = (char)((id & 0xF0) >> 4), lo = (char)(id & 0x0F);
	const char buffer[3] = {hi > 9 ? 'A' + hi - 10 : '0' + hi, lo > 9 ? 'A' + lo - 10 : '0' + lo, '\0'};
	bool r = Config::rainbow_tiles();
	Fl_Color bg = bg_colors[r ? lo : 0];
	if (!active) { bg = fl_inactive(bg); }
	fl_rectf(x, y, TILE_SIZE_2X, TILE_SIZE_2X, bg);
	Fl_Font f = x_flip || y_flip ? FL_COURIER_ITALIC : FL_COURIER;
	int s = OS::is_consolas() ? 11 : 10;
	fl_font(f, s);
	Fl_Color fg = selected ? FL_YELLOW : x_flip ? y_flip ? FL_YELLOW : FL_MAGENTA : y_flip ? FL_CYAN : fg_colors[r ? hi : 0];
	if (!active) { fg = fl_inactive(fg); }
	fl_color(fg);
	fl_draw(buffer, x, y, TILE_SIZE_2X, TILE_SIZE_2X, FL_ALIGN_CENTER);
}

Tile_Tessera::Tile_Tessera(int x, int y, uint8_t row, uint8_t col, uint8_t id, bool x_flip, bool y_flip) :
	Fl_Box(x, y, TILE_SIZE_2X, TILE_SIZE_2X), _row(row), _col(col), _state(id, x_flip, y_flip) {
	user_data(NULL);
	box(FL_NO_BOX);
	labeltype(FL_NO_LABEL);
	when(FL_WHEN_RELEASE);
}

void Tile_Tessera::draw() {
	_state.draw(x(), y(), !!active());
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

Tile_Button::Tile_Button(int x, int y, uint8_t id) : Fl_Radio_Button(x, y, TILE_SIZE_2X, TILE_SIZE_2X), _state(id) {
	user_data(NULL);
	box(FL_NO_BOX);
	labeltype(FL_NO_LABEL);
	when(FL_WHEN_RELEASE);
}

void Tile_Button::draw() {
	_state.draw(x(), y(), !!active(), !!value());
	if (value()) {
		draw_selection_border(x(), y());
	}
}
