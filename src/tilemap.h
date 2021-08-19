#ifndef TILEMAP_H
#define TILEMAP_H

#include <deque>
#include <vector>

#include "config.h"
#include "utils.h"
#include "tile-buttons.h"
#include "option-dialogs.h"

#define MAX_HISTORY_SIZE 100

struct Tilemap_State {
	std::vector<Tile_State> states;
	Tilemap_State() : states() {}
	Tilemap_State(size_t n) : states(n) {}
	inline const Tile_State &state(size_t i) const { return states[i]; }
};

class Tilemap {
public:
	enum class Result { TILEMAP_OK, TILEMAP_BAD_FILE, TILEMAP_EMPTY, TILEMAP_TOO_SHORT_FF, TILEMAP_TOO_LONG_FF,
		TILEMAP_TOO_SHORT_00, TILEMAP_TOO_LONG_00, TILEMAP_TOO_SHORT_RLE, TILEMAP_TOO_SHORT_ATTRS, TILEMAP_NULL,
		ATTRMAP_BAD_FILE, ATTRMAP_TOO_SHORT, ATTRMAP_TOO_LONG };
private:
	std::vector<Tile_Tessera *> _tiles;
	size_t _width;
	Result _result;
	bool _modified;
	std::deque<Tilemap_State> _history, _future;
public:
	Tilemap();
	~Tilemap();
	inline size_t size(void) const { return _tiles.size(); }
	inline size_t width(void) const { return _width; }
	void width(size_t w);
	void resize(size_t w, size_t h, Resize_Dialog::Hor_Align ha, Resize_Dialog::Vert_Align va);
	void shift(int dx, int dy);
	inline bool can_shift(void) const { return size() % _width == 0; }
	inline size_t height(void) const { return (size() + _width - 1) / _width; }
	inline Tile_Tessera *tile(size_t x, size_t y) const { return tile(y * _width + x); }
	inline Tile_Tessera *tile(size_t i) const { return i < _tiles.size() ? _tiles[i] : NULL; }
	inline void tile(size_t x, size_t y, Tile_Tessera *tt) { _tiles[y * _width + x] = tt; }
	inline Result result(void) const { return _result; }
	inline bool modified(void) const { return _modified; }
	inline void modified(bool m) { _modified = m; }
	inline bool can_undo(void) const { return !_history.empty(); }
	inline bool can_redo(void) const { return !_future.empty(); }
	inline const Tilemap_State &last_state(void) const { return _history.back(); }
	void clear();
	void reposition_tiles(int x, int y);
	void remember(void);
	void undo(void);
	void redo(void);
	bool can_format_as(Tilemap_Format fmt);
	void limit_to_format(Tilemap_Format fmt);
	void new_tiles(size_t w, size_t h);
	Result read_tiles(const char *tf, const char *af);
	inline bool write_tiles(const char *tf, const char *af) { return write_tiles(tf, af, _tiles, Config::format()); }
	inline bool export_tiles(const char *f) {
		return ends_with_ignore_case(f, ".csv") ? export_csv_tiles(f, Config::format()) : export_c_tiles(f, Config::format());
	}
	void print_tilemap(void) const;
	void guess_width(void);
private:
	Result make_tiles(const std::vector<uchar> &tbytes, const std::vector<uchar> &abytes);
	bool export_c_tiles(const char *f, Tilemap_Format fmt);
	bool export_csv_tiles(const char *f, Tilemap_Format fmt);
public:
	static bool write_tiles(const char *tf, const char *af, std::vector<Tile_Tessera *> &tiles, Tilemap_Format fmt);
	static const char *error_message(Result result);
};

#endif
