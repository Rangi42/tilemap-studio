#ifndef TILEMAP_FORMAT_H
#define TILEMAP_FORMAT_H

#include <vector>

#pragma warning(push, 0)
#include <FL/fl_types.h>
#pragma warning(pop)

#define GAME_BOY_WIDTH 20
#define GAME_BOY_HEIGHT 18

#define SGB_WIDTH 32
#define SGB_HEIGHT 28

#define GBA_WIDTH 30
#define GBA_HEIGHT 20

#define NDS_WIDTH 32
#define NDS_HEADER_SIZE 0x24

#define GAME_BOY_VRAM_SIZE 32

#define ATTRMAP_EXT ".attrmap"

#define TILEPAL_EXT ".asm"

#define NUM_FORMATS 16

enum class Tilemap_Format { PLAIN, GBC_ATTRS, GBC_ATTRMAP, GBA_4BPP, GBA_8BPP, NDS_4BPP, NDS_8BPP, SGB_BORDER, SNES_ATTRS,
	GENESIS, TG16, RBY_TOWN_MAP, GSC_TOWN_MAP, PC_TOWN_MAP, SW_TOWN_MAP, POKEGEAR_CARD };

inline constexpr bool format_has_landmarks(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP;
}

inline constexpr bool format_has_emaps(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::RBY_TOWN_MAP;
}

inline constexpr bool format_has_attrmap(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GBC_ATTRMAP;
}

inline constexpr bool format_can_make_palettes(Tilemap_Format fmt) {
	return fmt != Tilemap_Format::RBY_TOWN_MAP && fmt != Tilemap_Format::SW_TOWN_MAP && fmt != Tilemap_Format::POKEGEAR_CARD;
}

inline constexpr bool format_can_edit_palettes(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBC_ATTRMAP || fmt == Tilemap_Format::GBA_4BPP ||
		fmt == Tilemap_Format::NDS_4BPP || fmt == Tilemap_Format::SGB_BORDER || fmt == Tilemap_Format::SNES_ATTRS ||
		fmt == Tilemap_Format::GENESIS || fmt == Tilemap_Format::TG16;
}

inline constexpr bool format_has_per_tile_palettes(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP;
}

inline constexpr bool format_can_flip(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBC_ATTRMAP || fmt == Tilemap_Format::GBA_4BPP ||
		fmt == Tilemap_Format::GBA_8BPP || fmt == Tilemap_Format::NDS_4BPP || fmt == Tilemap_Format::NDS_8BPP ||
		fmt == Tilemap_Format::GENESIS || fmt == Tilemap_Format::SGB_BORDER || fmt == Tilemap_Format::SNES_ATTRS ||
		fmt == Tilemap_Format::PC_TOWN_MAP;
}

inline constexpr bool format_has_priority(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBC_ATTRMAP || fmt == Tilemap_Format::SNES_ATTRS ||
		fmt == Tilemap_Format::GENESIS;
}

inline constexpr bool format_has_obp1(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBC_ATTRMAP;
}

inline constexpr bool format_uses_rgb_asm_pal(Tilemap_Format fmt) {
	return fmt == Tilemap_Format::GBC_ATTRS || fmt == Tilemap_Format::GBC_ATTRMAP || fmt == Tilemap_Format::SGB_BORDER ||
		fmt == Tilemap_Format::RBY_TOWN_MAP || fmt == Tilemap_Format::GSC_TOWN_MAP || fmt == Tilemap_Format::PC_TOWN_MAP ||
		fmt == Tilemap_Format::SW_TOWN_MAP || fmt == Tilemap_Format::POKEGEAR_CARD;
}

int format_tileset_size(Tilemap_Format fmt);
int format_palettes_size(Tilemap_Format fmt);
int format_palette_size(Tilemap_Format fmt);
int format_color_depth(Tilemap_Format fmt);
const char *format_name(Tilemap_Format fmt);
const char *format_extension(Tilemap_Format fmt);
int format_max_name_width(void);
int format_bytes_per_tile(Tilemap_Format fmt);

Tilemap_Format guess_format(const char *filename);

class Tile_Tessera;

std::vector<uchar> make_tilemap_bytes(const std::vector<Tile_Tessera *> &tiles, Tilemap_Format fmt, size_t width, size_t height);

#endif
