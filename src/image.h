#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_RGB_Image.H>
#pragma warning(pop)

#define LE16(n) (uchar)((n) & 0xFF), (uchar)(((n) >> 8) & 0xFF)
#define LE32(n) (uchar)((n) & 0xFF), (uchar)(((n) >> 8) & 0xFF), (uchar)(((n) >> 16) & 0xFF), (uchar)(((n) >> 24) & 0xFF)

#define MAX_PALETTE_LENGTH 256

#define INCHES_PER_METER 39.3701

#define NUM_CHANNELS 3

typedef std::vector<Fl_Color> Palette;

class Image {
public:
	enum class Result { IMAGE_OK, IMAGE_BAD_FILE, IMAGE_BAD_PALETTE, IMAGE_BAD_PNG };
	static Result write_image(const char *f, Fl_RGB_Image *img, int bpp = 0,
		const std::vector<Palette> *palettes = NULL, size_t max_colors = 0);
	static const char *error_message(Result result);
	static bool make_deimage(Fl_Widget *wgt);
	static Fl_Color get_indexed_grayscale(size_t i, size_t nc);
private:
	static Result write_bmp_image(const char *f, Fl_RGB_Image *img, int bpp,
		const std::vector<Palette> *palettes, size_t max_colors);
	static Result write_png_image(const char *f, Fl_RGB_Image *img, int bpp,
		const std::vector<Palette> *palettes, size_t max_colors);
};

#endif
