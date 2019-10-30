#include <cstdio>
#include <string>
#include <sstream>
#include <png.h>
#include <zlib.h>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#pragma warning(pop)

#include "utils.h"
#include "image.h"

Image::Result Image::write_image(const char *f, Fl_RGB_Image *img, Type type) {
	FILE *file = fl_fopen(f, "wb");
	if (!file) { return IMAGE_BAD_FILE; }
	// Create the necessary PNG structures
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) { fclose(file); return IMAGE_BAD_PNG; }
	png_infop info = png_create_info_struct(png);
	if (!info) { fclose(file); return IMAGE_BAD_PNG; }
	png_init_io(png, file);
	// Set compression options
	png_set_compression_level(png, Z_BEST_COMPRESSION);
	png_set_compression_mem_level(png, Z_BEST_COMPRESSION);
	png_set_compression_strategy(png, Z_DEFAULT_STRATEGY);
	png_set_compression_window_bits(png, 15);
	png_set_compression_method(png, Z_DEFLATED);
	png_set_compression_buffer_size(png, 8192);
	// Write the PNG IHDR chunk
	size_t w = img->w(), h = img->h();
	int depth = type == IMAGE_TYPE_2BPP ? 2 : type == IMAGE_TYPE_4BPP ? 4 : 8;
	int color_type = type == IMAGE_TYPE_RGB ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY;
	png_set_IHDR(png, info, (png_uint_32)w, (png_uint_32)h, depth, color_type,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	// Write the other PNG header chunks
	png_write_info(png, info);
	// Write the RGB pixels in row-major order from top to bottom
	const char *buffer = img->data()[0];
	int d = img->d();
	int ld = img->ld();
	if (!ld) { ld = (int)w * d; }
	int pd = d > 1;
	png_bytep png_row = NULL;
	if (type == IMAGE_TYPE_RGB) {
		size_t rs = w * NUM_CHANNELS;
		png_row = new png_byte[rs];
		for (size_t i = 0; i < h; i++) {
			for (size_t j = 0; j < w; j++) {
				size_t rd = NUM_CHANNELS * j;
				size_t px = ld * i + d * j;
				for (size_t k = 0; k < NUM_CHANNELS; k++) {
					png_row[rd+k] = buffer[px+pd*k];
				}
			}
			png_write_row(png, png_row);
		}
	}
	else {
		size_t pq = 8 / (size_t)depth;
		uchar m = (uchar)pow(2, 8 - depth);
		size_t rs = w / pq;
		png_row = new png_byte[rs];
		for (size_t i = 0; i < h; i++) {
			for (size_t j = 0; j < rs; j++) {
				uchar pp = 0;
				for (size_t k = 0; k < pq; k++) {
					size_t px = ld * i + d * (j * pq + k);
					uchar v = (buffer[px] & 0xFF) / m; // [0, 2^8-1] -> [0, 2^depth-1]
					pp = (pp << depth) | v;
				}
				png_row[j] = pp;
			}
			png_write_row(png, png_row);
		}
	}
	png_write_end(png, NULL);
	delete [] png_row;
	png_destroy_write_struct(&png, &info);
	png_free_data(png, info, PNG_FREE_ALL, -1);
	fclose(file);
	return IMAGE_OK;
}

const char *Image::error_message(Result result) {
	switch (result) {
	case IMAGE_OK:
		return "OK.";
	case IMAGE_BAD_FILE:
		return "Cannot open file.";
	case IMAGE_BAD_PNG:
		return "Cannot write PNG data.";
	default:
		return "Unspecified error.";
	}
}

bool Image::make_deimage(Fl_Widget *wgt) {
	if (!wgt || !wgt->image()) {
		return false;
	}
	Fl_Image *deimg = wgt->image()->copy();
	if (!deimg) {
		return false;
	}
	deimg->desaturate();
	deimg->color_average(FL_GRAY, 0.5f);
	if (wgt->deimage()) {
		delete wgt->deimage();
	}
	wgt->deimage(deimg);
	return true;
}
