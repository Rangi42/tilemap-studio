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
	if (ends_with(f, ".bmp") || ends_with(f, ".BMP")) {
		return write_bmp_image(f, img, type);
	}
	return write_png_image(f, img, type);
}

Image::Result Image::write_png_image(const char *f, Fl_RGB_Image *img, Type type) {
	FILE *file = fl_fopen(f, "wb");
	if (!file) { return Result::IMAGE_BAD_FILE; }
	// Create the necessary PNG structures
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) { fclose(file); return Result::IMAGE_BAD_PNG; }
	png_infop info = png_create_info_struct(png);
	if (!info) { fclose(file); return Result::IMAGE_BAD_PNG; }
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
	int depth = type == Type::IMAGE_TYPE_2BPP ? 2 : type == Type::IMAGE_TYPE_4BPP ? 4 : 8;
	int color_type = type == Type::IMAGE_TYPE_RGB ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY;
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
	if (type == Type::IMAGE_TYPE_RGB) {
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
	return Result::IMAGE_OK;
}

Image::Result Image::write_bmp_image(const char *f, Fl_RGB_Image *img, Type) {
	FILE *file = fl_fopen(f, "wb");
	if (!file) { return Result::IMAGE_BAD_FILE; }
	// Write the BMP headers
	size_t w = img->w(), h = img->h();
	size_t file_header_size = 14;
	size_t info_header_size = 40;
	size_t header_size = file_header_size + info_header_size;
	size_t row_size = NUM_CHANNELS * w;
	size_t row_pad = 4 - row_size % 4; // align rows to 32-bit boundaries
	if (row_pad == 4) { row_pad = 0; }
	size_t data_size = (row_size + row_pad) * h;
	size_t data_pad = 4 - data_size % 4;
	if (data_pad == 4) { data_pad = 0; }
	size_t image_size = data_size + data_pad;
	size_t file_size = header_size + image_size;
	float x_dpi, y_dpi;
	Fl::screen_dpi(x_dpi, y_dpi);
	size32_t x_ppm = (size32_t)(x_dpi * INCHES_PER_METER);
	size32_t y_ppm = (size32_t)(y_dpi * INCHES_PER_METER);
	uchar file_header[14] = {
		'B', 'M',         // magic number
		LE32(file_size),  // file size
		LE16(0),          // reserved 1 (unused)
		LE16(0),          // reserved 2 (unused)
		LE32(header_size) // header size
	};
	uchar info_header[40] = {
		LE32(info_header_size), // info header size
		LE32(w),                // image width
		LE32(h),                // image height
		LE16(1),                // num color planes
		LE16(24),               // bits per pixel
		LE32(0),                // compression method (RGB)
		LE32(image_size),       // image size in bytes
		LE32(x_ppm),            // horizontal pixels per meter
		LE32(y_ppm),            // vertical pixels per meter
		LE32(0),                // num colors (ignored)
		LE32(0)                 // num important colors (ignored)
	};
	// Write the BMP file header
	fwrite(&file_header, sizeof(file_header), 1, file);
	// Write the BMP info header
	fwrite(&info_header, sizeof(info_header), 1, file);
	// Write the BGR pixels in row-major order from bottom to top
	const char *buffer = img->data()[0];
	int d = img->d();
	int ld = img->ld();
	if (!ld) { ld = (int)w * d; }
	int pd = d > 1;
	for (size_t i = h; i-- > 0;) {
		for (size_t j = 0; j < w; j++) {
			size_t px = ld * i + d * j;
			for (size_t k = NUM_CHANNELS; k-- > 0;) {
				fputc(buffer[px+pd*k], file);
			}
		}
		// Pad the rows to the nearest 4 bytes
		for (size_t j = 0; j < row_pad; j++) {
			fputc(0, file);
		}
	}
	// Pad the pixel data to the nearest 4 bytes
	for (size_t i = 0; i < data_pad; i++) {
		fputc(0, file);
	}
	fclose(file);
	return Result::IMAGE_OK;
}

const char *Image::error_message(Result result) {
	switch (result) {
	case Result::IMAGE_OK:
		return "OK.";
	case Result::IMAGE_BAD_FILE:
		return "Cannot open file.";
	case Result::IMAGE_BAD_PNG:
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
