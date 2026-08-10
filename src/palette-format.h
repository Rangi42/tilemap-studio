#ifndef PALETTES_H
#define PALETTES_H

#include <string>
#include <vector>

#pragma warning(push, 0)
#include <FL/Enumerations.H>
#include <FL/fl_types.h>
#pragma warning(pop)

typedef std::vector<Fl_Color> Palette;
typedef std::vector<Palette> Palettes;

#define MAX_PALETTE_LENGTH 256

#define NUM_PALETTE_FORMATS 16

enum class Palette_Format { INDEXED, PNG, BMP, RGB, JASC, ACT, ACO, ASE, COL, RIFF, TXT, GPL, XML, JSON, MAP, HEX };

enum class Palette_Result { PALETTE_OK, PALETTE_BAD_FILE, PALETTE_BAD_EXT, PALETTE_EMPTY, PALETTE_UNSUPPORTED };

const char *palette_name(Palette_Format pal_fmt);
const char *palette_extension(Palette_Format pal_fmt);
int palette_max_name_width(void);
bool write_palette(const char *f, const Palettes &palettes, Palette_Format pal_fmt, size_t nc);
bool write_tilepal(const char *f, const std::vector<size_t> &tileset, const std::vector<int> &tile_palettes);

// Read a palette file into palettes, split into groups of palette_size colors (capped at palettes_size groups).
// The format is detected from the filename extension, with content-sniffing for the ambiguous .pal extension.
// If detected is non-null, it is set to the format to write the file back as, but only for formats that
// write_palette() supports (it is left untouched for read-only formats such as binary BGR555 .pal/.gbapal).
Palette_Result read_palette(const char *f, Palettes &palettes, size_t palettes_size, size_t palette_size,
	Palette_Format *detected = nullptr);
const char *palette_error_message(Palette_Result result);

// FLTK-free content parsers, exposed for testing. Each fills rgb with flat r,g,b,r,g,b... triples.
namespace Palette_Parse {
	bool jasc(const std::string &text, std::vector<uchar> &rgb);
	bool bgr555(const std::vector<uchar> &data, std::vector<uchar> &rgb);
	bool act(const std::vector<uchar> &data, std::vector<uchar> &rgb);
	bool hex(const std::string &text, std::vector<uchar> &rgb);
	bool gpl(const std::string &text, std::vector<uchar> &rgb);
	bool paint_net(const std::string &text, std::vector<uchar> &rgb);
	bool fractint_map(const std::string &text, std::vector<uchar> &rgb);
	bool rgb_asm(const std::string &text, std::vector<uchar> &rgb);
	bool looks_like_text(const std::vector<uchar> &data);
}

#endif
