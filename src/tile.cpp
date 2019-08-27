#include "tile.h"

bool is_blank_tile(Tile &tile) {
	for (int i = 0; i < NUM_TILE_PIXELS; i++) {
		if (tile[i] != SPACE_COLOR) {
			return false;
		}
	}
	return true;
}

bool are_identical_tiles(Tile &t1, Tile &t2) {
	for (int i = 0; i < NUM_TILE_PIXELS; i++) {
		if (t1[i] != t2[i]) {
			return false;
		}
	}
	return true;
}

Tile *get_image_tiles(Fl_RGB_Image *img, size_t &n) {
	if (!img) { return NULL; }

	int w = img->w(), h = img->h();
	if (w % TILE_SIZE || h % TILE_SIZE) { return NULL; }
	w /= TILE_SIZE;
	h /= TILE_SIZE;
	n = (size_t)(w * h);

	const uchar *data = (const uchar *)img->data()[0];
	int d = img->d(), ld = img->ld();
	if (!ld) { ld = img->w() * d; }
	int p = d > 1;

	Tile *tiles = new Tile[n]();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = y * w + x;
			for (int ty = 0; ty < TILE_SIZE; ty++) {
				for (int tx = 0; tx < TILE_SIZE; tx++) {
					int ti = ty * TILE_SIZE + tx;
					int off = (y * TILE_SIZE + ty) * ld + (x * TILE_SIZE + tx) * d;
					uchar r = data[off], g = data[off+p], b = data[off+p+p];
					Fl_Color rgb = fl_rgb_color(r, g, b);
					tiles[i][ti] = rgb;
				}
			}
		}
	}

	return tiles;
}
