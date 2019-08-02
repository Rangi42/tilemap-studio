#ifndef TILEMAP_H
#define TILEMAP_H

#include <deque>
#include <vector>

#include "utils.h"
#include "tile-buttons.h"
#include "option-dialogs.h"

#define MAX_HISTORY_SIZE 100

#define NUM_FORMATS 6

#define DEFAULT_WIDTH 20
#define DEFAULT_HEIGHT 18

class Tilemap {
protected:
	struct Tilemap_State {
		std::vector<Tile_State> states;
		Tilemap_State() : states() {}
		Tilemap_State(size_t n) : states(n) {}
	};
public:
	enum Format { PLAIN, RLE, FF_END, RLE_FF_END, RLE_NYBBLES, XY_FLIP };
	enum Result { TILEMAP_OK, TILEMAP_BAD_FILE, TILEMAP_EMPTY, TILEMAP_TOO_LARGE, TILEMAP_TOO_SHORT_FF, TILEMAP_TOO_LONG_FF,
		TILEMAP_TOO_SHORT_00, TILEMAP_TOO_LONG_00, TILEMAP_TOO_SHORT_RLE, TILEMAP_NULL };
private:
	size_t _size, _width;
	Tile_Tessera **_tiles;
	Result _result;
	bool _modified;
	std::deque<Tilemap_State> _history, _future;
public:
	Tilemap();
	~Tilemap();
	inline size_t size(void) const { return _size; }
	void size(size_t w, size_t h);
	void size(size_t n);
	inline size_t width(void) const { return _width; }
	void width(size_t w);
	void resize(size_t w, size_t h, Resize_Dialog::Hor_Align ha, Resize_Dialog::Vert_Align va);
	inline size_t height(void) const { return (_size + _width - 1) / _width; }
	inline Tile_Tessera *tile(uint8_t x, uint8_t y) const { return _tiles[(size_t)y * _width + (size_t)x]; }
	inline Tile_Tessera *tile(size_t i) const { return _tiles[i]; }
	inline void tile(uint8_t x, uint8_t y, Tile_Tessera *tt) { _tiles[(size_t)y * _width + (size_t)x] = tt; }
	inline void tile(size_t i, Tile_Tessera *tt) { _tiles[i] = tt; }
	inline Result result(void) const { return _result; }
	inline bool modified(void) const { return _modified; }
	inline void modified(bool m) { _modified = m; }
	inline bool can_undo(void) const { return !_history.empty(); }
	inline bool can_redo(void) const { return !_future.empty(); }
	void clear();
	void reposition_tiles(int x, int y);
	void remember(void);
	void undo(void);
	void redo(void);
	void new_tiles(size_t w, size_t h);
	Result read_tiles(const char *f);
	bool can_write_tiles(void);
	bool write_tiles(const char *f);
private:
	void guess_width(void);
public:
	static int format_tileset_size(Format fmt);
	static const char *format_name(Format fmt);
	static const char *format_extension(Format fmt);
	static const char *error_message(Result result);
};

#endif
