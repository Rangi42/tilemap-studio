#include <iostream>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#pragma warning(pop)

#include "version.h"
#include "preferences.h"
#include "themes.h"
#include "main-window.h"

#ifdef _WIN32

#include <shlobj.h>
#include <tchar.h>

#define MAKE_WSTR_HELPER(x) L ## x
#define MAKE_WSTR(x) MAKE_WSTR_HELPER(x)

#endif

static void use_theme(OS::Theme theme) {
	OS::use_native_fonts();
	switch (theme) {
	case OS::Theme::CLASSIC:
		OS::use_classic_theme();
		return;
	case OS::Theme::AERO:
		OS::use_aero_theme();
		return;
	case OS::Theme::METRO:
		OS::use_metro_theme();
		return;
	case OS::Theme::AQUA:
		OS::use_aqua_theme();
		return;
	case OS::Theme::GREYBIRD:
		OS::use_greybird_theme();
		return;
	case OS::Theme::OCEAN:
		OS::use_ocean_theme();
		return;
	case OS::Theme::BLUE:
		OS::use_blue_theme();
		return;
	case OS::Theme::OLIVE:
		OS::use_olive_theme();
		return;
	case OS::Theme::ROSE_GOLD:
		OS::use_rose_gold_theme();
		return;
	case OS::Theme::DARK:
		OS::use_dark_theme();
		return;
	case OS::Theme::BRUSHED_METAL:
		OS::use_brushed_metal_theme();
		return;
	case OS::Theme::HIGH_CONTRAST:
		OS::use_high_contrast_theme();
	}
}

int main(int argc, char **argv) {
	Preferences::initialize(argv[0]);
	std::ios::sync_with_stdio(false);
#ifdef _WIN32
	SetCurrentProcessExplicitAppUserModelID(MAKE_WSTR(PROGRAM_AUTHOR) L"." MAKE_WSTR(PROGRAM_NAME));
#endif
	Fl::visual(FL_DOUBLE | FL_RGB);

#ifdef _WIN32
	OS::Theme theme = (OS::Theme)Preferences::get("theme", (int)OS::Theme::BLUE);
#else
	OS::Theme theme = (OS::Theme)Preferences::get("theme", (int)OS::Theme::GREYBIRD);
#endif
	use_theme(theme);

#ifdef _WIN32
	int x = Preferences::get("x", 48), y = Preferences::get("y", 48 + GetSystemMetrics(SM_CYCAPTION));
#else
	int x = Preferences::get("x", 48), y = Preferences::get("y", 48);
#endif
	int w = Preferences::get("w", 647), h = Preferences::get("h", 406);
	Main_Window window(x, y, w, h);
	window.show();
	if (window.full_screen()) {
		window.fullscreen();
	}

	if (argc > 1) {
		window.open_tilemap(argv[1]);
		if (argc > 2) {
			window.load_tileset(argv[2]);
		}
	}

	return Fl::run();
}
