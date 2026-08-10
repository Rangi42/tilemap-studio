#ifndef TILESET_H
#define TILESET_H

#include <array>
#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Image.H>
#pragma warning(pop)

#include "utils.h"
#include "tile.h"
#include "palette-format.h"

#define NUM_HUES 4
#define BYTES_PER_1BPP_TILE (NUM_TILE_PIXELS / 8)
#define BYTES_PER_2BPP_TILE (BYTES_PER_1BPP_TILE * 2)
#define BYTES_PER_4BPP_TILE (BYTES_PER_1BPP_TILE * 4)
#define BYTES_PER_8BPP_TILE (BYTES_PER_1BPP_TILE * 8)

#define PALETTES_PER_ROW 8
#define MAX_NUM_PALETTES 16

#define DEFAULT_TILES_PER_ROW 16
#define MAX_NUM_TILES 0x800 // max(tileset_sizes) in tilemap-format.cpp

// One tile's palette color indices, addressed row-major: [row * TILE_SIZE + col].
typedef std::array<uchar, NUM_TILE_PIXELS> Tile_Color_Indices;

struct Tile_State;

class Tileset {
public:
	enum class Result { TILESET_OK, TILESET_BAD_FILE, TILESET_BAD_EXT, TILESET_BAD_DIMS,
		TILESET_TOO_SHORT, TILESET_TOO_LARGE, TILESET_BAD_CMD, TILESET_NULL };
private:
	Fl_RGB_Image *_1x_image, *_2x_image, *_zoomed_image;
	size_t _num_tiles;
	int _start_id, _offset, _length;
	Result _result;
	std::vector<Tile_Color_Indices> _color_indices; // one entry per tile
	bool _has_color_indices;
public:
	Tileset(int start_id, int offset, int length);
	~Tileset();
	inline size_t num_tiles(void) const { return _num_tiles; }
	inline int start_id(void) const { return _start_id; }
	inline int offset(void) const { return _offset; }
	inline int length(void) const { return _length; }
	inline Result result(void) const { return _result; }
	inline bool has_color_indices(void) const { return _has_color_indices; }
	const uchar *tile_color_indices(int index) const;
	// Reconstruct the baked-in palette (color index -> RGB) from the tileset image; pal is filled with
	// n entries (missing ones left black). Returns how many distinct indices were resolved.
	size_t color_palette(Palette &pal, size_t n) const;
	void clear(void);
	void update_zoom(void);
	void shift(int dn);
	bool draw_tile(const Tile_State *ts, int x, int y, int z, bool active) const;
	bool print_tile(const Tile_State *ts, int x, int y, bool active) const;
	Result read_tiles(const char *f);
private:
	Result read_png_graphics(const char *f);
	Result read_gif_graphics(const char *f);
	Result read_bmp_graphics(const char *f);
	Result read_1bpp_graphics(const char *f);
	Result read_2bpp_graphics(const char *f);
	Result read_4bpp_graphics(const char *f);
	Result read_8bpp_graphics(const char *f);
	Result read_1bpp_lz_graphics(const char *f);
	Result read_2bpp_lz_graphics(const char *f);
	Result read_rgcn_graphics(const char *f);
	Result read_rts_graphics(const char *f, bool skip_rmp);
	Result parse_1bpp_data(const std::vector<uchar> &data);
	Result parse_2bpp_data(const std::vector<uchar> &data);
	Result parse_4bpp_data(const std::vector<uchar> &data);
	Result parse_8bpp_data(const std::vector<uchar> &data);
	Result postprocess_graphics(Fl_RGB_Image *img);
	void build_color_indices_from_pixels(const std::vector<uchar> &pixels, int w, int h);
	bool draw_recolored(const uchar *idx, const Palette &palette, int x, int y, int z, bool x_flip, bool y_flip) const;
public:
	static const char *error_message(Result result);
};

#endif
