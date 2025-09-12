#ifndef TILE_BUTTON_H
#define TILE_BUTTON_H

#include <vector>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "utils.h"
#include "config.h"
#include "tile.h"

#define TILE_SIZE_2X (TILE_SIZE * DEFAULT_ZOOM)

class Tileset;

void draw_selection_border(int x, int y, int w, int h, Fl_Color c, bool zoom);

struct Tile_State {
private:
	static std::vector<Tileset> *_tilesets;
	static Fl_PNG_Image *_palette_bgs_image;
public:
	inline static void tilesets(std::vector<Tileset> *ts) { _tilesets = ts; }
	static void alpha(uchar alfa);
	static void update_zoom(void);
public:
	uint16_t id;
	bool x_flip, y_flip, priority, obp1;
	int palette;
public:
	inline Tile_State(uint16_t id_ = 0x000, bool x_flip_ = false, bool y_flip_ = false, bool priority_ = false,
		bool obp1_ = false, int palette_ = -1) : id(id_), x_flip(x_flip_), y_flip(y_flip_), priority(priority_),
		obp1(obp1_), palette(palette_) {}
	inline bool same_tiles(const Tile_State &other) const {
		return id == other.id && x_flip == other.x_flip && y_flip == other.y_flip;
	}
	inline bool same_attributes(const Tile_State &other) const {
		return priority == other.priority && obp1 == other.obp1 && palette == other.palette;
	}
	inline bool same(const Tile_State &other, bool attr) const {
		return attr ? same_attributes(other) : same_tiles(other);
	}
	inline bool highlighted(void) const { return id == Config::highlight_id(); }
	void draw(int x, int y, int z, bool tile, bool attr, int style, bool active, bool selected);
	void print(int x, int y, bool active, bool selected, int palette_ = -1);
private:
	void draw_tile(int x, int y, int z, bool active, bool selected);
	void draw_tile_1x(int x, int y, bool active, bool selected);
	void draw_attributes(int x, int y, int z, int style, bool active);
};

class Tile_Thing {
protected:
	Tile_State _state;
public:
	inline Tile_Thing(uint16_t id_ = 0x000, bool x_flip_ = false, bool y_flip_ = false, bool priority_ = false,
		bool obp1_ = false, int palette_ = -1) : _state(id_, x_flip_, y_flip_, priority_, obp1_, palette_) {}
	inline Tile_State state(void) const { return _state; }
	inline void state(const Tile_State &state) { _state = state; }
	inline void tile(Tile_State state) {
		_state.id = state.id; _state.x_flip = state.x_flip; _state.y_flip = state.y_flip;
	}
	inline void attributes(const Tile_State &state) {
		_state.palette = state.palette; _state.priority = state.priority; _state.obp1 = state.obp1;
	}
	inline void assign(const Tile_State &state, bool attr) {
		if (attr) { attributes(state); } else { tile(state); }
	}
	inline void replace(const Tile_State &state, bool attr) {
		attributes(state); if (!attr) { tile(state); }
	}
	inline uint16_t id(void) const { return _state.id; }
	inline void id(uint16_t id) { _state.id = id; }
	void shift_id(int d, int n);
	inline bool x_flip(void) const { return _state.x_flip; }
	inline void x_flip(bool x_flip) { _state.x_flip = x_flip; }
	inline bool y_flip(void) const { return _state.y_flip; }
	inline void y_flip(bool y_flip) { _state.y_flip = y_flip; }
	inline bool priority(void) const { return _state.priority; }
	inline void priority(bool priority) { _state.priority = priority; }
	inline bool obp1(void) const { return _state.obp1; }
	inline void obp1(bool obp1) { _state.obp1 = obp1; }
	inline int palette(void) const { return _state.palette; }
	inline void palette(int palette) { _state.palette = palette; }
};

class Tile_Swatch : public Tile_Thing, public Fl_Box {
private:
	bool _attributes;
public:
	Tile_Swatch(int x, int y, int w, int h);
	void draw(void);
	inline bool attributes(void) const { return _attributes; }
	inline void attributes(bool a) { _attributes = a; }
};

class Groupable : public Tile_Thing, public Fl_Box {
private:
	size_t _row, _col;
public:
	inline Groupable(int x = 0, int y = 0, size_t row = 0, size_t col = 0, uint16_t id = 0x000,
		bool x_flip = false, bool y_flip = false, bool priority = false, bool obp1 = false, int palette = -1) :
		Tile_Thing(id, x_flip, y_flip, priority, obp1, palette), Fl_Box(x, y, TILE_SIZE_2X, TILE_SIZE_2X),
		_row(row), _col(col) {}
	inline size_t row(void) const { return _row; }
	inline size_t col(void) const { return _col; }
	inline void coords(size_t row, size_t col) { _row = row; _col = col; }
};

class Tile_Tessera : public Groupable {
public:
	static const uchar TILE_TESSERA_TYPE = 0x42;
public:
	Tile_Tessera(int x = 0, int y = 0, size_t row = 0, size_t col = 0, uint16_t id = 0x000,
		bool x_flip = false, bool y_flip = false, bool priority = false, bool obp1 = false, int palette = -1);
	inline void print(int dx, int dy, bool active, bool selected) { _state.print(dx, dy, active, selected, palette()); }
	void draw(void);
	int handle(int event);
};

class Tile_Button : public Groupable {
private:
	char _value, _old_value;
public:
	Tile_Button(int x, int y, size_t row = 0, size_t col = 0, uint16_t id = 0x000);
	inline char value(void) const { return _value; }
	int value(char v);
	inline int set(void) { return value(1); }
	inline int clear(void) { return value(0); }
	void setonly(void);
	void draw(void);
	int handle(int event);
};

class Palette_Button : public Tile_Thing, public Fl_Radio_Button {
public:
	Palette_Button(int x, int y, int p = -1);
	void draw(void);
	int handle(int event);
};

#endif
