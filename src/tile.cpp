#include "tile.h"

bool is_blank_tile(Tile &tile) {
	for (int i = 0; i < NUM_TILE_PIXELS; i++) {
		if (tile[i] != SPACE_COLOR) {
			return false;
		}
	}
	return true;
}

bool are_identical_tiles(Tile &t1, Tile &t2, Tilemap_Format fmt, bool &x_flip, bool &y_flip) {
	for (int i = 0; i < NUM_TILE_PIXELS; i++) {
		if (t1[i] != t2[i]) {
			goto not_identical;
		}
	}
	return true;
not_identical:
	if (fmt == Tilemap_Format::XY_FLIP || fmt == Tilemap_Format::TILE_ATTR) {
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
