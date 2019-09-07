#include "tilemap-format.h"

int format_tileset_size(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::RLE_NYBBLES:
		// High nybble is reserved for run length
		return 0x10;
	case Tilemap_Format::XY_FLIP:
		// High two bits are reserved for X/Y flip
		return 0x40;
	case Tilemap_Format::FF_END:
	case Tilemap_Format::RLE_FF_END:
		// $FF is reserved for the end marker
		return 0xFF;
	default:
		return 0x100;
	}
}

const char *format_name(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::PLAIN:
		return "Plain tiles";
	case Tilemap_Format::TILE_ATTR:
		return "Tiles + attributes";
	case Tilemap_Format::RLE_NYBBLES:
		return "RBY Town Map";
	case Tilemap_Format::FF_END:
		return "GSC Town Map";
	case Tilemap_Format::XY_FLIP:
		return "PC Town Map";
	case Tilemap_Format::RLE_FF_END:
		return "Pok\xc3\xa9gear card";
	default:
		return "Any";
	}
}

const char *format_extension(Tilemap_Format fmt) {
	switch (fmt) {
	case Tilemap_Format::PLAIN:
	default:
		return ".tilemap"; // e.g. pokecrystal/gfx/card_flip/card_flip.tilemap
	case Tilemap_Format::RLE_NYBBLES:
		return ".rle"; // e.g. pokered/gfx/town_map.rle
	case Tilemap_Format::FF_END:
	case Tilemap_Format::XY_FLIP:
		return ".bin"; // e.g. pokecrystal/gfx/pokegear/*.bin, polishedcrystal/gfx/town_map/*.bin
	case Tilemap_Format::RLE_FF_END:
		return ".tilemap.rle"; // e.g. pokecrystal/gfx/pokegear/*.tilemap.bin
	case Tilemap_Format::TILE_ATTR:
		return ".map"; // e.g. pokered/gfx/{red|blue}/sgbborder.map
	}
}
