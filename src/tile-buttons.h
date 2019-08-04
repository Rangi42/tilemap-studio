#ifndef TILE_BUTTON_H
#define TILE_BUTTON_H

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#include <FL/Fl_Radio_Button.H>
#pragma warning(pop)

#include "utils.h"
#include "tileset.h"

#define NUM_SGB_COLORS 4

class Tileset;

struct Tile_State {
private:
	static Tileset *_tileset;
public:
	inline static void tileset(Tileset *t) { _tileset = t; }
public:
	uint8_t id;
	bool x_flip, y_flip;
	int color;
public:
	Tile_State(uint8_t id = 0x00, bool x_flip = false, bool y_flip = false, int color = -1);
	void draw(int x, int y, bool active, bool selected = false);
	inline bool operator==(const Tile_State &other) const {
		return id == other.id && x_flip == other.x_flip && y_flip == other.y_flip && color == other.color;
	}
	inline bool operator!=(const Tile_State &other) const {
		return !(*this == other);
	}
};

class Tile_Tessera : public Fl_Box {
private:
	size_t _row, _col;
	Tile_State _state;
public:
	Tile_Tessera(int x, int y, size_t row = 0, size_t col = 0, uint8_t id = 0x00,
		bool x_flip = false, bool y_flip = false, int color = -1);
	inline size_t row(void) const { return _row; }
	inline size_t col(void) const { return _col; }
	inline void coords(size_t row, size_t col) { _row = row; _col = col; }
	inline Tile_State state(void) const { return _state; }
	inline void state(Tile_State state) { _state = state; }
	inline uint8_t id(void) const { return _state.id; }
	inline void id(uint8_t id) { _state.id = id; }
	inline bool x_flip(void) const { return _state.x_flip; }
	inline void x_flip(bool x_flip) { _state.x_flip = x_flip; }
	inline bool y_flip(void) const { return _state.y_flip; }
	inline void y_flip(bool y_flip) { _state.y_flip = y_flip; }
	inline int sgb_color(void) const { return _state.color; }
	inline void sgb_color(int color) { _state.color = color; }
	void draw(void);
	int handle(int event);
};

class Tile_Button : public Fl_Radio_Button {
private:
	size_t _row, _col;
	Tile_State _state;
public:
	Tile_Button(int x, int y, uint8_t id = 0x00);
	inline Tile_State state(void) const { return _state; }
	inline void state(Tile_State state) { _state = state; }
	inline uint8_t id(void) const { return _state.id; }
	inline void id(uint8_t id) { _state.id = id; }
	void draw(void);
};

#endif
