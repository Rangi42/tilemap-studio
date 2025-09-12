#include <cstring>

#pragma warning(push, 0)
#include <FL/filename.H>
#pragma warning(pop)

#include "tilemap-format.h"
#include "tile-buttons.h"
#include "config.h"
#include "utils.h"

static const int tileset_sizes[NUM_FORMATS] = {
	0x100, // PLAIN - 8-bit tile IDs
	0x200, // GBC_ATTRS - 9-bit tile IDs
	0x200, // GBC_ATTRMAP - 9-bit tile IDs
	0x400, // GBA_4BPP - 10-bit tile IDs
	0x400, // GBA_8BPP - 10-bit tile IDs
	0x400, // NDS_4BPP - 10-bit tile IDs
	0x400, // NDS_8BPP - 10-bit tile IDs
	0x100, // SGB_BORDER - 8-bit tile IDs
	0x400, // SNES_ATTRS - 10-bit tile IDs
	0x800, // GENESIS - 11-bit tile IDs
	0x800, // TG16 - 11-bit tile IDs
	0x10,  // RBY_TOWN_MAP - High nybble is reserved for run length
	0xFF,  // GSC_TOWN_MAP - $FF is reserved for the end marker
	0x40,  // PC_TOWN_MAP - High two bits are reserved for X/Y flip
	0x100, // SW_TOWN_MAP - 8-bit tile IDs (but $00 is reserved for the end marker)
	0xFF,  // POKEGEAR_CARD - $FF is reserved for the end marker
};

int format_tileset_size(Tilemap_Format fmt) {
	return tileset_sizes[(int)fmt];
}

int format_palettes_size(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::GBC_ATTRS:
	case Tilemap_Format::GBC_ATTRMAP:
	case Tilemap_Format::SNES_ATTRS:
	case Tilemap_Format::GSC_TOWN_MAP:
	case Tilemap_Format::PC_TOWN_MAP:
		return 8;
	case Tilemap_Format::GBA_4BPP:
	case Tilemap_Format::NDS_4BPP:
	case Tilemap_Format::GENESIS:
	case Tilemap_Format::TG16:
		return 16;
	case Tilemap_Format::SGB_BORDER:
		return 4;
	case Tilemap_Format::PLAIN:
	case Tilemap_Format::GBA_8BPP:
	case Tilemap_Format::NDS_8BPP:
		return 1;
	case Tilemap_Format::RBY_TOWN_MAP:
	case Tilemap_Format::SW_TOWN_MAP:
	case Tilemap_Format::POKEGEAR_CARD:
	default:
		return 0;
	}
}

int format_palette_size(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::PLAIN:
	case Tilemap_Format::GBA_8BPP:
	case Tilemap_Format::NDS_8BPP:
		return 256;
	case Tilemap_Format::GBA_4BPP:
	case Tilemap_Format::NDS_4BPP:
	case Tilemap_Format::SNES_ATTRS:
	case Tilemap_Format::TG16:
		return 16;
	case Tilemap_Format::GBC_ATTRS:
	case Tilemap_Format::GBC_ATTRMAP:
	case Tilemap_Format::GENESIS:
	case Tilemap_Format::SGB_BORDER:
	case Tilemap_Format::GSC_TOWN_MAP:
	case Tilemap_Format::PC_TOWN_MAP:
		return 4;
	case Tilemap_Format::RBY_TOWN_MAP:
	case Tilemap_Format::SW_TOWN_MAP:
	case Tilemap_Format::POKEGEAR_CARD:
	default:
		return 0;
	}
}

int format_color_depth(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::PLAIN:
	case Tilemap_Format::GBA_8BPP:
	case Tilemap_Format::NDS_8BPP:
		return 8;
	case Tilemap_Format::GBA_4BPP:
	case Tilemap_Format::NDS_4BPP:
	case Tilemap_Format::SNES_ATTRS:
	case Tilemap_Format::GENESIS:
	case Tilemap_Format::TG16:
		return 4;
	case Tilemap_Format::GBC_ATTRS:
	case Tilemap_Format::GBC_ATTRMAP:
	case Tilemap_Format::SGB_BORDER:
	case Tilemap_Format::GSC_TOWN_MAP:
	case Tilemap_Format::PC_TOWN_MAP:
		return 2;
	case Tilemap_Format::RBY_TOWN_MAP:
	case Tilemap_Format::SW_TOWN_MAP:
	case Tilemap_Format::POKEGEAR_CARD:
	default:
		return 0;
	}
}

static const char *format_names[NUM_FORMATS] = {
	"Plain tiles",                // PLAIN
	"GBC tiles + attributes",     // GBC_ATTRS
	"GBC tilemap + attrmap",      // GBC_ATTRMAP
	"GBA tiles + 4bpp palettes",  // GBA_4BPP
	"GBA tiles + 8bpp palette",   // GBA_8BPP
	"NDS tiles + 4bpp palettes",  // NDS_4BPP
	"NDS tiles + 8bpp palette",   // NDS_8BPP
	"SGB border",                 // SGB_BORDER
	"SNES tiles + attributes",    // SNES_ATTRS
	"Genesis tiles + attributes", // GENESIS
	"TG16 tiles + palettes",      // TG16
	"RBY Town Map",               // RBY_TOWN_MAP
	"GSC Town Map",               // GSC_TOWN_MAP
	"PC Town Map",                // PC_TOWN_MAP
	"SW Town Map",                // SW_TOWN_MAP
	"Pok\xc3\xa9gear card",       // POKEGEAR_CARD
};

const char *format_name(Tilemap_Format fmt) {
	return format_names[(int)fmt];
}

int format_max_name_width() {
	int mw = 0;
	for (const char *fmt_name : format_names) {
		mw = std::max(mw, text_width(fmt_name, 6));
	}
	return mw;
}

static const char *format_extensions[NUM_FORMATS] = {
	".tilemap",     // PLAIN - e.g. pokecrystal/gfx/card_flip/card_flip.tilemap
	".bin",         // GBC_ATTRS - e.g. pokecrystal/gfx/mobile/*.bin
	".tilemap",     // GBC_ATTRMAP - e.g. pokecrystal/gfx/mobile/*.{tilemap|attrmap}
	".bin",         // GBA_4BPP - e.g. {pokeruby|pokeemerald}/graphics/*/*.bin
	".bin",         // GBA_8BPP - e.g. {pokeruby|pokeemerald}/graphics/*/*.bin
	".rcsn",        // NDS_4BPP - extracted by Tinke <https://github.com/pleonex/tinke>
	".rcsn",        // NDS_8BPP - extracted by Tinke <https://github.com/pleonex/tinke>
	".map",         // SGB_BORDER - e.g. pokered/gfx/{red|blue}/sgbborder.map
	".bin",         // SNES_ATTRS
	".bin",         // GENESIS <http://dreamjam.co.uk/emuviews/txt/genvdp.txt>
	".map",         // TG16 <https://www.chibiakumas.com/6502/pcengine.php>
	".rle",         // RBY_TOWN_MAP - e.g. pokered/gfx/town_map.rle
	".bin",         // GSC_TOWN_MAP - e.g. pokecrystal/gfx/pokegear/*.bin
	".bin",         // PC_TOWN_MAP - e.g. polishedcrystal/gfx/town_map/*.bin
	".tilemap.rle", // SW_TOWN_MAP - e.g. pokegold-spaceworld/gfx/trainer_gear/town_map.tilemap.rle
	".tilemap.rle", // POKEGEAR_CARD - e.g. pokecrystal/gfx/pokegear/*.tilemap.rle
};

const char *format_extension(Tilemap_Format fmt) {
	return format_extensions[(int)fmt];
}

int format_bytes_per_tile(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::PLAIN:
	case Tilemap_Format::GSC_TOWN_MAP:
	case Tilemap_Format::PC_TOWN_MAP:
	case Tilemap_Format::GBC_ATTRMAP:
		return 1;
	case Tilemap_Format::GBC_ATTRS:
	case Tilemap_Format::GBA_4BPP:
	case Tilemap_Format::GBA_8BPP:
	case Tilemap_Format::NDS_4BPP:
	case Tilemap_Format::NDS_8BPP:
	case Tilemap_Format::SGB_BORDER:
	case Tilemap_Format::SNES_ATTRS:
	case Tilemap_Format::GENESIS:
	case Tilemap_Format::TG16:
		return 2;
	case Tilemap_Format::RBY_TOWN_MAP:
	case Tilemap_Format::SW_TOWN_MAP:
	case Tilemap_Format::POKEGEAR_CARD:
	default:
		return 0;
	}
}

Tilemap_Format guess_format(const char *filename) {
	size_t fs = file_size(filename);
	const char *basename = fl_filename_name(filename);
	std::string s(basename);
	char attrmap_name[FL_PATH_MAX] = {};
	strcpy(attrmap_name, filename);
	fl_filename_setext(attrmap_name, sizeof(attrmap_name), ATTRMAP_EXT);

	if (file_exists(attrmap_name)) {
		return Tilemap_Format::GBC_ATTRMAP;
	}
	if (starts_with_ignore_case(s, "sgb") || fs == SGB_WIDTH * SGB_HEIGHT * 2 ||
		fs == SGB_WIDTH * SGB_HEIGHT * 2 - GAME_BOY_WIDTH * GAME_BOY_HEIGHT * 2) {
		return Tilemap_Format::SGB_BORDER;
	}
	if (ends_with_ignore_case(s, ".tilemap.rle")) {
		return Tilemap_Format::POKEGEAR_CARD;
	}
	if (ends_with_ignore_case(s, ".rle")) {
		return Tilemap_Format::RBY_TOWN_MAP;
	}
	if (s == "johto.bin" || s == "kanto.bin" ||
		fs == GAME_BOY_WIDTH * GAME_BOY_HEIGHT + 1) {
		return Tilemap_Format::GSC_TOWN_MAP;
	}
	if (ends_with_ignore_case(s, ".rcsn") || ends_with_ignore_case(s, ".nscr") || fs == 0x624 || fs == 0x824) {
		return Tilemap_Format::NDS_4BPP;
	}
	if (ends_with_ignore_case(s, ".kmp") || fs % 2 ||
		fs > GAME_BOY_VRAM_SIZE * GAME_BOY_VRAM_SIZE * 2) {
		return Tilemap_Format::PLAIN;
	}
	if (fs == GBA_WIDTH * GBA_HEIGHT * 2 ||
		fs == GAME_BOY_VRAM_SIZE * GBA_HEIGHT * 2) {
		return Tilemap_Format::GBA_4BPP;
	}
	if (fs >= GAME_BOY_WIDTH * GAME_BOY_HEIGHT * 2 &&
		fs < GAME_BOY_VRAM_SIZE * GBA_HEIGHT * 2) {
		return Tilemap_Format::GBC_ATTRS;
	}
	Tilemap_Format fmt = Config::format();
	if (fmt == Tilemap_Format::SGB_BORDER || fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBA_4BPP ||
		fmt == Tilemap_Format::GBA_8BPP || fmt == Tilemap_Format::NDS_4BPP || fmt == Tilemap_Format::NDS_8BPP ||
		fmt == Tilemap_Format::SNES_ATTRS || fmt == Tilemap_Format::GENESIS || fmt == Tilemap_Format::TG16) {
		return fmt;
	}
	return Tilemap_Format::PLAIN;
}

std::vector<uchar> make_tilemap_bytes(const std::vector<Tile_Tessera *> &tiles, Tilemap_Format fmt, size_t width, size_t height) {
	std::vector<uchar> bytes;
	size_t n = tiles.size();

	if (fmt == Tilemap_Format::PLAIN || fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP) {
		bytes.reserve(n + 1);
		for (Tile_Tessera *tt : tiles) {
			uchar v = (uchar)tt->id();
			if (tt->x_flip()) { v |= 0x40; }
			if (tt->y_flip()) { v |= 0x80; }
			bytes.push_back(v);
		}
	}
	else if (fmt == Tilemap_Format::GBC_ATTRS) {
		bytes.reserve(n * 2);
		for (Tile_Tessera *tt : tiles) {
			uchar v = (uchar)(tt->id() & 0xFF);
			bytes.push_back(v);
			uchar a = 0;
			if (tt->id() & 0x100) { a |= 0x08; }
			if (tt->obp1())     { a |= 0x10; }
			if (tt->priority()) { a |= 0x80; }
			if (tt->x_flip())   { a |= 0x20; }
			if (tt->y_flip())   { a |= 0x40; }
			if (tt->palette() > -1) { a |= tt->palette() & 0x07; }
			bytes.push_back(a);
		}
	}
	else if (fmt == Tilemap_Format::GBC_ATTRMAP) {
		bytes.reserve(n * 2);
		for (Tile_Tessera *tt : tiles) {
			uchar v = (uchar)(tt->id() & 0xFF);
			bytes.push_back(v);
		}
		for (Tile_Tessera *tt : tiles) {
			uchar a = 0;
			if (tt->id() & 0x100) { a |= 0x08; }
			if (tt->obp1())     { a |= 0x10; }
			if (tt->priority()) { a |= 0x80; }
			if (tt->x_flip())   { a |= 0x20; }
			if (tt->y_flip())   { a |= 0x40; }
			if (tt->palette() > -1) { a |= tt->palette() & 0x07; }
			bytes.push_back(a);
		}
	}
	else if (fmt == Tilemap_Format::GBA_4BPP || fmt == Tilemap_Format::GBA_8BPP ||
		fmt == Tilemap_Format::NDS_4BPP || fmt == Tilemap_Format::NDS_8BPP) {
		if (fmt == Tilemap_Format::GBA_4BPP || fmt == Tilemap_Format::GBA_8BPP) {
			bytes.reserve(n * 2);
		}
		else {
			// <https://www.romhacking.net/documents/[469]nds_formats.htm#NSCR>
			bytes.reserve(std::max(n * 2 + NDS_HEADER_SIZE, (size_t)NDS_HEADER_SIZE));
			uchar header[NDS_HEADER_SIZE] = {
				// Generic header
				'R', 'C', 'S', 'N', // magic number
				0xFF, 0xFE, 0, 1,   // constant 0xFFFE0001
				LE32(n * 2 + 0x24), // section size
				LE16(0x10),         // header size
				LE16(1),            // number of sub-sections
				// Nintendo Screen Resource header
				'N', 'R', 'C', 'S', // magic number
				LE32(n * 2 + 0x14), // sub-section size
				LE16(width * 8),    // width in pixels
				LE16(height * 8),   // height in pixels
				0, 0, 0, 0,         // padding
				LE32(n * 2)         // screen data size
			};
			bytes.insert(bytes.begin(), RANGE(header));
		}
		for (Tile_Tessera *tt : tiles) {
			uchar v = (uchar)(tt->id() & 0xFF);
			bytes.push_back(v);
			uchar a = (tt->id() >> 8) & 0x03;
			if (tt->x_flip()) { a |= 0x04; }
			if (tt->y_flip()) { a |= 0x08; }
			if (tt->palette() > -1) { a |= (tt->palette() << 4) & 0xF0; }
			bytes.push_back(a);
		}
	}
	else if (fmt == Tilemap_Format::GENESIS) {
		bytes.reserve(n * 2);
		for (Tile_Tessera *tt : tiles) {
			uchar a = (tt->id() >> 8) & 0x07;
			if (tt->priority()) { a |= 0x80; }
			if (tt->x_flip())   { a |= 0x08; }
			if (tt->y_flip())   { a |= 0x10; }
			if (tt->palette() > -1) { a |= (tt->palette() << 5) & 0x60; }
			bytes.push_back(a);
			uchar v = (uchar)(tt->id() & 0xFF);
			bytes.push_back(v);
		}
	}
	else if (fmt == Tilemap_Format::TG16) {
		bytes.reserve(n * 2);
		for (Tile_Tessera *tt : tiles) {
			uchar v = (uchar)(tt->id() & 0xFF);
			bytes.push_back(v);
			uchar a = (tt->id() >> 8) & 0x07;
			if (tt->palette() > -1) { a |= (tt->palette() << 4) & 0xF0; }
			bytes.push_back(a);
		}
	}
	else if (fmt == Tilemap_Format::SGB_BORDER) {
		bytes.reserve(n * 2);
		for (Tile_Tessera *tt : tiles) {
			uchar v = (uchar)(tt->id() & 0xFF);
			bytes.push_back(v);
			uchar a = 0x10;
			if (tt->x_flip()) { a |= 0x40; }
			if (tt->y_flip()) { a |= 0x80; }
			if (tt->palette() > -1) { a |= (tt->palette() << 2) & 0x0C; }
			bytes.push_back(a);
		}
	}
	else if (fmt == Tilemap_Format::SNES_ATTRS) {
		bytes.reserve(n * 2);
		for (Tile_Tessera *tt : tiles) {
			uchar v = (uchar)(tt->id() & 0xFF);
			bytes.push_back(v);
			uchar a = (tt->id() >> 8) & 0x03;
			if (tt->priority()) { a |= 0x20; }
			if (tt->x_flip())   { a |= 0x40; }
			if (tt->y_flip())   { a |= 0x80; }
			if (tt->palette() > -1) { a |= (tt->palette() << 2) & 0x1C; }
			bytes.push_back(a);
		}
	}
	else if (fmt == Tilemap_Format::RBY_TOWN_MAP) {
		bytes.reserve(n);
		for (size_t i = 0; i < n;) {
			Tile_Tessera *tt = tiles[i++];
			uchar v = (uchar)tt->id(), r = 1;
			while (i < n && (uchar)tiles[i]->id() == v) {
				i++;
				if (++r == 0x0F) { break; } // maximum nybble
			}
			uchar b = (v << 4) | r;
			bytes.push_back(b);
		}
	}
	else if (fmt == Tilemap_Format::POKEGEAR_CARD || fmt == Tilemap_Format::SW_TOWN_MAP) {
		bytes.reserve(n + 1);
		for (size_t i = 0; i < n;) {
			Tile_Tessera *tt = tiles[i++];
			uchar v = (uchar)tt->id(), r = 1;
			while (i < n && (uchar)tiles[i]->id() == v) {
				i++;
				if (++r == 0xFF) { break; } // maximum byte
			}
			bytes.push_back(v);
			bytes.push_back(r);
		}
	}

	if (fmt == Tilemap_Format::RBY_TOWN_MAP || fmt == Tilemap_Format::SW_TOWN_MAP) {
		bytes.push_back(0x00);
	}
	else if (fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP || fmt == Tilemap_Format::POKEGEAR_CARD) {
		bytes.push_back(0xFF);
	}

	return bytes;
}
