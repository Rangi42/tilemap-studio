#ifndef TILEMAP_FORMAT_H
#define TILEMAP_FORMAT_H

#define NUM_FORMATS 8

#define GAME_BOY_WIDTH 20
#define GAME_BOY_HEIGHT 18

#define SGB_WIDTH 32
#define SGB_HEIGHT 28

#define GBA_WIDTH 30
#define GBA_HEIGHT 20

#define GAME_BOY_VRAM_SIZE 32

enum Tilemap_Format { PLAIN, GBC_ATTRS, GBA_PALETTES, SGB_BORDER, RBY_TOWN_MAP, GSC_TOWN_MAP, PC_TOWN_MAP, POKEGEAR_CARD };

inline bool format_has_landmarks(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP;
}

inline bool format_has_emaps(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::RBY_TOWN_MAP;
}

inline bool format_can_flip(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::SGB_BORDER || fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBA_PALETTES ||
		fmt == PC_TOWN_MAP;
}

inline bool format_has_palettes(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::SGB_BORDER || fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBA_PALETTES;
}

inline bool format_has_metadata(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GBC_ATTRS;
}

int format_tileset_size(Tilemap_Format fmt);
int format_palettes_size(Tilemap_Format fmt);
const char *format_name(Tilemap_Format fmt);
const char *format_extension(Tilemap_Format fmt);

Tilemap_Format guess_format(const char *filename);

#endif
