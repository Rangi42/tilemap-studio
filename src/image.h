#ifndef IMAGE_H
#define IMAGE_H

#include "tilemap.h"

#define NUM_CHANNELS 3

class Image {
public:
	enum Result { IMAGE_OK, IMAGE_BAD_FILE, IMAGE_BAD_PNG };
	static Result write_tilemap_image(const char *f, const Tilemap &tilemap);
	static const char *error_message(Result result);
private:
	static Result write_image(const char *f, size_t w, size_t h, Fl_RGB_Image *img);
};

#endif
