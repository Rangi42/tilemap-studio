#ifndef PALETTES_H
#define PALETTES_H

#include <vector>

#pragma warning(push, 0)
#include <FL/Enumerations.H>
#pragma warning(pop)

typedef std::vector<Fl_Color> Palette;
typedef std::vector<Palette> Palettes;

#define MAX_PALETTE_LENGTH 256

#define NUM_PALETTE_FORMATS 15

enum class Palette_Format { INDEXED, RGB, JASC, ACT, ACO, ASE, RIFF, TXT, GPL, XML, JSON, MAP, HEX, PNG, BMP };

const char *palette_name(Palette_Format pal_fmt);
const char *palette_extension(Palette_Format pal_fmt);
int palette_max_name_width(void);
bool write_palette(const char *f, const Palettes &palettes, Palette_Format pal_fmt, size_t nc);
bool write_tilepal(const char *f, const std::vector<size_t> &tileset, const std::vector<int> &tile_palettes);

#endif
