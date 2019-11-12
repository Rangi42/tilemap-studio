#include "tile-selection.h"
#include "widgets.h"
#include "config.h"

void Tile_Selection::draw_selection_border_at(Grossable *t) const {
	if (!_tile1 || !_tile2) { return; }
	Workspace *p = (Workspace *)t->parent();
	if (!p) { return; }
	int pw = p->w() - (p->has_y_scroll() ? Fl::scrollbar_size() : 0);
	int ph = p->h() - (p->has_x_scroll() ? Fl::scrollbar_size() : 0);
	int tx = t == _tile1 ? MIN(_tile1->x(), _tile2->x()) : t->x();
	int ty = t == _tile1 ? MIN(_tile1->y(), _tile2->y()) : t->y();
	int tw = t->w() * (int)width(), th = t->h() * (int)height();
	bool zoom = !_from_tileset && Config::zoom() > 5;
	fl_push_clip(p->x(), p->y(), pw, ph);
	draw_selection_border(tx, ty, tw, th, FL_WHITE, zoom);
	fl_pop_clip();
}

void Tile_Selection::select_single(Tile_Button *tb) {
	_tile1 = tb;
	_tile2 = NULL;
	_dragging = false;
	_from_tileset = true;
	tb->setonly();
}

void Tile_Selection::start_selecting(Tile_Tessera *tt) {
	_tile1 = tt;
	_tile2 = tt;
	_dragging = true;
	_from_tileset = false;
}

void Tile_Selection::start_selecting(Tile_Button *tb) {
	_tile1 = tb;
	_tile2 = tb;
	_dragging = true;
	_from_tileset = true;
}

void Tile_Selection::finish_selecting() {
	_dragging = false;
	if (_tile1 == _tile2) {
		_tile2 = NULL;
	}
	if (_tile1) {
		_tile1->redraw();
	}
	if (_tile2) {
		_tile2->redraw();
	}
}
