#ifndef THEMES_H
#define THEMES_H

#pragma warning(push, 0)
#include <FL/Enumerations.H>
#pragma warning(pop)

#include "utils.h"

#define OS_FONT FL_FREE_FONT

#ifdef _WIN32
#define OS_FONT_SIZE 12
#else
#define OS_FONT_SIZE 13
#endif

#define OS_BUTTON_UP_BOX FL_GTK_UP_BOX
#define OS_CHECK_DOWN_BOX FL_GTK_DOWN_BOX
#define OS_BUTTON_UP_FRAME FL_GTK_UP_FRAME
#define OS_CHECK_DOWN_FRAME FL_GTK_DOWN_FRAME
#define OS_PANEL_THIN_UP_BOX FL_GTK_THIN_UP_BOX
#define OS_SPACER_THIN_DOWN_BOX FL_GTK_THIN_DOWN_BOX
#define OS_PANEL_THIN_UP_FRAME FL_GTK_THIN_UP_FRAME
#define OS_SPACER_THIN_DOWN_FRAME FL_GTK_THIN_DOWN_FRAME
#define OS_RADIO_ROUND_DOWN_BOX FL_GTK_ROUND_DOWN_BOX
#define OS_HOVERED_UP_BOX FL_PLASTIC_UP_BOX
#define OS_DEPRESSED_DOWN_BOX FL_PLASTIC_DOWN_BOX
#define OS_HOVERED_UP_FRAME FL_PLASTIC_UP_FRAME
#define OS_DEPRESSED_DOWN_FRAME FL_PLASTIC_DOWN_FRAME
#define OS_INPUT_THIN_DOWN_BOX FL_PLASTIC_THIN_DOWN_BOX
#define OS_INPUT_THIN_DOWN_FRAME FL_PLASTIC_ROUND_DOWN_BOX
#define OS_MINI_BUTTON_UP_BOX FL_GLEAM_UP_BOX
#define OS_MINI_DEPRESSED_DOWN_BOX FL_GLEAM_DOWN_BOX
#define OS_MINI_BUTTON_UP_FRAME FL_GLEAM_UP_FRAME
#define OS_MINI_DEPRESSED_DOWN_FRAME FL_GLEAM_DOWN_FRAME
#define OS_DEFAULT_BUTTON_BOX FL_EMBOSSED_BOX
#define OS_TOOLBAR_BUTTON_HOVER_BOX FL_GLEAM_ROUND_UP_BOX
#define OS_SWATCH_FRAME FL_ENGRAVED_FRAME
#define OS_SWATCH_BOX FL_ENGRAVED_BOX

class OS {
public:
	enum Theme { CLASSIC, AERO, METRO, AQUA, GREYBIRD, METAL, BLUE, OLIVE, ROSE_GOLD, DARK };
private:
	static Theme _current_theme;
	static bool _is_consolas;
public:
#ifdef _WIN32
	static bool is_classic_windows(void);
	static bool is_modern_windows(void);
#endif
	inline static Theme current_theme(void) { return _current_theme; }
	inline static bool is_consolas(void) { return _is_consolas; }
	static void use_native_fonts(void);
	static void use_native_scheme(void);
	static void use_native_colors(void);
	static void use_native_theme(void);
	static void use_native_settings(void);
	static void use_classic_theme(void);
	static void use_aero_theme(void);
	static void use_metro_theme(void);
	static void use_aqua_theme(void);
	static void use_greybird_theme(void);
	static void use_metal_theme(void);
	static void use_blue_theme(void);
	static void use_olive_theme(void);
	static void use_rose_gold_theme(void);
	static void use_dark_theme(void);
};

#endif
