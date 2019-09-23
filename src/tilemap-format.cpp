#include "tilemap-format.h"

static const int tileset_sizes[NUM_FORMATS] = {
	0x100, // PLAIN - 8-bit tile IDs
	0x200, // GBC_ATTRS - 9-bit tile IDs
	0x400, // GBA_ATTRS - 10-bit tile IDs
	0x100, // SGB_BORDER - 8-bit tile IDs
	0x10,  // RBY_TOWN_MAP - High nybble is reserved for run length
	0xFF,  // GSC_TOWN_MAP - $FF is reserved for the end marker
	0x40,  // PC_TOWN_MAP - High two bits are reserved for X/Y flip
	0xFF,  // POKEGEAR_CARD - $FF is reserved for the end marker
};

int format_tileset_size(Tilemap_Format fmt) {
	return tileset_sizes[fmt];
}

static const char *format_names[NUM_FORMATS] = {
	"Plain tiles",            // PLAIN
	"GBC tiles + attributes", // GBC_ATTRS
	"GBA tiles + colors",     // GBA_ATTRS
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
	".bin",         // GBA_ATTRS  e.g. {pokeruby|pokeemerald}/graphics/*/*.bin
	".map",         // SGB_BORDER - e.g. pokered/gfx/{red|blue}/sgbborder.map
	".rle",         // RBY_TOWN_MAP - e.g. pokered/gfx/town_map.rle
	".bin",         // GSC_TOWN_MAP - e.g. pokecrystal/gfx/pokegear/*.bin
	".bin",         // PC_TOWN_MAP - e.g. polishedcrystal/gfx/town_map/*.bin
	".tilemap.rle", //  - e.g. pokecrystal/gfx/pokegear/*.tilemap.rle
};

const char *format_extension(Tilemap_Format fmt) {
	return format_extensions[fmt];
}
