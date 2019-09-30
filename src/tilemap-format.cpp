#pragma warning(push, 0)
#include <FL/filename.H>
#pragma warning(pop)

#include "tilemap-format.h"
#include "config.h"
#include "utils.h"

static const int tileset_sizes[NUM_FORMATS] = {
	0x100, // PLAIN - 8-bit tile IDs
	0x200, // GBC_ATTRS - 9-bit tile IDs
	0x400, // GBA_PALETTES - 10-bit tile IDs
	0x100, // SGB_BORDER - 8-bit tile IDs
	0x10,  // RBY_TOWN_MAP - High nybble is reserved for run length
	0xFF,  // GSC_TOWN_MAP - $FF is reserved for the end marker
	0x40,  // PC_TOWN_MAP - High two bits are reserved for X/Y flip
	0xFF,  // POKEGEAR_CARD - $FF is reserved for the end marker
};

int format_tileset_size(Tilemap_Format fmt) {
	return tileset_sizes[fmt];
}

int format_palettes_size(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::GBC_ATTRS:
		return 8;
	case Tilemap_Format::GBA_PALETTES:
		return 16;
	case Tilemap_Format::SGB_BORDER:
		return 4;
	default:
		return 0;
	}
}

static const char *format_names[NUM_FORMATS] = {
	"Plain tiles",            // PLAIN
	"GBC tiles + attributes", // GBC_ATTRS
	"GBA tiles + palettes",   // GBA_PALETTES
	"SGB border",             // SGB_BORDER
	"RBY Town Map",           // RBY_TOWN_MAP
	"GSC Town Map",           // GSC_TOWN_MAP
	"PC Town Map",            // PC_TOWN_MAP
	"Pok\xc3\xa9gear card",   // POKEGEAR_CARD
};

const char *format_name(Tilemap_Format fmt) {
	return format_names[fmt];
}

static const char *format_extensions[NUM_FORMATS] = {
	".tilemap",     // PLAIN - e.g. pokecrystal/gfx/card_flip/card_flip.tilemap
	".bin",         // GBC_ATTRS - e.g. pokecrystal/gfx/mobile/*.bin
	".bin",         // GBA_PALETTES  e.g. {pokeruby|pokeemerald}/graphics/*/*.bin
	".map",         // SGB_BORDER - e.g. pokered/gfx/{red|blue}/sgbborder.map
	".rle",         // RBY_TOWN_MAP - e.g. pokered/gfx/town_map.rle
	".bin",         // GSC_TOWN_MAP - e.g. pokecrystal/gfx/pokegear/*.bin
	".bin",         // PC_TOWN_MAP - e.g. polishedcrystal/gfx/town_map/*.bin
	".tilemap.rle", // POKEGEAR_CARD - e.g. pokecrystal/gfx/pokegear/*.tilemap.rle
};

const char *format_extension(Tilemap_Format fmt) {
	return format_extensions[fmt];
}

Tilemap_Format guess_format(const char *filename) {
	size_t fs = file_size(filename);
	const char *basename = fl_filename_name(filename);
	std::string s(basename);
	if (starts_with(s, "sgb") || fs == SGB_WIDTH * SGB_HEIGHT * 2 ||
		fs == SGB_WIDTH * SGB_HEIGHT * 2 - GAME_BOY_WIDTH * GAME_BOY_HEIGHT * 2) {
		// e.g. pokered/gfx/{red|blue}/sgbborder.map
		// e.g. pokecrystal/gfx/sgb/sgb_border.bin
		return Tilemap_Format::SGB_BORDER;
	}
	if (ends_with(s, ".rle")) {
		// e.g. pokered/gfx/town_map.rle
		return Tilemap_Format::RBY_TOWN_MAP;
	}
	if (ends_with(s, ".kmp") || s == "johto.bin" || s == "kanto.bin" ||
		fs == GAME_BOY_WIDTH * GAME_BOY_HEIGHT + 1) {
		// e.g. pokecrystal/gfx/pokegear/*.bin
		return Tilemap_Format::GSC_TOWN_MAP;
	}
	if (ends_with(s, ".tilemap.rle")) {
		// e.g. pokecrystal/gfx/pokegear/*.tilemap.rle
		return Tilemap_Format::POKEGEAR_CARD;
	}
	if (fs % 2) {
		return Tilemap_Format::PLAIN;
	}
	if (fs == GBA_WIDTH * GBA_HEIGHT * 2 || fs == GAME_BOY_VRAM_SIZE * GBA_HEIGHT * 2 ||
		fs > GAME_BOY_VRAM_SIZE * GAME_BOY_VRAM_SIZE * 2) {
		return Tilemap_Format::GBA_PALETTES;
	}
	if (fs >= GAME_BOY_WIDTH * GAME_BOY_HEIGHT * 2 && fs < GAME_BOY_VRAM_SIZE * GBA_HEIGHT * 2) {
		return Tilemap_Format::GBC_ATTRS;
	}
	return Config::format();
}
