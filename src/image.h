#ifndef IMAGE_H
#define IMAGE_H

#pragma warning(push, 0)
#include <FL/Fl_RGB_Image.H>
#pragma warning(pop)

#define LE16(n) (uchar)((n) & 0xFF), (uchar)(((n) >> 8) & 0xFF)
#define LE32(n) (uchar)((n) & 0xFF), (uchar)(((n) >> 8) & 0xFF), (uchar)(((n) >> 16) & 0xFF), (uchar)(((n) >> 24) & 0xFF)

#define INCHES_PER_METER 39.3701

#define NUM_CHANNELS 3

class Image {
public:
	enum class Result { IMAGE_OK, IMAGE_BAD_FILE, IMAGE_BAD_PNG };
	enum class Type { IMAGE_TYPE_RGB, IMAGE_TYPE_2BPP, IMAGE_TYPE_4BPP, IMAGE_TYPE_8BPP };
	static Result write_image(const char *f, Fl_RGB_Image *img, Type type);
	static const char *error_message(Result result);
	static bool make_deimage(Fl_Widget *wgt);
private:
	static Result write_bmp_image(const char *f, Fl_RGB_Image *img, Type type);
	static Result write_png_image(const char *f, Fl_RGB_Image *img, Type type);
};

#endif
