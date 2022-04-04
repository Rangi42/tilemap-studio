#include <algorithm>

#include "tile.h"
#include "utils.h"

bool is_blank_tile(const Tile &tile, Fl_Color blank_color) {
	return std::all_of(RANGE(tile), [&](const Fl_Color &c) {
		return c == blank_color;
	});
}

bool are_identical_tiles(const Tile &t1, const Tile &t2, bool allow_flip, bool &x_flip, bool &y_flip) {
	for (int i = 0; i < NUM_TILE_PIXELS; i++) {
		if (t1[i] != t2[i]) {
			goto not_identical;
		}
	}
	return true;
not_identical:
	if (allow_flip) {
		for (int y = 0; y < TILE_SIZE; y++) {
			for (int x = 0; x < TILE_SIZE; x++) {
				if (t1[y*TILE_SIZE+x] != t2[y*TILE_SIZE+TILE_SIZE-x-1]) {
					goto not_x_flipped;
				}
			}
		}
		x_flip = true;
		return true;
not_x_flipped:
		for (int y = 0; y < TILE_SIZE; y++) {
			for (int x = 0; x < TILE_SIZE; x++) {
				if (t1[y*TILE_SIZE+x] != t2[(TILE_SIZE-y-1)*TILE_SIZE+x]) {
					goto not_y_flipped;
				}
			}
		}
		y_flip = true;
		return true;
not_y_flipped:
		for (int y = 0; y < TILE_SIZE; y++) {
			for (int x = 0; x < TILE_SIZE; x++) {
				if (t1[y*TILE_SIZE+x] != t2[(TILE_SIZE-y-1)*TILE_SIZE+TILE_SIZE-x-1]) {
					goto not_xy_flipped;
				}
			}
		}
		x_flip = y_flip = true;
		return true;
	}
not_xy_flipped:
	return false;
}

Tile *get_image_tiles(Fl_RGB_Image *img, size_t &n, size_t &iw, bool alt_norm, Fl_Color blank_color) {
	if (!img) { return NULL; }

	int w = img->w(), h = img->h();
	if (w % TILE_SIZE || h % TILE_SIZE) { return NULL; }
	w /= TILE_SIZE;
	h /= TILE_SIZE;
	n = (size_t)(w * h);
	iw = (size_t)w;

	const uchar *data = (const uchar *)img->data()[0];
	int d = img->d(), ld = img->ld();
	if (!ld) { ld = img->w() * d; }
	int dp = d > 1;

	Tile *tiles = new Tile[n + 1]();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = y * w + x;
			for (int ty = 0; ty < TILE_SIZE; ty++) {
				int oy = (y * TILE_SIZE + ty) * ld;
				for (int tx = 0; tx < TILE_SIZE; tx++) {
					int ox = (x * TILE_SIZE + tx) * d;
					const uchar *px = data + oy + ox;
					// Round color channels to 5 bits
					uchar r = NORMRGB(px[0]), g = NORMRGB(px[dp]), b = NORMRGB(px[dp+dp]);
					Fl_Color c = fl_rgb_color(r, g, b);
					if (alt_norm) { c &= ALT_NORM_MASK; }
					int ti = ty * TILE_SIZE + tx;
					tiles[i][ti] = c;
				}
			}
		}
	}
	std::fill(RANGE(tiles[n]), blank_color); // Fail-safe blank tile at the end

	return tiles;
}
