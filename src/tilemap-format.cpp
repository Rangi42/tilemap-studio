#include <cstring>

#pragma warning(push, 0)
#include <FL/filename.H>
#pragma warning(pop)

#include "tilemap-format.h"
#include "config.h"
#include "utils.h"

static const int tileset_sizes[NUM_FORMATS] = {
	0x100, // PLAIN - 8-bit tile IDs
	0x200, // GBC_ATTRS - 9-bit tile IDs
	0x200, // GBC_ATTRMAP - 9-bit tile IDs
	0x400, // GBA_4BPP - 10-bit tile IDs
	0x400, // GBA_4BPP - 10-bit tile IDs
	0x100, // SGB_BORDER - 8-bit tile IDs
	0x400, // SNES_ATTRS - 10-bit tile IDs
	0x10,  // RBY_TOWN_MAP - High nybble is reserved for run length
	0xFF,  // GSC_TOWN_MAP - $FF is reserved for the end marker
	0x40,  // PC_TOWN_MAP - High two bits are reserved for X/Y flip
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
		return 16;
	case Tilemap_Format::SGB_BORDER:
		return 4;
	case Tilemap_Format::PLAIN:
	case Tilemap_Format::GBA_8BPP:
		return 1;
	case Tilemap_Format::RBY_TOWN_MAP:
	case Tilemap_Format::POKEGEAR_CARD:
	default:
		return 0;
	}
}

int format_palette_size(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::PLAIN:
	case Tilemap_Format::GBA_8BPP:
		return 256;
	case Tilemap_Format::GBA_4BPP:
	case Tilemap_Format::SNES_ATTRS:
		return 16;
	case Tilemap_Format::GBC_ATTRS:
	case Tilemap_Format::GBC_ATTRMAP:
	case Tilemap_Format::SGB_BORDER:
	case Tilemap_Format::GSC_TOWN_MAP:
	case Tilemap_Format::PC_TOWN_MAP:
		return 4;
	case Tilemap_Format::RBY_TOWN_MAP:
	case Tilemap_Format::POKEGEAR_CARD:
	default:
		return 0;
	}
}

int format_color_depth(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::PLAIN:
	case Tilemap_Format::GBA_8BPP:
		return 8;
	case Tilemap_Format::GBA_4BPP:
	case Tilemap_Format::SNES_ATTRS:
		return 4;
	case Tilemap_Format::GBC_ATTRS:
	case Tilemap_Format::GBC_ATTRMAP:
	case Tilemap_Format::SGB_BORDER:
	case Tilemap_Format::GSC_TOWN_MAP:
	case Tilemap_Format::PC_TOWN_MAP:
		return 2;
	case Tilemap_Format::RBY_TOWN_MAP:
	case Tilemap_Format::POKEGEAR_CARD:
	default:
		return 0;
	}
}

static const char *format_names[NUM_FORMATS] = {
	"Plain tiles",               // PLAIN
	"GBC tiles + attributes",    // GBC_ATTRS
	"GBC tilemap + attrmap",     // GBC_ATTRMAP
	"GBA tiles + 4bpp palettes", // GBA_4BPP
	"GBA tiles + 8bpp palettes", // GBA_8BPP
	"SGB border",                // SGB_BORDER
	"SNES tiles + attributes",   // SNES_ATTRS
	"RBY Town Map",              // RBY_TOWN_MAP
	"GSC Town Map",              // GSC_TOWN_MAP
	"PC Town Map",               // PC_TOWN_MAP
	"Pok\xc3\xa9gear card",      // POKEGEAR_CARD
};

const char *format_name(Tilemap_Format fmt) {
	return format_names[(int)fmt];
}

int format_max_name_width() {
	int mw = 0;
	for (int i = 0; i < NUM_FORMATS; i++) {
		int fw = text_width(format_names[i], 6);
		if (fw > mw) { mw = fw; }
	}
	return mw;
}

static const char *format_extensions[NUM_FORMATS] = {
	".tilemap",     // PLAIN - e.g. pokecrystal/gfx/card_flip/card_flip.tilemap
	".bin",         // GBC_ATTRS - e.g. pokecrystal/gfx/mobile/*.bin
	".tilemap",     // GBC_ATTRMAP - e.g. pokecrystal/gfx/mobile/*.{tilemap|attrmap}
	".bin",         // GBA_4BPP - e.g. {pokeruby|pokeemerald}/graphics/*/*.bin
	".bin",         // GBA_8BPP - e.g. {pokeruby|pokeemerald}/graphics/*/*.bin
	".map",         // SGB_BORDER - e.g. pokered/gfx/{red|blue}/sgbborder.map
	".bin",         // SNES_ATTRS
	".rle",         // RBY_TOWN_MAP - e.g. pokered/gfx/town_map.rle
	".bin",         // GSC_TOWN_MAP - e.g. pokecrystal/gfx/pokegear/*.bin
	".bin",         // PC_TOWN_MAP - e.g. polishedcrystal/gfx/town_map/*.bin
	".tilemap.rle", // POKEGEAR_CARD - e.g. pokecrystal/gfx/pokegear/*.tilemap.rle
};

const char *format_extension(Tilemap_Format fmt) {
	return format_extensions[(int)fmt];
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
	if (starts_with(s, "sgb") || fs == SGB_WIDTH * SGB_HEIGHT * 2 ||
		fs == SGB_WIDTH * SGB_HEIGHT * 2 - GAME_BOY_WIDTH * GAME_BOY_HEIGHT * 2) {
		return Tilemap_Format::SGB_BORDER;
	}
	if (ends_with(s, ".tilemap.rle")) {
		return Tilemap_Format::POKEGEAR_CARD;
	}
	if (ends_with(s, ".rle")) {
		return Tilemap_Format::RBY_TOWN_MAP;
	}
	if (s == "johto.bin" || s == "kanto.bin" ||
		fs == GAME_BOY_WIDTH * GAME_BOY_HEIGHT + 1) {
		return Tilemap_Format::GSC_TOWN_MAP;
	}
	if (ends_with(s, ".kmp") || fs % 2 ||
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
		fmt == Tilemap_Format::GBA_8BPP || fmt == Tilemap_Format::SNES_ATTRS) {
		return fmt;
	}
	return Tilemap_Format::PLAIN;
}
