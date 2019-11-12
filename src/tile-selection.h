#ifndef TILE_SELECTION_H
#define TILE_SELECTION_H

#include "utils.h"
#include "tile-buttons.h"

class Tile_Selection {
private:
	Grossable *_tile1, *_tile2;
	bool _dragging, _from_tileset;
public:
	inline Tile_Selection() : _tile1(NULL), _tile2(NULL), _dragging(false), _from_tileset(false) {}
	inline bool selected(void) const { return !!_tile1; }
	inline bool selected_multiple(void) const { return _tile1 && _tile2; }
	inline bool selecting(void) const { return _dragging; }
	inline bool from_tileset(void) const { return _from_tileset; }
	inline void from_tileset(bool t) { _from_tileset = t; }
	inline uint16_t id(void) const { return _tile1->id(); }
	inline size_t top_row(void) const { return MIN(_tile1->row(), _tile2->row()); }
	inline size_t left_col(void) const { return MIN(_tile1->col(), _tile2->col()); }
	void select_single(Tile_Button *tb);
	void start_selecting(Tile_Tessera *tt);
	void start_selecting(Tile_Button *tb);
	inline void continue_selecting(Grossable *t) { _tile2 = t; }
	void finish_selecting(void);
	inline size_t width(void) const {
		return 1 + (_tile2 ? _tile1->col() > _tile2->col() ? _tile1->col() - _tile2->col() : _tile2->col() - _tile1->col() : 0);
	}
	inline size_t height(void) const {
		return 1 + (_tile2 ? _tile1->row() > _tile2->row() ? _tile1->row() - _tile2->row() : _tile2->row() - _tile1->row() : 0);
	}
	inline void draw_selection_border_at(void) const { draw_selection_border_at(_tile1); }
	void draw_selection_border_at(Grossable *t) const;
};

#endif
