#include <cstdio>
#include <string>
#include <sstream>
#include <png.h>
#include <zlib.h>

#pragma warning(push, 0)
#include <FL/Fl.H>
#pragma warning(pop)

#include "utils.h"
#include "image.h"

Image::Result Image::write_image(const char *f, Fl_RGB_Image *img, bool grayscale) {
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
	png_set_IHDR(png, info, (png_uint_32)w, (png_uint_32)h, 8, grayscale ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	// Write the other PNG header chunks
	png_write_info(png, info);
	// Write the RGB pixels in row-major order from top to bottom
	const char *buffer = img->data()[0];
	int d = img->d();
	int ld = img->ld();
	if (!ld) { ld = (int)w * d; }
	int pd = d > 1;
	size_t pc = grayscale ? 1 : 3;
	size_t row_size = pc * w;
	png_bytep png_row = new png_byte[row_size];
	for (size_t i = 0; i < h; i++) {
		for (size_t j = 0; j < w; j++) {
			size_t rd = pc * j;
			size_t px = ld * i + d * j;
			for (size_t k = 0; k < pc; k++) {
				png_row[rd+k] = buffer[px+pd*k];
			}
		}
		png_write_row(png, png_row);
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
