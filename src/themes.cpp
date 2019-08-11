#include <cstdlib>
#include <cstring>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Tooltip.H>
#pragma warning(pop)

#ifdef _WIN32
#include <WinUser.h>
#endif

#include "utils.h"
#include "themes.h"

#ifdef _WIN32

bool OS::is_classic_windows() {
	// Return true for Windows XP and below, false for Windows 7 and above
	OSVERSIONINFOEX osvi;
	DWORDLONG cm = 0;
	char op = VER_LESS_EQUAL;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 5;
	osvi.dwMinorVersion = 1;
	osvi.wServicePackMajor = 0;
	osvi.wServicePackMinor = 0;
	VER_SET_CONDITION(cm, VER_MAJORVERSION, op);
	VER_SET_CONDITION(cm, VER_MINORVERSION, op);
	VER_SET_CONDITION(cm, VER_SERVICEPACKMAJOR, op);
	VER_SET_CONDITION(cm, VER_SERVICEPACKMINOR, op);
	return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR, cm) != FALSE;
}

bool OS::is_modern_windows() {
	// Return true for Windows 8 and above, false for Windows 7 and below
	OSVERSIONINFOEX osvi;
	DWORDLONG cm = 0;
	char op = VER_GREATER_EQUAL;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;
	osvi.wServicePackMajor = 0;
	osvi.wServicePackMinor = 0;
	VER_SET_CONDITION(cm, VER_MAJORVERSION, op);
	VER_SET_CONDITION(cm, VER_MINORVERSION, op);
	VER_SET_CONDITION(cm, VER_SERVICEPACKMAJOR, op);
	VER_SET_CONDITION(cm, VER_SERVICEPACKMINOR, op);
	return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR, cm) != FALSE;
}

#endif

static bool use_font(Fl_Font font, const char *system_name) {
	int num_fonts = Fl::set_fonts(NULL);
	for (Fl_Font f = 0; f < num_fonts; f++) {
		const char *name = Fl::get_font_name(f);
		if (!strcmp(name, system_name)) {
			Fl::set_font(font, name);
			return true;
		}
	}
	return false;
}

static int use_any_font(Fl_Font font, const char **font_names, int n) {
	for (int i = 0; i < n; i++) {
		if (use_font(font, font_names[i])) {
			return i;
		}
	}
	return -1;
}

static Fl_Color activated_color(Fl_Color c) {
	return Fl::draw_box_active() ? c : fl_inactive(c);
}

static Fl_Color devalued(Fl_Color c, float a) {
	return fl_color_average(FL_BLACK, c, a);
}

static void vertical_gradient(int x1, int y1, int x2, int y2, Fl_Color c1, Fl_Color c2) {
	int imax = y2 - y1;
	int d = imax ? imax : 1;
	if (Fl::draw_box_active()) {
		for (int i = 0; i <= imax; i++) {
			float w = 1.0f - (float)i / d;
			fl_color(fl_color_average(c1, c2, w));
			fl_xyline(x1, y1+i, x2);
		}
	}
	else {
		for (int i = 0; i <= imax; i++) {
			float w = 1.0f - (float)i / d;
			fl_color(fl_inactive(fl_color_average(c1, c2, w)));
			fl_xyline(x1, y1+i, x2);
		}
	}
}

static void horizontal_gradient(int x1, int y1, int x2, int y2, Fl_Color c1, Fl_Color c2) {
	int imax = x2 - x1;
	int d = imax ? imax : 1;
	if (Fl::draw_box_active()) {
		for (int i = 0; i <= imax; i++) {
			float w = 1.0f - (float)i / d;
			fl_color(fl_color_average(c1, c2, w));
			fl_yxline(x1+i, y1, y2);
		}
	}
	else {
		for (int i = 0; i <= imax; i++) {
			float w = 1.0f - (float)i / d;
			fl_color(fl_inactive(fl_color_average(c1, c2, w)));
			fl_yxline(x1+i, y1, y2);
		}
	}
}

/*************************** Classic (Windows 2000) ***************************/

static void classic_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	fl_frame2("AAWWMMTT", x, y, w, h);
}

static void classic_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+2, w-4, h-4);
	classic_button_up_frame(x, y, w, h, c);
}

static void classic_check_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_frame2("WWMMPPAA", x, y, w, h);
}

static void classic_check_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+2, w-4, h-4);
	classic_check_down_frame(x, y, w, h, c);
}

static void classic_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color) {
	fl_frame2("MMWW", x, y, w, h);
}

static void classic_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	classic_panel_thin_up_frame(x, y, w, h, c);
}

static void classic_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_frame2("WWMM", x, y, w, h);
}

static void classic_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	classic_spacer_thin_down_frame(x, y, w, h, c);
}

static void classic_radio_round_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left outer border
	fl_color(activated_color(fl_gray_ramp('M'-'A')));
	fl_arc(x, y, w, h, 45.0, 225.0);
	// bottom and right outer border
	fl_color(activated_color(fl_gray_ramp('W'-'A')));
	fl_arc(x, y, w, h, -135.0, 45.0);
	// top and left inner border
	fl_color(activated_color(fl_gray_ramp('A'-'A')));
	fl_arc(x+1, y+1, w-2, h-2, 45.0, 225.0);
	// bottom and right inner border
	fl_color(activated_color(fl_gray_ramp('T'-'A')));
	fl_arc(x+1, y+1, w-2, h-2, -135.0, 45.0);
}

static void classic_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_gray_ramp('W'-'A')));
	fl_pie(x+2, y+2, w-4, h-4, 0.0, 360.0);
	classic_radio_round_down_frame(x, y, w, h, c);
}

static void use_classic_scheme() {
	Fl::scheme("none");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, classic_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, classic_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, classic_button_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, classic_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, classic_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, classic_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, classic_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, classic_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, classic_radio_round_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, classic_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, classic_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, classic_button_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, classic_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, classic_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, classic_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, classic_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, FL_FLAT_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, classic_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, classic_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, classic_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, classic_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, classic_button_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, classic_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(FL_UP_BOX, classic_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(FL_DOWN_BOX, classic_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, classic_radio_round_down_box, 2, 2, 4, 4);
}

static void use_classic_colors() {
	Fl::background(0xD4, 0xD0, 0xC8);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x00, 0x00, 0x00);
	Fl::set_color(FL_SELECTION_COLOR, 0x0A, 0x24, 0x6A);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xE1));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_classic_theme() {
	use_classic_scheme();
	use_classic_colors();
	use_native_settings();
	_current_theme = CLASSIC;
}

/****************************** Aero (Windows 7) ******************************/

static void aero_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x70, 0x70, 0x70)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xFB, 0xFB, 0xFB)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xF2, 0xF2, 0xF2)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x90, 0x90, 0x90)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0x88, 0x88, 0x88)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void aero_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		// top gradient
		vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xF1, 0xF1, 0xF1)),
			activated_color(fl_rgb_color(0xEA, 0xEA, 0xEA)));
		// bottom gradient
		vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xDC, 0xDC, 0xDC)),
			activated_color(fl_rgb_color(0xCE, 0xCE, 0xCE)));
	}
	else {
		// left gradient
		horizontal_gradient(x+2, y+2, x+w/2-1, y+h-3, activated_color(fl_rgb_color(0xF1, 0xF1, 0xF1)),
			activated_color(fl_rgb_color(0xEA, 0xEA, 0xEA)));
		// right gradient
		horizontal_gradient(x+w/2, y+2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xDC, 0xDC, 0xDC)),
			activated_color(fl_rgb_color(0xCE, 0xCE, 0xCE)));
	}
	aero_button_up_frame(x, y, w, h, c);
}

static void aero_check_down_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x8F, 0x8F, 0x8F)));
	fl_rect(x, y, w, h);
	// middle border
	fl_color(activated_color(fl_rgb_color(0xF4, 0xF4, 0xF4)));
	fl_rect(x+1, y+1, w-2, h-2);
	// top and left inner borders
	fl_color(activated_color(fl_rgb_color(0xAE, 0xB3, 0xB9)));
	fl_yxline(x+2, y+h-3, y+2, x+w-3);
	// bottom and right inner borders
	fl_color(activated_color(fl_rgb_color(0xE9, 0xE9, 0xEA)));
	fl_xyline(x+3, y+h-3, x+w-3, y+3);
}

static void aero_check_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_rgb_color(0xF5, 0xF5, 0xF5)));
	fl_rectf(x+3, y+3, w-6, h-6);
	aero_check_down_frame(x, y, w, h, c);
}

static void aero_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xFA, 0xFA, 0xFA)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xC9, 0xC9, 0xC9)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void aero_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	aero_panel_thin_up_frame(x, y, w, h, c);
}

static void aero_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xC9, 0xC9, 0xC9)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xFA, 0xFA, 0xFA)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void aero_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	aero_spacer_thin_down_frame(x, y, w, h, c);
}

static void aero_radio_round_down_frame(int x, int y, int w, int h, Fl_Color) {
	// middle border
	fl_color(activated_color(fl_rgb_color(0xF4, 0xF4, 0xF4)));
	fl_arc(x+1, y+1, w-2, h-2, 0.0, 360.0);
	// outer border
	fl_color(activated_color(fl_rgb_color(0x8F, 0x8F, 0x8F)));
	fl_arc(x, y, w, h, 0.0, 360.0);
	// top and left inner border
	fl_color(activated_color(fl_rgb_color(0xAE, 0xAE, 0xAE)));
	fl_arc(x+2, y+2, w-4, h-4, 45.0, 225.0);
	// bottom and right inner border
	fl_color(activated_color(fl_rgb_color(0xE4, 0xE4, 0xE4)));
	fl_arc(x+2, y+2, w-4, h-4, -135.0, 45.0);
}

static void aero_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_rgb_color(0xF5, 0xF5, 0xF5)));
	fl_pie(x+2, y+2, w-4, h-4, 0.0, 360.0);
	aero_radio_round_down_frame(x, y, w, h, c);
}

static void aero_hovered_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x3C, 0x7F, 0xB0)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xF9, 0xFC, 0xFD)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xE7, 0xF4, 0xFB)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x69, 0x9C, 0xC2)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0x64, 0x9A, 0xC2)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void aero_hovered_up_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xE9, 0xF5, 0xFC)),
		activated_color(fl_rgb_color(0xD8, 0xEF, 0xFB)));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xBD, 0xE5, 0xFC)),
		activated_color(fl_rgb_color(0xA7, 0xD8, 0xF4)));
	aero_hovered_up_frame(x, y, w, h, c);
}

static void aero_depressed_down_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x2C, 0x62, 0x8B)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0x9D, 0xAF, 0xB9)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0x55, 0x92, 0xB5)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	// corners
	fl_color(activated_color(fl_rgb_color(0x33, 0x56, 0x71)));
	fl_xyline(x, y+1, x+1, y);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void aero_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xE4, 0xF3, 0xFB)),
		activated_color(fl_rgb_color(0xC4, 0xE5, 0xF6)));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-2, activated_color(fl_rgb_color(0x98, 0xD1, 0xEF)),
		activated_color(fl_rgb_color(0x68, 0xB3, 0xDB)));
	aero_depressed_down_frame(x, y, w, h, c);
}

static void aero_input_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top border
	fl_color(activated_color(fl_rgb_color(0xAA, 0xAC, 0xB2)));
	fl_xyline(x+1, y, x+w-2);
	// side borders
	fl_color(activated_color(fl_rgb_color(0xDA, 0xDE, 0xE5)));
	fl_yxline(x, y+1, y+h-2);
	fl_yxline(x+w-1, y+1, y+h-2);
	// bottom border
	fl_color(activated_color(fl_rgb_color(0xE2, 0xE8, 0xEE)));
	fl_xyline(x+1, y+h-1, x+w-2);
	// inner corners
	fl_color(activated_color(fl_rgb_color(0xE8, 0xEB, 0xEF)));
	fl_point(x+1, y+1);
	fl_point(x+w-2, y+1);
	fl_point(x+1, y+h-2);
	fl_point(x+w-2, y+h-2);
}

static void aero_input_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	aero_input_thin_down_frame(x, y, w, h, c);
}

static void aero_default_button_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x3C, 0x7F, 0xB1)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0x40, 0xD7, 0xFC)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0x35, 0xCE, 0xF4)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// top innermost borders
	fl_color(activated_color(fl_rgb_color(0xD2, 0xEE, 0xF6)));
	fl_xyline(x+3, y+2, x+w-4);
	fl_yxline(x+2, y+3, y+h/2-1);
	fl_yxline(x+w-3, y+3, y+h/2-1);
	// bottom innermost borders
	fl_color(activated_color(fl_rgb_color(0xB0, 0xD1, 0xDC)));
	fl_yxline(x+2, y+h/2, y+h-4);
	fl_yxline(x+w-3, y+h/2, y+h-4);
	fl_xyline(x+3, y+h-3, x+w-4);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x3A, 0x93, 0xC2)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0x38, 0x91, 0xC1)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void aero_default_button_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+3, y+3, x+w-4, y+h/2-1, activated_color(fl_rgb_color(0xF0, 0xF3, 0xF5)),
		activated_color(fl_rgb_color(0xE9, 0xEE, 0xF1)));
	// bottom gradient
	vertical_gradient(x+3, y+h/2, x+w-4, y+h-4, activated_color(fl_rgb_color(0xD7, 0xE1, 0xE7)),
		activated_color(fl_rgb_color(0xC8, 0xD5, 0xDD)));
	aero_default_button_frame(x, y, w, h, c);
}

static void aero_swatch_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0xA0, 0xA0, 0xA0)));
	fl_rect(x, y, w, h);
	// inner border
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_rect(x+1, y+1, w-2, h-2);
}

static void aero_swatch_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+2, w-4, h-4);
	aero_swatch_frame(x, y, w, h, c);
}

static void use_aero_scheme() {
	Fl::scheme("gtk+");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, aero_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, aero_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, aero_button_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, aero_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, aero_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, aero_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, aero_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, aero_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, aero_radio_round_down_box, 3, 3, 6, 6);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, aero_hovered_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, aero_depressed_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, aero_hovered_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, aero_depressed_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, aero_input_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, aero_input_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, aero_default_button_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, FL_FLAT_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, aero_swatch_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, aero_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_DEPRESSED_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_DEPRESSED_DOWN_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_aero_colors() {
	Fl::background(0xF0, 0xF0, 0xF0);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x00, 0x00, 0x00);
	Fl::set_color(FL_SELECTION_COLOR, 0x33, 0x99, 0xFF);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xF0));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_aero_theme() {
	use_aero_scheme();
	use_aero_colors();
	use_native_settings();
	_current_theme = AERO;
}

/****************************** Metro (Windows 8) *****************************/

static void metro_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0xAC, 0xAC, 0xAC)));
	fl_rect(x, y, w, h);
}

static void metro_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		vertical_gradient(x+1, y+1, x+w-2, y+h-2, activated_color(fl_rgb_color(0xF0, 0xF0, 0xF0)),
			activated_color(fl_rgb_color(0xE5, 0xE5, 0xE5)));
	}
	else {
		horizontal_gradient(x+1, y+1, x+w-2, y+h-2, activated_color(fl_rgb_color(0xF0, 0xF0, 0xF0)),
			activated_color(fl_rgb_color(0xE5, 0xE5, 0xE5)));
	}
	metro_button_up_frame(x, y, w, h, c);
}

static void metro_check_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x70, 0x70, 0x70)));
	fl_rect(x, y, w, h);
}

static void metro_check_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_rectf(x+1, y+1, w-2, h-2);
	metro_check_down_frame(x, y, w, h, c);
}

static void metro_radio_round_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x73, 0x73, 0x73)));
	fl_arc(x, y, w, h, 0.0, 360.0);
}

static void metro_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_pie(x, y, w, h, 0.0, 360.0);
	metro_radio_round_down_frame(x, y, w, h, c);
}

static void metro_hovered_up_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x7E, 0xB4, 0xEA)));
	fl_rect(x, y, w, h);
}

static void metro_hovered_up_box(int x, int y, int w, int h, Fl_Color c) {
	vertical_gradient(x+1, y+1, x+w-2, y+h-2, activated_color(fl_rgb_color(0xEC, 0xF4, 0xFC)),
		activated_color(fl_rgb_color(0xDC, 0xEC, 0xFC)));
	metro_hovered_up_frame(x, y, w, h, c);
}

static void metro_depressed_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x56, 0x9D, 0xE5)));
	fl_rect(x, y, w, h);
}

static void metro_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	vertical_gradient(x+1, y+1, x+w-2, y+h-2, activated_color(fl_rgb_color(0xDA, 0xEC, 0xFC)),
		activated_color(fl_rgb_color(0xC4, 0xE0, 0xFC)));
	metro_depressed_down_frame(x, y, w, h, c);
}

static void metro_input_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0xAB, 0xAD, 0xB3)));
	fl_rect(x, y, w, h);
}

static void metro_input_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_rectf(x+1, y+1, w-2, h-2);
	metro_input_thin_down_frame(x, y, w, h, c);
}

static void metro_default_button_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x33, 0x99, 0xFF)));
	fl_rect(x, y, w, h);
}

static void metro_default_button_box(int x, int y, int w, int h, Fl_Color c) {
	vertical_gradient(x+1, y+1, x+w-2, y+h-2, activated_color(fl_rgb_color(0xF0, 0xF0, 0xF0)),
		activated_color(fl_rgb_color(0xE5, 0xE5, 0xE5)));
	metro_default_button_frame(x, y, w, h, c);
}

static void use_metro_scheme() {
	Fl::scheme("gtk+");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, metro_button_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, metro_check_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, metro_button_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, metro_check_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, aero_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, aero_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, aero_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, aero_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, metro_radio_round_down_box, 3, 3, 6, 6);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, metro_hovered_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, metro_depressed_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, metro_hovered_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, metro_depressed_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, metro_input_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, metro_input_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, metro_default_button_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, FL_FLAT_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, aero_swatch_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, aero_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_DEPRESSED_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_DEPRESSED_DOWN_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_metro_colors() {
	Fl::reload_scheme();
	Fl::background(0xF0, 0xF0, 0xF0);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x00, 0x00, 0x00);
	Fl::set_color(FL_SELECTION_COLOR, 0x33, 0x99, 0xFF);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xFF));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_metro_theme() {
	use_metro_scheme();
	use_metro_colors();
	use_native_settings();
	_current_theme = METRO;
}

/**************************** Aqua (Mac OS X Lion) ****************************/

static void aqua_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x9A, 0x9A, 0x9A)));
	fl_xyline(x+3, y, x+w-4);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x91, 0x91, 0x91)));
	fl_yxline(x, y+3, y+h-4);
	fl_yxline(x+w-1, y+3, y+h-4);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x90, 0x90, 0x90)));
	fl_xyline(x+3, y+h-1, x+w-4);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_xyline(x+3, y+1, x+w-4);
	// side top inner borders
	fl_color(activated_color(fl_rgb_color(0xFC, 0xFC, 0xFC)));
	fl_yxline(x+1, y+3, y+h/2-1);
	fl_yxline(x+w-2, y+3, y+h/2-1);
	// side bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xF4, 0xF4, 0xF4)));
	fl_yxline(x+1, y+h/2-1, y+h-4);
	fl_yxline(x+w-2, y+3, y+h-4);
	// bottom inner border
	fl_color(activated_color(fl_rgb_color(0xF3, 0xF2, 0xF0)));
	fl_xyline(x+3, y+h-2, x+w-4);
	// corners
	fl_color(activated_color(fl_rgb_color(0xAF, 0xAF, 0xAF)));
	fl_arc(x, y, 8, 8, 90.0, 180.0);
	fl_arc(x, y+h-8, 8, 8, 180.0, 270.0);
	fl_arc(x+w-8, y+h-8, 8, 8, 270.0, 360.0);
	fl_arc(x+w-8, y, 8, 8, 0.0, 90.0);
}

static void aqua_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		// top gradient
		vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, fl_rgb_color(0xFF, 0xFF, 0xFF), fl_rgb_color(0xF6, 0xF5, 0xF4));
		// bottom fill
		fl_color(activated_color(fl_rgb_color(0xED, 0xEC, 0xEA)));
		fl_rectf(x+2, y+h/2, w-4, h-h/2-3);
		// bottom gradient
		fl_color(activated_color(fl_rgb_color(0xEF, 0xEE, 0xEC)));
		fl_xyline(x+2, y+h-3, x+w-3);
	}
	else {
		// left gradient
		horizontal_gradient(x+2, y+2, x+w/2-1, y+h-3, fl_rgb_color(0xFF, 0xFF, 0xFF), fl_rgb_color(0xF6, 0xF5, 0xF4));
		// right fill
		fl_color(activated_color(fl_rgb_color(0xED, 0xEC, 0xEA)));
		fl_rectf(x+w/2, y+2, w-w/2-3, h-4);
		// right gradient
		fl_color(activated_color(fl_rgb_color(0xEF, 0xEE, 0xEC)));
		fl_yxline(x+w-3, y+2, y+h-3);
	}
	aqua_button_up_frame(x, y, w, h, c);
}

static void aqua_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(devalued(c, 0.06751f)));
	fl_rect(x, y, w, h);
}

static void aqua_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	aqua_panel_thin_up_frame(x, y, w, h, c);
}

static void aqua_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xD6, 0xD6, 0xD6)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xF3, 0xF3, 0xF3)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void aqua_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	aqua_spacer_thin_down_frame(x, y, w, h, c);
}

static void aqua_radio_round_down_frame(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(devalued(c, 0.42194f)));
	fl_arc(x, y, w, h, 0.0, 360.0);
}

static void aqua_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top edges
	fl_color(activated_color(fl_rgb_color(0xF6, 0xF6, 0xF6)));
	fl_arc(x+1, y+1, w-2, h-2, 0.0, 180.0);
	// bottom edges
	fl_color(activated_color(fl_rgb_color(0xEB, 0xEB, 0xEB)));
	fl_arc(x+1, y+1, w-2, h-2, 180.0, 360.0);
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, fl_rgb_color(0xFF, 0xFF, 0xFF), fl_rgb_color(0xF6, 0xF5, 0xF4));
	// bottom fill
	fl_color(activated_color(fl_rgb_color(0xED, 0xEC, 0xEA)));
	fl_rectf(x+2, y+h/2, w-4, h-h/2-3);
	// bottom gradient
	fl_color(activated_color(fl_rgb_color(0xEF, 0xEE, 0xEC)));
	fl_xyline(x+2, y+h-3, x+w-3);
	aqua_radio_round_down_frame(x, y, w, h, c);
}

static void aqua_depressed_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x4C, 0x54, 0xAA)));
	fl_xyline(x+3, y, x+w-4);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x49, 0x4C, 0x8F)));
	fl_yxline(x, y+3, y+h-4);
	fl_yxline(x+w-1, y+3, y+h-4);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x43, 0x46, 0x72)));
	fl_xyline(x+3, y+h-1, x+w-4);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0xBC, 0xD6, 0xEF)));
	fl_xyline(x+3, y+1, x+w-4);
	// side top inner borders
	fl_color(activated_color(fl_rgb_color(0x7C, 0xAB, 0xE9)));
	fl_yxline(x+1, y+3, y+h/2-1);
	fl_yxline(x+w-2, y+3, y+h/2-1);
	// side bottom inner borders
	fl_color(activated_color(fl_rgb_color(0x5F, 0xA1, 0xEA)));
	fl_yxline(x+1, y+h/2, y+h-4);
	fl_yxline(x+w-2, y+h/2, y+h-4);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x79, 0x81, 0xBC)));
	fl_arc(x, y, 8, 8, 90.0, 180.0);
	fl_arc(x+w-8, y, 8, 8, 0.0, 90.0);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0x72, 0x79, 0x96)));
	fl_arc(x, y+h-8, 8, 8, 180.0, 270.0);
	fl_arc(x+w-8, y+h-8, 8, 8, 270.0, 360.0);
}

static void aqua_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, fl_rgb_color(0xA3, 0xC1, 0xEF), fl_rgb_color(0x67, 0xA1, 0xE9));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-2, fl_rgb_color(0x46, 0x93, 0xE9), fl_rgb_color(0xAA, 0xD4, 0xF0));
	aqua_depressed_down_frame(x, y, w, h, c);
}

static void aqua_input_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x9B, 0x9B, 0x9B)));
	fl_xyline(x, y, x+w-1);
	// side and bottom outer borders
	fl_color(activated_color(fl_rgb_color(0xBA, 0xBA, 0xBA)));
	fl_yxline(x, y+1, y+h-1, x+w-1, y+1);
	// top shadow
	fl_color(activated_color(fl_rgb_color(0xE3, 0xE3, 0xE3)));
	fl_xyline(x+1, y+1, x+w-2);
	// inner border
	fl_color(activated_color(fl_rgb_color(0xF5, 0xF5, 0xF5)));
	fl_yxline(x+1, y+h-2, y+2, x+w-2, y+h-2);
}

static void aqua_input_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_rectf(x+2, y+3, w-4, h-4);
	aqua_input_thin_down_frame(x, y, w, h, c);
}

static void aqua_default_button_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x4E, 0x59, 0xA6)));
	fl_xyline(x+3, y, x+w-4);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x4C, 0x52, 0x89)));
	fl_yxline(x, y+3, y+h-4);
	fl_yxline(x+w-1, y+3, y+h-4);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x48, 0x4F, 0x69)));
	fl_xyline(x+3, y+h-1, x+w-4);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0xD0, 0xEA, 0xF6)));
	fl_xyline(x+3, y+1, x+w-4);
	// side top inner borders
	fl_color(activated_color(fl_rgb_color(0x7A, 0xBF, 0xEF)));
	fl_yxline(x+1, y+3, y+h/2-1);
	fl_yxline(x+w-2, y+3, y+h/2-1);
	// side bottom inner borders
	fl_color(activated_color(fl_rgb_color(0x53, 0xAF, 0xEF)));
	fl_yxline(x+1, y+h/2, y+h-4);
	fl_yxline(x+w-2, y+h/2, y+h-4);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x76, 0x80, 0xB5)));
	fl_arc(x, y, 8, 8, 90.0, 180.0);
	fl_arc(x+w-8, y, 8, 8, 0.0, 90.0);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0x6F, 0x75, 0x89)));
	fl_arc(x, y+h-8, 8, 8, 180.0, 270.0);
	fl_arc(x+w-8, y+h-8, 8, 8, 270.0, 360.0);
}

static void aqua_default_button_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, fl_rgb_color(0xBF, 0xDC, 0xF7), fl_rgb_color(0x84, 0xC4, 0xF1));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-2, fl_rgb_color(0x59, 0xB5, 0xF1), fl_rgb_color(0xBA, 0xE9, 0xF7));
	aqua_default_button_frame(x, y, w, h, c);
}

static void aqua_swatch_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0xA3, 0xA3, 0xA3)));
	fl_rect(x, y, w, h);
	// inner border
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_rect(x+1, y+1, w-2, h-2);
}

static void aqua_swatch_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+2, w-4, h-4);
	aqua_swatch_frame(x, y, w, h, c);
}

static void use_aqua_scheme() {
	Fl::scheme("gtk+");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, aqua_button_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, aqua_button_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, aqua_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, aqua_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, aqua_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, aqua_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, aqua_radio_round_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, aqua_depressed_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, aqua_depressed_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, aqua_input_thin_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, aqua_input_thin_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, aqua_default_button_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, FL_FLAT_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, aqua_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, aqua_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_DEPRESSED_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_DEPRESSED_DOWN_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_aqua_colors() {
	Fl::background(0xED, 0xED, 0xED);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x00, 0x00, 0x00);
	Fl::set_color(FL_SELECTION_COLOR, 0x30, 0x60, 0xF6);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xC7));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_aqua_theme() {
	use_aqua_scheme();
	use_aqua_colors();
	use_native_settings();
	_current_theme = AQUA;
}

/**************************** Greybird (Linux GTK+) ***************************/

static void greybird_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0xA6, 0xA6, 0xA6)));
	fl_xyline(x+2, y, x+w-3);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x96, 0x96, 0x96)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x87, 0x87, 0x87)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0xEE, 0xEE, 0xEE)));
	fl_xyline(x+2, y+1, x+w-3);
	// side inner borders
	fl_color(activated_color(fl_rgb_color(0xE4, 0xE4, 0xE4)));
	fl_yxline(x+1, y+2, y+h-3);
	fl_yxline(x+w-2, y+2, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0xB8, 0xB8, 0xB8)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xA0, 0xA0, 0xA0)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void greybird_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		vertical_gradient(x+2, y+2, x+w-3, y+h-2, activated_color(fl_rgb_color(0xDB, 0xDB, 0xDB)),
			activated_color(fl_rgb_color(0xCC, 0xCC, 0xCC)));
	}
	else {
		horizontal_gradient(x+2, y+2, x+w-3, y+h-2, activated_color(fl_rgb_color(0xDB, 0xDB, 0xDB)),
			activated_color(fl_rgb_color(0xCC, 0xCC, 0xCC)));
	}
	greybird_button_up_frame(x, y, w, h, c);
}

static void greybird_check_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top border
	fl_color(activated_color(fl_rgb_color(0x80, 0x80, 0x80)));
	fl_xyline(x+2, y, x+w-3);
	// side borders
	fl_color(activated_color(fl_rgb_color(0x89, 0x89, 0x89)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom border
	fl_color(activated_color(fl_rgb_color(0x90, 0x90, 0x90)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0xA6, 0xA6, 0xA6)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xB0, 0xB0, 0xB0)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void greybird_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xDA, 0xDA, 0xDA)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xC1, 0xC1, 0xC1)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void greybird_check_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	greybird_check_down_frame(x, y, w, h, c);
}

static void greybird_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	greybird_panel_thin_up_frame(x, y, w, h, c);
}

static void greybird_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xBA, 0xBA, 0xBA)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xDA, 0xDA, 0xDA)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void greybird_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	greybird_spacer_thin_down_frame(x, y, w, h, c);
}

static void greybird_radio_round_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x80, 0x80, 0x80)));
	fl_arc(x, y, w, h, 0.0, 360.0);
}

static void greybird_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(c);
	fl_pie(x+1, y+1, w-2, h-2, 0.0, 360.0);
	greybird_radio_round_down_frame(x, y, w, h, c);
}

static void greybird_hovered_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0xAE, 0xAE, 0xAE)));
	fl_xyline(x+2, y, x+w-3);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x9E, 0x9E, 0x9E)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x8E, 0x8E, 0x8E)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0xF3, 0xF3, 0xF3)));
	fl_xyline(x+2, y+1, x+w-3);
	// side inner borders
	fl_color(activated_color(fl_rgb_color(0xED, 0xED, 0xED)));
	fl_yxline(x+1, y+2, y+h-3);
	fl_yxline(x+w-2, y+2, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0xC0, 0xC0, 0xC0)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xA7, 0xA7, 0xA7)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void greybird_hovered_up_box(int x, int y, int w, int h, Fl_Color c) {
	vertical_gradient(x+2, y+2, x+w-3, y+h-2, activated_color(fl_rgb_color(0xE6, 0xE6, 0xE6)),
		activated_color(fl_rgb_color(0xD6, 0xD6, 0xD6)));
	greybird_hovered_up_frame(x, y, w, h, c);
}

static void greybird_depressed_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x8A, 0x8A, 0x8A)));
	fl_xyline(x+2, y, x+w-3);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x7D, 0x7D, 0x7D)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x71, 0x71, 0x71)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x98, 0x98, 0x98)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0x88, 0x88, 0x88)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void greybird_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+1, y+1, x+w-2, y+4, activated_color(fl_rgb_color(0xAF, 0xAF, 0xAF)),
		activated_color(fl_rgb_color(0xB4, 0xB4, 0xB4)));
	vertical_gradient(x+1, y+5, x+w-2, y+h-1, activated_color(fl_rgb_color(0xB4, 0xB4, 0xB4)),
		activated_color(fl_rgb_color(0xAA, 0xAA, 0xAA)));
	greybird_depressed_down_frame(x, y, w, h, c);
}

static void greybird_input_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x84, 0x84, 0x84)));
	fl_xyline(x+2, y, x+w-3);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x97, 0x97, 0x97)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0xAA, 0xAA, 0xAA)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// inner border
	fl_color(activated_color(fl_rgb_color(0xEC, 0xEC, 0xEC)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h-3);
	fl_yxline(x+w-2, y+2, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0xA4, 0xA4, 0xA4)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xBE, 0xBE, 0xBE)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void greybird_input_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+2, w-4, h-3);
	greybird_input_thin_down_frame(x, y, w, h, c);
}

static void greybird_default_button_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x69, 0x82, 0x9D)));
	fl_xyline(x+2, y, x+w-3);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x61, 0x77, 0x8E)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x59, 0x6B, 0x7D)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0x88, 0xB7, 0xE9)));
	fl_xyline(x+2, y+1, x+w-3);
	// side inner borders
	fl_color(activated_color(fl_rgb_color(0x79, 0xAC, 0xE1)));
	fl_yxline(x+1, y+2, y+h-3);
	fl_yxline(x+w-2, y+2, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x76, 0x99, 0xBE)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0x5D, 0x81, 0xA6)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void greybird_default_button_box(int x, int y, int w, int h, Fl_Color c) {
	vertical_gradient(x+2, y+2, x+w-3, y+h-2, activated_color(fl_rgb_color(0x72, 0xA7, 0xDF)),
		activated_color(fl_rgb_color(0x63, 0x9C, 0xD7)));
	greybird_default_button_frame(x, y, w, h, c);
}

static void use_greybird_scheme() {
	Fl::scheme("gtk+");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, greybird_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, greybird_check_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, greybird_button_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, greybird_check_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, greybird_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, greybird_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, greybird_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, greybird_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, greybird_radio_round_down_box, 3, 3, 6, 6);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, greybird_hovered_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, greybird_depressed_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, greybird_hovered_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, greybird_depressed_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, greybird_input_thin_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, greybird_input_thin_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, greybird_default_button_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, OS_SPACER_THIN_DOWN_FRAME);
	Fl::set_boxtype(OS_SWATCH_BOX, OS_SPACER_THIN_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_DEPRESSED_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_DEPRESSED_DOWN_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_greybird_colors() {
	Fl::background(0xCE, 0xCE, 0xCE);
	Fl::background2(0xFC, 0xFC, 0xFC);
	Fl::foreground(0x3C, 0x3C, 0x3C);
	Fl::set_color(FL_SELECTION_COLOR, 0x50, 0xA0, 0xF4);
	Fl_Tooltip::color(fl_rgb_color(0x0A, 0x0A, 0x0A));
	Fl_Tooltip::textcolor(fl_rgb_color(0xFF, 0xFF, 0xFF));
}

void OS::use_greybird_theme() {
	use_greybird_scheme();
	use_greybird_colors();
	use_native_settings();
	_current_theme = GREYBIRD;
}

/***************************** Metal (Java Swing) *****************************/

static void metal_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x7A, 0x8A, 0x99)));
	fl_rect(x, y, w, h);
}

static void metal_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		int m = h * 3 / 10;
		// top gradient
		vertical_gradient(x+1, y+1, x+w-2, y+m, activated_color(fl_rgb_color(0xDF, 0xE9, 0xF3)),
			activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
		// bottom gradient
		vertical_gradient(x+1, y+m, x+w-2, y+h-2, activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)),
			activated_color(fl_rgb_color(0xBB, 0xD1, 0xE6)));
	}
	else {
		int m = w * 3 / 10;
		// left gradient
		horizontal_gradient(x+1, y+1, x+m, y+h-2, activated_color(fl_rgb_color(0xDF, 0xE9, 0xF3)),
			activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
		// right gradient
		horizontal_gradient(x+m, y+1, x+w-2, y+h-2, activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)),
			activated_color(fl_rgb_color(0xBB, 0xD1, 0xE6)));
	}
	metal_button_up_frame(x, y, w, h, c);
}

static void metal_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0x63, 0x82, 0xBF)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void metal_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	metal_panel_thin_up_frame(x, y, w, h, c);
}

static void metal_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0x63, 0x82, 0xBF)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void metal_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	metal_spacer_thin_down_frame(x, y, w, h, c);
}

static void metal_radio_round_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x7A, 0x8A, 0x99)));
	fl_arc(x, y, w, h, 0.0, 360.0);
}

static void metal_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	int m = h * 3 / 10;
	// top gradient
	vertical_gradient(x+1, y+1, x+w-2, y+m, activated_color(fl_rgb_color(0xDF, 0xE9, 0xF3)),
		activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	// bottom gradient
	vertical_gradient(x+1, y+m, x+w-2, y+h-2, activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)),
		activated_color(fl_rgb_color(0xBB, 0xD1, 0xE6)));
	metal_radio_round_down_frame(x, y, w, h, c);
}

static void metal_input_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	metal_button_up_frame(x, y, w, h, c);
}

static void metal_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(fl_rgb_color(0xBB, 0xCF, 0xE3)));
	fl_rectf(x+1, y+1, w-2, h-2);
	metal_button_up_frame(x, y, w, h, c);
}

static void use_metal_scheme() {
	Fl::scheme("none");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, metal_button_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, metal_depressed_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, metal_button_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, metal_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, metal_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, metal_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, metal_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, metal_radio_round_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, metal_input_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, FL_FLAT_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_SWATCH_BOX, OS_INPUT_THIN_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_metal_colors() {
	Fl::background(0xEE, 0xEE, 0xEE);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x33, 0x33, 0x33);
	Fl::set_color(FL_SELECTION_COLOR, 0xA3, 0xB8, 0xCC);
	Fl_Tooltip::color(fl_rgb_color(0xB8, 0xCF, 0xE5));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_metal_theme() {
	use_metal_scheme();
	use_metal_colors();
	use_native_settings();
	_current_theme = METAL;
}

/************************** Blue (Windows Calculator) *************************/

static void blue_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x87, 0x97, 0xAA)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xF6, 0xFA, 0xFE)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xFE, 0xFF, 0xFF)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x9B, 0xAA, 0xBB)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xA1, 0xAE, 0xBD)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void blue_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		// top gradient
		vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xF0, 0xF6, 0xFB)),
			activated_color(fl_rgb_color(0xE2, 0xEA, 0xF3)));
		// bottom gradient
		vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xD5, 0xE0, 0xED)),
			activated_color(fl_rgb_color(0xD7, 0xE2, 0xEF)));
	}
	else {
		// left gradient
		horizontal_gradient(x+2, y+2, x+w/2-1, y+h-3, activated_color(fl_rgb_color(0xF0, 0xF6, 0xFB)),
			activated_color(fl_rgb_color(0xE2, 0xEA, 0xF3)));
		// right gradient
		horizontal_gradient(x+w/2, y+2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xD5, 0xE0, 0xED)),
			activated_color(fl_rgb_color(0xD7, 0xE2, 0xEF)));
	}
	blue_button_up_frame(x, y, w, h, c);
}

static void blue_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xF0, 0xF6, 0xFB)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xBF, 0xCB, 0xDA)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void blue_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	blue_panel_thin_up_frame(x, y, w, h, c);
}

static void blue_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xBF, 0xCB, 0xDA)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xF0, 0xF6, 0xFB)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void blue_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	blue_spacer_thin_down_frame(x, y, w, h, c);
}

static void blue_hovered_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0xFF, 0xDB, 0x00)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFC, 0xF8)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFA, 0xE2)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// corners
	fl_color(activated_color(fl_rgb_color(0xF7, 0xD7, 0x3F)));
	fl_xyline(x, y+1, x+1, y);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void blue_hovered_up_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xFF, 0xF0, 0xDF)),
		activated_color(fl_rgb_color(0xFF, 0xE2, 0xC2)));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xFF, 0xCF, 0x6A)),
		activated_color(fl_rgb_color(0xFF, 0xE9, 0x83)));
	blue_hovered_up_frame(x, y, w, h, c);
}

static void blue_depressed_down_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0xC2, 0x9B, 0x29)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0xE3, 0xC1, 0x85)));
	fl_xyline(x+2, y+1, x+w-3);
	// corners
	fl_color(activated_color(fl_rgb_color(0xCB, 0xAB, 0x53)));
	fl_xyline(x, y+1, x+1, y);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void blue_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+1, y+2, x+w-2, y+h/2-1, activated_color(fl_rgb_color(0xEE, 0xCB, 0x8E)),
		activated_color(fl_rgb_color(0xF5, 0xC7, 0x79)));
	// bottom gradient
	vertical_gradient(x+1, y+h/2, x+w-2, y+h-1, activated_color(fl_rgb_color(0xF5, 0xBB, 0x57)),
		activated_color(fl_rgb_color(0xF3, 0xE1, 0x77)));
	blue_depressed_down_frame(x, y, w, h, c);
}

static void blue_default_button_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x87, 0x97, 0xAA)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xF7, 0xFB, 0xFF)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xFB, 0xFE, 0xFF)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0x9B, 0xAA, 0xBB)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xA1, 0xAE, 0xBD)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void blue_default_button_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xF7, 0xFB, 0xFF)),
		activated_color(fl_rgb_color(0xED, 0xF3, 0xF8)));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xE7, 0xED, 0xF5)),
		activated_color(fl_rgb_color(0xEA, 0xF1, 0xF8)));
	blue_default_button_frame(x, y, w, h, c);
}

static void use_blue_scheme() {
	Fl::scheme("gtk+");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, blue_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, aero_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, blue_button_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, aero_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, blue_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, blue_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, blue_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, blue_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, aero_radio_round_down_box, 3, 3, 6, 6);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, blue_hovered_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, blue_depressed_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, blue_hovered_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, blue_depressed_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, aero_input_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, aero_input_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, blue_default_button_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, FL_FLAT_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, aero_swatch_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, aero_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_DEPRESSED_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_DEPRESSED_DOWN_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_blue_colors() {
	Fl::background(0xD9, 0xE4, 0xF1);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x1E, 0x39, 0x5B);
	Fl::set_color(FL_SELECTION_COLOR, 0x33, 0x33, 0x33);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xFF));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_blue_theme() {
	use_blue_scheme();
	use_blue_colors();
	use_native_settings();
	_current_theme = BLUE;
}

/***************************** Olive (Windows XP) *****************************/

static void olive_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	if (w >= h) {
		// top inner borders
		fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xF6)));
		fl_xyline(x+3, y+1, x+w-4);
		fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xF5)));
		fl_xyline(x+2, y+2, x+w-3);
		// bottom inner borders
		fl_color(activated_color(fl_rgb_color(0xEC, 0xE1, 0xC9)));
		fl_xyline(x+2, y+h-3, x+w-3);
		fl_color(activated_color(fl_rgb_color(0xE3, 0xD1, 0xB8)));
		fl_xyline(x+3, y+h-2, x+w-4);
	}
	else {
		// left inner borders
		fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xF6)));
		fl_yxline(x+1, y+3, y+h-4);
		fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xF5)));
		fl_yxline(x+2, y+2, y+h-3);
		// right inner borders
		fl_color(activated_color(fl_rgb_color(0xEC, 0xE1, 0xC9)));
		fl_yxline(x+w-3, y+2, y+h-3);
		fl_color(activated_color(fl_rgb_color(0xE3, 0xD1, 0xB8)));
		fl_yxline(x+w-2, y+3, y+h-4);
	}
	// outer corners
	fl_color(activated_color(fl_rgb_color(0x6D, 0x8A, 0x4D)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
	if (w >= h) {
		// top inner corners
		fl_color(activated_color(fl_rgb_color(0xE4, 0xDD, 0xB4)));
		fl_point(x+2, y+1);
		fl_point(x+w-3, y+1);
		fl_point(x+1, y+2);
		fl_point(x+w-2, y+2);
		// bottom inner corners
		fl_color(activated_color(fl_rgb_color(0xC9, 0xB1, 0xA4)));
		fl_point(x+1, y+h-3);
		fl_point(x+w-2, y+h-3);
		fl_point(x+2, y+h-2);
		fl_point(x+w-3, y+h-2);
	}
	else {
		// left inner corners
		fl_color(activated_color(fl_rgb_color(0xE4, 0xDD, 0xB4)));
		fl_point(x+2, y+1);
		fl_point(x+1, y+h-3);
		fl_point(x+1, y+2);
		fl_point(x+2, y+h-2);
		// right inner corners
		fl_color(activated_color(fl_rgb_color(0xC9, 0xB1, 0xA4)));
		fl_point(x+w-3, y+1);
		fl_point(x+w-2, y+h-3);
		fl_point(x+w-2, y+2);
		fl_point(x+w-3, y+h-2);
	}

}

static void olive_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		// top gradient
		vertical_gradient(x+1, y+3, x+w-2, y+h/3, activated_color(fl_rgb_color(0xFE, 0xFF, 0xF2)),
			activated_color(fl_rgb_color(0xFA, 0xF9, 0xE9)));
		// bottom gradient
		vertical_gradient(x+1, y+h/3, x+w-2, y+h-4, activated_color(fl_rgb_color(0xFB, 0xFA, 0xE8)),
			activated_color(fl_rgb_color(0xF3, 0xEE, 0xDB)));
	}
	else {
		// left gradient
		horizontal_gradient(x+3, y+1, x+w/3, y+h-2, activated_color(fl_rgb_color(0xFE, 0xFF, 0xF2)),
			activated_color(fl_rgb_color(0xFA, 0xF9, 0xE9)));
		// right gradient
		horizontal_gradient(x+w/3, y+1, x+w-4, y+h-2, activated_color(fl_rgb_color(0xFB, 0xFA, 0xE8)),
			activated_color(fl_rgb_color(0xF3, 0xEE, 0xDB)));
	}
	olive_button_up_frame(x, y, w, h, c);
}

static void olive_button_down_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xDB, 0xD1, 0xAD)));
	fl_xyline(x+3, y+1, x+w-4);
	fl_color(activated_color(fl_rgb_color(0xDD, 0xDA, 0xBB)));
	fl_xyline(x+2, y+2, x+w-3);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xE8, 0xE7, 0xDB)));
	fl_xyline(x+2, y+h-3, x+w-3);
	fl_color(activated_color(fl_rgb_color(0xF3, 0xF2, 0xE6)));
	fl_xyline(x+3, y+h-2, x+w-4);
	// outer corners
	fl_color(activated_color(fl_rgb_color(0x6D, 0x8A, 0x4D)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
	// top inner corners
	fl_color(activated_color(fl_rgb_color(0xC9, 0xB1, 0xA4)));
	fl_point(x+2, y+1);
	fl_point(x+w-3, y+1);
	fl_point(x+1, y+2);
	fl_point(x+w-2, y+2);
	// bottom inner corners
	fl_color(activated_color(fl_rgb_color(0xE4, 0xDD, 0xB4)));
	fl_point(x+1, y+h-3);
	fl_point(x+w-2, y+h-3);
	fl_point(x+2, y+h-2);
	fl_point(x+w-3, y+h-2);

}

static void olive_button_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+1, y+3, x+w-2, y+h/3, activated_color(fl_rgb_color(0xDC, 0xE1, 0xD0)),
		activated_color(fl_rgb_color(0xDF, 0xE1, 0xC8)));
	// bottom gradient
	vertical_gradient(x+1, y+h/3, x+w-2, y+h-4, activated_color(fl_rgb_color(0xDF, 0xE1, 0xC8)),
		activated_color(fl_rgb_color(0xD8, 0xDF, 0xC8)));
	olive_button_down_frame(x, y, w, h, c);
}

static void olive_hovered_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xF9, 0xEF, 0xCB)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_color(activated_color(fl_rgb_color(0xFA, 0xCF, 0x70)));
	fl_xyline(x+1, y+2, x+w-2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xF5, 0xA5, 0x10)));
	fl_xyline(x+1, y+h-3, x+w-2);
	fl_color(activated_color(fl_rgb_color(0xE4, 0x88, 0x01)));
	fl_xyline(x+2, y+h-2, x+w-3);
	// outer corners
	fl_color(activated_color(fl_rgb_color(0x6D, 0x8A, 0x4D)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void olive_hovered_up_box(int x, int y, int w, int h, Fl_Color c) {
	// edges' top gradient
	vertical_gradient(x+1, y+3, x+w-2, y+h/2-1, activated_color(fl_rgb_color(0xF7, 0xC7, 0x61)),
		activated_color(fl_rgb_color(0xF7, 0xC3, 0x59)));
	// edges' bottom gradient
	vertical_gradient(x+1, y+h/2, x+w-2, y+h-4, activated_color(fl_rgb_color(0xF7, 0xBF, 0x52)),
		activated_color(fl_rgb_color(0xF2, 0xA8, 0x13)));
	fl_push_clip(x+3, y+3, w-6, h-6);
	// top gradient
	vertical_gradient(x+1, y+3, x+w-2, y+h/3, activated_color(fl_rgb_color(0xFE, 0xFF, 0xF2)),
		activated_color(fl_rgb_color(0xFA, 0xF9, 0xE9)));
	// bottom gradient
	vertical_gradient(x+1, y+h/3, x+w-2, y+h-4, activated_color(fl_rgb_color(0xFB, 0xFA, 0xE8)),
		activated_color(fl_rgb_color(0xF3, 0xEE, 0xDB)));
	fl_pop_clip();
	olive_hovered_up_frame(x, y, w, h, c);
}

static void olive_radio_round_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_arc(x, y, w, h, 0.0, 360.0);
}

static void olive_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	// bottom fill
	fl_color(activated_color(fl_rgb_color(0xF7, 0xF7, 0xF7)));
	fl_pie(x, y, w, h, 0.0, 360.0);
	// top edges
	fl_color(activated_color(fl_rgb_color(0xDC, 0xDA, 0xDC)));
	fl_arc(x+1, y+1, w-2, h-2, 0.0, 180.0);
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, fl_rgb_color(0xDE, 0xE2, 0xE1), fl_rgb_color(0xF4, 0xF7, 0xF6));
	olive_radio_round_down_frame(x, y, w, h, c);
}

static void olive_input_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0xA4, 0xB9, 0x7F)));
	fl_rect(x, y, w, h);
}

static void olive_input_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	olive_input_thin_down_frame(x, y, w, h, c);
}

static void olive_default_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xC2, 0xD1, 0x8F)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_color(activated_color(fl_rgb_color(0xB1, 0xCB, 0x80)));
	fl_xyline(x+1, y+2, x+w-2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0x90, 0xC1, 0x54)));
	fl_xyline(x+1, y+h-3, x+w-2);
	fl_color(activated_color(fl_rgb_color(0xA8, 0xA7, 0x66)));
	fl_xyline(x+2, y+h-2, x+w-3);
	// outer corners
	fl_color(activated_color(fl_rgb_color(0x6D, 0x8A, 0x4D)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);

}

static void olive_default_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	// edges' top gradient
	vertical_gradient(x+1, y+3, x+w-2, y+h/2-1, activated_color(fl_rgb_color(0xB0, 0xCC, 0x7D)),
		activated_color(fl_rgb_color(0xA0, 0xC7, 0x67)));
	// edges' bottom gradient
	vertical_gradient(x+1, y+h/2, x+w-2, y+h-4, activated_color(fl_rgb_color(0xA0, 0xC7, 0x67)),
		activated_color(fl_rgb_color(0x90, 0xC1, 0x54)));
	fl_push_clip(x+3, y+3, w-6, h-6);
	// top gradient
	vertical_gradient(x+1, y+3, x+w-2, y+h/3, activated_color(fl_rgb_color(0xFE, 0xFF, 0xF2)),
		activated_color(fl_rgb_color(0xFA, 0xF9, 0xE9)));
	// bottom gradient
	vertical_gradient(x+1, y+h/3, x+w-2, y+h-4, activated_color(fl_rgb_color(0xFB, 0xFA, 0xE8)),
		activated_color(fl_rgb_color(0xF3, 0xEE, 0xDB)));
	fl_pop_clip();
	olive_default_button_up_frame(x, y, w, h, c);
}

static void olive_toolbar_button_hover_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner border
	fl_color(activated_color(FL_LIGHT3));
	fl_xyline(x+2, y+1, x+w-3);
	// bottom inner border
	fl_color(activated_color(FL_DARK3));
	fl_xyline(x+2, y+h-2, x+w-3);
	// outer corners
	fl_color(activated_color(fl_rgb_color(0x6D, 0x8A, 0x4D)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void olive_toolbar_button_hover_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+1, y+2, x+w-2, y+h/3, activated_color(FL_LIGHT2), activated_color(FL_BACKGROUND_COLOR));
	// bottom gradient
	vertical_gradient(x+1, y+h/3, x+w-2, y+h-3, activated_color(FL_BACKGROUND_COLOR), activated_color(FL_DARK2));
	olive_toolbar_button_hover_frame(x, y, w, h, c);
}

static void olive_check_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_rect(x, y, w, h);
}

static void olive_check_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top edge
	fl_color(activated_color(fl_rgb_color(0xDC, 0xDA, 0xDC)));
	fl_xyline(x+1, y+1, x+w-2);
	fl_arc(x+1, y+1, w-2, h-2, 0.0, 180.0);
	// top gradient
	vertical_gradient(x+1, y+2, x+w-2, y+h/2-1, fl_rgb_color(0xDE, 0xE2, 0xE1), fl_rgb_color(0xF4, 0xF7, 0xF6));
	// bottom fill
	fl_color(activated_color(fl_rgb_color(0xF7, 0xF7, 0xF7)));
	fl_rectf(x+1, y+h/2, w-2, h-h/2-1);
	olive_check_down_frame(x, y, w, h, c);
}

static void olive_swatch_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0xA4, 0xB9, 0x7F)));
	fl_rect(x, y, w, h);
	// inner border
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_rect(x+1, y+1, w-2, h-2);
}

static void olive_swatch_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+2, w-4, h-4);
	olive_swatch_frame(x, y, w, h, c);
}

static void olive_mini_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x37, 0x62, 0x06)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xC2, 0xD1, 0x8F)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_color(activated_color(fl_rgb_color(0xB1, 0xCB, 0x80)));
	fl_xyline(x+1, y+2, x+w-2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0x90, 0xC1, 0x54)));
	fl_xyline(x+1, y+h-3, x+w-2);
	fl_color(activated_color(fl_rgb_color(0xA8, 0xA7, 0x66)));
	fl_xyline(x+2, y+h-2, x+w-3);
	// outer corners
	fl_color(activated_color(fl_rgb_color(0x6D, 0x8A, 0x4D)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);

}

static void olive_mini_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		// top gradient
		vertical_gradient(x+1, y+3, x+w-2, y+h/2-1, activated_color(fl_rgb_color(0xB0, 0xCC, 0x7D)),
			activated_color(fl_rgb_color(0xA0, 0xC7, 0x67)));
		// bottom gradient
		vertical_gradient(x+1, y+h/2, x+w-2, y+h-4, activated_color(fl_rgb_color(0xA0, 0xC7, 0x67)),
			activated_color(fl_rgb_color(0x90, 0xC1, 0x54)));
	}
	else {
		// left gradient
		horizontal_gradient(x+1, y+3, x+w/2-1, y+h-2, activated_color(fl_rgb_color(0xB0, 0xCC, 0x7D)),
			activated_color(fl_rgb_color(0xA0, 0xC7, 0x67)));
		// right gradient
		horizontal_gradient(x+w/2, y+3, x+w-2, y+h-4, activated_color(fl_rgb_color(0xA0, 0xC7, 0x67)),
			activated_color(fl_rgb_color(0x90, 0xC1, 0x54)));
	}
	olive_mini_button_up_frame(x, y, w, h, c);
}

static void use_olive_scheme() {
	Fl::scheme("none");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, olive_button_up_box, 1, 3, 2, 6);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, olive_button_down_box, 1, 3, 2, 6);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, olive_button_up_frame, 1, 3, 2, 6);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, olive_button_down_frame, 1, 3, 2, 6);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, classic_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, classic_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, classic_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, classic_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, olive_radio_round_down_box, 3, 3, 6, 6);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, olive_hovered_up_box, 1, 3, 2, 6);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, olive_hovered_up_frame, 1, 3, 2, 6);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, OS_CHECK_DOWN_FRAME);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, olive_input_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, olive_input_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, olive_default_button_up_box, 1, 3, 2, 6);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, olive_toolbar_button_hover_box, 1, 2, 2, 4);
	Fl::set_boxtype(OS_SWATCH_FRAME, olive_swatch_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, olive_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, olive_mini_button_up_box, 1, 3, 2, 6);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_MINI_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, olive_mini_button_up_frame, 1, 3, 2, 6);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_MINI_BUTTON_UP_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, olive_check_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_olive_colors() {
	Fl::background(0xD9, 0xE2, 0xB4);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x41, 0x40, 0x0A);
	Fl::set_color(FL_SELECTION_COLOR, 0xCC, 0x45, 0x18);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xE1));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_olive_theme() {
	use_olive_scheme();
	use_olive_colors();
	use_native_settings();
	_current_theme = OLIVE;
}

/********************************** Rose Gold *********************************/

static void rose_gold_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0xB0, 0x8C, 0x83)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xF7, 0xF5)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFE)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0xC1, 0x9F, 0x97)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xC3, 0xA6, 0x9E)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void rose_gold_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		// top gradient
		vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xFD, 0xF0, 0xEF)),
			activated_color(fl_rgb_color(0xF6, 0xE4, 0xE0)));
		// bottom gradient
		vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xF1, 0xD9, 0xD2)),
			activated_color(fl_rgb_color(0xF3, 0xDB, 0xD4)));
	}
	else {
		// left gradient
		horizontal_gradient(x+2, y+2, x+w/2-1, y+h-3, activated_color(fl_rgb_color(0xFD, 0xF0, 0xEF)),
			activated_color(fl_rgb_color(0xF6, 0xE4, 0xE0)));
		// right gradient
		horizontal_gradient(x+w/2, y+2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xF1, 0xD9, 0xD2)),
			activated_color(fl_rgb_color(0xF3, 0xDB, 0xD4)));
	}
	rose_gold_button_up_frame(x, y, w, h, c);
}

static void rose_gold_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xFD, 0xF0, 0xEF)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xDF, 0xC4, 0xBC)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void rose_gold_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	rose_gold_panel_thin_up_frame(x, y, w, h, c);
}

static void rose_gold_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0xDF, 0xC4, 0xBC)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0xFD, 0xF0, 0xEF)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void rose_gold_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	rose_gold_spacer_thin_down_frame(x, y, w, h, c);
}

static void rose_gold_hovered_up_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x6C, 0x80, 0x96)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xEF, 0xF1, 0xF3)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// corners
	fl_color(activated_color(fl_rgb_color(0x8F, 0x9F, 0xAD)));
	fl_xyline(x, y+1, x+1, y);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void rose_gold_hovered_up_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xF0, 0xF4, 0xF5)),
		activated_color(fl_rgb_color(0xE1, 0xE9, 0xEB)));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xC0, 0xD0, 0xD8)),
		activated_color(fl_rgb_color(0xBA, 0xC4, 0xCE)));
	rose_gold_hovered_up_frame(x, y, w, h, c);
}

static void rose_gold_depressed_down_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x75, 0x84, 0x8F)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0xAC, 0xB9, 0xC1)));
	fl_xyline(x+2, y+1, x+w-3);
	// corners
	fl_color(activated_color(fl_rgb_color(0x90, 0x9C, 0xA4)));
	fl_xyline(x, y+1, x+1, y);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void rose_gold_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+1, y+2, x+w-2, y+h/2-1, activated_color(fl_rgb_color(0xCA, 0xD8, 0xDF)),
		activated_color(fl_rgb_color(0xD1, 0xDC, 0xE1)));
	// bottom gradient
	vertical_gradient(x+1, y+h/2, x+w-2, y+h-1, activated_color(fl_rgb_color(0xB7, 0xC9, 0xD0)),
		activated_color(fl_rgb_color(0xAF, 0xBA, 0xC3)));
	rose_gold_depressed_down_frame(x, y, w, h, c);
}

static void rose_gold_default_button_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0xB0, 0x8C, 0x83)));
	fl_xyline(x+2, y, x+w-3);
	fl_xyline(x+2, y+h-1, x+w-3);
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// top inner borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xF7, 0xF6)));
	fl_xyline(x+2, y+1, x+w-3);
	fl_yxline(x+1, y+2, y+h/2-1);
	fl_yxline(x+w-2, y+2, y+h/2-1);
	fl_point(x+2, y+2);
	fl_point(x+w-3, y+2);
	// bottom inner borders
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFA, 0xFB)));
	fl_yxline(x+1, y+h/2, y+h-3);
	fl_yxline(x+w-2, y+h/2, y+h-3);
	fl_xyline(x+2, y+h-2, x+w-3);
	fl_point(x+2, y+h-3);
	fl_point(x+w-3, y+h-3);
	// top corners
	fl_color(activated_color(fl_rgb_color(0xC1, 0x9F, 0x97)));
	fl_xyline(x, y+1, x+1, y);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	// bottom corners
	fl_color(activated_color(fl_rgb_color(0xC3, 0xA6, 0x9E)));
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void rose_gold_default_button_box(int x, int y, int w, int h, Fl_Color c) {
	// top gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h/2-1, activated_color(fl_rgb_color(0xFF, 0xF7, 0xF6)),
		activated_color(fl_rgb_color(0xFA, 0xED, 0xEC)));
	// bottom gradient
	vertical_gradient(x+2, y+h/2, x+w-3, y+h-3, activated_color(fl_rgb_color(0xF7, 0xE9, 0xE5)),
		activated_color(fl_rgb_color(0xFB, 0xEC, 0xE9)));
	rose_gold_default_button_frame(x, y, w, h, c);
}

static void use_rose_gold_scheme() {
	Fl::scheme("gtk+");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, rose_gold_button_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, aero_check_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, rose_gold_button_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, aero_check_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, rose_gold_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, rose_gold_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, rose_gold_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, rose_gold_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, aero_radio_round_down_box, 3, 3, 6, 6);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, rose_gold_hovered_up_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, rose_gold_depressed_down_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, rose_gold_hovered_up_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, rose_gold_depressed_down_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, aero_input_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, aero_input_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, rose_gold_default_button_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, aero_swatch_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, aero_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_DEPRESSED_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_DEPRESSED_DOWN_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_CHECK_DOWN_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_rose_gold_colors() {
	Fl::background(0xF5, 0xDD, 0xD6);
	Fl::background2(0xFF, 0xFF, 0xFF);
	Fl::foreground(0x4C, 0x1E, 0x12);
	Fl::set_color(FL_SELECTION_COLOR, 0x15, 0x81, 0xFA);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xFF));
	Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

void OS::use_rose_gold_theme() {
	use_rose_gold_scheme();
	use_rose_gold_colors();
	use_native_settings();
	_current_theme = ROSE_GOLD;
}

/************************* Dark (Adobe Photoshop CS6) *************************/

static void dark_button_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x30, 0x30, 0x30)));
	fl_xyline(x+2, y, x+w-3);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x2C, 0x2C, 0x2C)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x27, 0x27, 0x27)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0x91, 0x91, 0x91)));
	fl_xyline(x+2, y+1, x+w-3);
	// corners
	fl_color(activated_color(fl_rgb_color(0x47, 0x47, 0x47)));
	fl_xyline(x, y+1, x+1, y);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
}

static void dark_button_up_box(int x, int y, int w, int h, Fl_Color c) {
	if (w >= h) {
		vertical_gradient(x+1, y+2, x+w-2, y+h-1, activated_color(fl_rgb_color(0x75, 0x75, 0x75)),
			activated_color(fl_rgb_color(0x62, 0x62, 0x62)));
	}
	else {
		horizontal_gradient(x+1, y+2, x+w-2, y+h-1, activated_color(fl_rgb_color(0x75, 0x75, 0x75)),
			activated_color(fl_rgb_color(0x62, 0x62, 0x62)));
	}
	dark_button_up_frame(x, y, w, h, c);
}

static void dark_panel_thin_up_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0x6A, 0x6A, 0x6A)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0x28, 0x28, 0x28)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void dark_panel_thin_up_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	dark_panel_thin_up_frame(x, y, w, h, c);
}

static void dark_spacer_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and left borders
	fl_color(activated_color(fl_rgb_color(0x38, 0x38, 0x38)));
	fl_yxline(x, y+h-2, y, x+w-2);
	// bottom and right borders
	fl_color(activated_color(fl_rgb_color(0x74, 0x74, 0x74)));
	fl_xyline(x, y+h-1, x+w-1, y);
}

static void dark_spacer_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+1, y+1, w-2, h-2);
	dark_spacer_thin_down_frame(x, y, w, h, c);
}

static void dark_radio_round_down_frame(int x, int y, int w, int h, Fl_Color) {
	fl_color(activated_color(fl_rgb_color(0x30, 0x30, 0x30)));
	fl_arc(x, y, w, h, 0.0, 360.0);
}

static void dark_radio_round_down_box(int x, int y, int w, int h, Fl_Color c) {
	// top edges
	fl_color(activated_color(fl_rgb_color(0x75, 0x75, 0x75)));
	fl_arc(x+1, y+1, w-2, h-2, 0.0, 180.0);
	// bottom edges
	fl_color(activated_color(fl_rgb_color(0x62, 0x62, 0x62)));
	fl_arc(x+1, y+1, w-2, h-2, 180.0, 360.0);
	// gradient
	vertical_gradient(x+2, y+2, x+w-3, y+h-3, fl_rgb_color(0x74, 0x74, 0x74), fl_rgb_color(0x63, 0x63, 0x63));
	dark_radio_round_down_frame(x, y, w, h, c);
}

static void dark_depressed_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top outer border
	fl_color(activated_color(fl_rgb_color(0x27, 0x27, 0x27)));
	fl_xyline(x+2, y, x+w-3);
	// side outer borders
	fl_color(activated_color(fl_rgb_color(0x2C, 0x2C, 0x2C)));
	fl_yxline(x, y+2, y+h-3);
	fl_yxline(x+w-1, y+2, y+h-3);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x30, 0x30, 0x30)));
	fl_xyline(x+2, y+h-1, x+w-3);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0x33, 0x33, 0x33)));
	fl_xyline(x+2, y+1, x+w-3);
	// corners
	fl_color(activated_color(fl_rgb_color(0x32, 0x32, 0x32)));
	fl_xyline(x, y+1, x+1, y);
	fl_xyline(x, y+h-2, x+1, y+h-1);
	fl_yxline(x+w-2, y, y+1, x+w-1);
	fl_yxline(x+w-2, y+h-1, y+h-2, x+w-1);
	fl_color(activated_color(fl_rgb_color(0x4B, 0x4B, 0x4B)));
	fl_point(x, y);
	fl_point(x+w-1, y);
	fl_point(x, y+h-1);
	fl_point(x+w-1, y+h-1);
}

static void dark_depressed_down_box(int x, int y, int w, int h, Fl_Color c) {
	vertical_gradient(x+1, y+2, x+w-2, y+h-1, activated_color(fl_rgb_color(0x3F, 0x3F, 0x3F)),
		activated_color(fl_rgb_color(0x37, 0x37, 0x37)));
	dark_depressed_down_frame(x, y, w, h, c);
}

static void dark_input_thin_down_frame(int x, int y, int w, int h, Fl_Color) {
	// top and side outer borders
	fl_color(activated_color(fl_rgb_color(0x30, 0x30, 0x30)));
	fl_xyline(x, y, x+w-1);
	fl_yxline(x, y+1, y+h-2);
	fl_yxline(x+w-1, y+1, y+h-2);
	// bottom outer border
	fl_color(activated_color(fl_rgb_color(0x29, 0x29, 0x29)));
	fl_xyline(x, y+h-1, x+w-1);
	// top inner border
	fl_color(activated_color(fl_rgb_color(0x37, 0x37, 0x37)));
	fl_xyline(x+1, y+1, x+w-2);
	// top and side innermost borders
	fl_color(activated_color(fl_rgb_color(0x39, 0x39, 0x39)));
	fl_xyline(x+1, y+2, x+w-2);
	fl_yxline(x+1, y+3, y+h-2);
	fl_yxline(x+w-2, y+3, y+h-2);
}

static void dark_input_thin_down_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+3, w-4, h-4);
	dark_input_thin_down_frame(x, y, w, h, c);
}

static void dark_swatch_frame(int x, int y, int w, int h, Fl_Color) {
	// outer border
	fl_color(activated_color(fl_rgb_color(0x25, 0x25, 0x25)));
	fl_rect(x, y, w, h);
	// inner border
	fl_color(activated_color(fl_rgb_color(0xFF, 0xFF, 0xFF)));
	fl_rect(x+1, y+1, w-2, h-2);
}

static void dark_swatch_box(int x, int y, int w, int h, Fl_Color c) {
	fl_color(activated_color(c));
	fl_rectf(x+2, y+2, w-4, h-4);
	dark_swatch_frame(x, y, w, h, c);
}

static void use_dark_scheme() {
	Fl::scheme("gtk+");
	Fl::set_boxtype(OS_BUTTON_UP_BOX, dark_button_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_BUTTON_UP_FRAME, dark_button_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_CHECK_DOWN_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_PANEL_THIN_UP_BOX, dark_panel_thin_up_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_BOX, dark_spacer_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_PANEL_THIN_UP_FRAME, dark_panel_thin_up_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_SPACER_THIN_DOWN_FRAME, dark_spacer_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_RADIO_ROUND_DOWN_BOX, dark_radio_round_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_HOVERED_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_BOX, dark_depressed_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_HOVERED_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_DEPRESSED_DOWN_FRAME, dark_depressed_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_BOX, dark_input_thin_down_box, 1, 1, 2, 2);
	Fl::set_boxtype(OS_INPUT_THIN_DOWN_FRAME, dark_input_thin_down_frame, 1, 1, 2, 2);
	Fl::set_boxtype(OS_DEFAULT_BUTTON_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_TOOLBAR_BUTTON_HOVER_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_SWATCH_FRAME, dark_swatch_frame, 2, 2, 4, 4);
	Fl::set_boxtype(OS_SWATCH_BOX, dark_swatch_box, 2, 2, 4, 4);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_BOX, OS_DEPRESSED_DOWN_BOX);
	Fl::set_boxtype(OS_MINI_BUTTON_UP_FRAME, OS_BUTTON_UP_FRAME);
	Fl::set_boxtype(OS_MINI_DEPRESSED_DOWN_FRAME, OS_DEPRESSED_DOWN_FRAME);
	Fl::set_boxtype(FL_UP_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_DOWN_BOX, OS_BUTTON_UP_BOX);
	Fl::set_boxtype(FL_ROUND_DOWN_BOX, OS_RADIO_ROUND_DOWN_BOX);
}

static void use_dark_colors() {
	Fl::background(0x53, 0x53, 0x53);
	Fl::background2(0x3A, 0x3A, 0x3A);
	Fl::foreground(0xFF, 0xFF, 0xFF);
	Fl::set_color(FL_SELECTION_COLOR, 0xD6, 0xD6, 0xD6);
	Fl_Tooltip::color(fl_rgb_color(0xFF, 0xFF, 0xCC));
	Fl_Tooltip::textcolor(fl_rgb_color(0x00, 0x00, 0x00));
}

void OS::use_dark_theme() {
	use_dark_scheme();
	use_dark_colors();
	use_native_settings();
	_current_theme = DARK;
}

/********************************** OS Native *********************************/

#ifdef _WIN32
OS::Theme OS::_current_theme = is_classic_windows() ? CLASSIC : is_modern_windows() ? METRO : AERO;
#else
OS::Theme OS::_current_theme = GREYBIRD;
#endif

bool OS::_is_consolas = false;

void OS::use_native_fonts() {
	Fl::set_font(OS_FONT, FL_HELVETICA);
#ifdef _WIN32
	// Use system UI font
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	char system_font[256] = {};
	int n = wcstombs(system_font, metrics.lfMessageFont.lfFaceName, sizeof(system_font));
	if (n == sizeof(system_font)) {
		system_font[sizeof(system_font)-1] = '\0';
	}
	bool found = n > 0;
	if (found) {
		found = use_font(OS_FONT, system_font);
	}
	if (!found) {
		const char *system_fonts[3] = {"Segoe UI", "Tahoma", "MS Sans Serif"};
		use_any_font(OS_FONT, system_fonts, sizeof(system_fonts));
	}
	// Use common monospace font
	const char *monospace_fonts[3] = {"Consolas", "Lucida Console", "Courier New"};
	int monospace_i = use_any_font(FL_COURIER, monospace_fonts, sizeof(monospace_fonts));
	if (monospace_i == 0) { _is_consolas = true; }
	// Use common bold monospace font
	const char *bold_monospace_fonts[3] = {
		"Consolas bold", "Lucida Console bold", "Courier New bold"
	};
	use_any_font(FL_COURIER_BOLD, bold_monospace_fonts, sizeof(bold_monospace_fonts));
	// Use common italic monospace font
	const char *italic_monospace_fonts[3] = {
		"Consolas italic", "Lucida Console italic", "Courier New italic"
	};
	use_any_font(FL_COURIER_ITALIC, italic_monospace_fonts, sizeof(italic_monospace_fonts));
#else
	// Use common system UI font
	const char *system_fonts[4] = {
		"Ubuntu", "Noto Sans", "Droid Sans", "DejaVu Sans"
	};
	use_any_font(OS_FONT, system_fonts, sizeof(system_fonts));
	// Use common monospace font
	const char *monospace_fonts[4] = {
		"Ubuntu Mono", "Noto Sans Mono", "Droid Sans Mono", "DejaVu Sans Mono"
	};
	use_any_font(FL_COURIER, monospace_fonts, sizeof(monospace_fonts));
	// Use common bold monospace font
	const char *bold_monospace_fonts[4] = {
		"Ubuntu Mono bold", "Noto Sans Mono bold", "Droid Sans Mono bold", "DejaVu Sans Mono bold"
	};
	use_any_font(FL_COURIER_BOLD, bold_monospace_fonts, sizeof(bold_monospace_fonts));
	// Use common italic monospace font
	const char *italic_monospace_fonts[4] = {
		"Ubuntu Mono italic", "Noto Sans Mono italic", "Droid Sans Mono italic", "DejaVu Sans Mono italic"
	};
	use_any_font(FL_COURIER_ITALIC, italic_monospace_fonts, sizeof(italic_monospace_fonts));
#endif
	fl_font(OS_FONT, OS_FONT_SIZE);
}

void OS::use_native_scheme() {
#ifdef _WIN32
	if (is_classic_windows()) {
		use_classic_scheme();
	}
	else if (is_modern_windows()) {
		use_metro_scheme();
	}
	else {
		use_aero_scheme();
	}
#else
	use_greybird_scheme();
#endif
}

void OS::use_native_colors() {
#ifdef _WIN32
	if (is_classic_windows()) {
		use_classic_colors();
	}
	else if (is_modern_windows()) {
		use_metro_colors();
	}
	else {
		use_aero_colors();
	}
#else
	use_greybird_colors();
#endif
}

void OS::use_native_settings() {
	Fl::visible_focus(0);
	Fl::scrollbar_size(15);
	Fl_Tooltip::font(OS_FONT);
	Fl_Tooltip::size(OS_FONT_SIZE);
	Fl_Tooltip::delay(0.5f);
}

void OS::use_native_theme() {
#ifdef _WIN32
	if (is_modern_windows()) {
		use_metro_theme();
	}
	else {
		use_aero_theme();
	}
#else
	use_greybird_theme();
#endif
}
