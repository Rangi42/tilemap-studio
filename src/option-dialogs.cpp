#include <cctype>
#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Enumerations.H>
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

#include "preferences.h"
#include "themes.h"
#include "widgets.h"
#include "utils.h"
#include "option-dialogs.h"
#include "tileset.h"
#include "config.h"
#include "icons.h"
#include "image.h"
#include "tile.h"

Option_Dialog::Option_Dialog(int w, const char *t) : _width(w), _title(t), _canceled(false),
	_dialog(NULL), _content(NULL), _ok_button(NULL), _cancel_button(NULL) {}

Option_Dialog::~Option_Dialog() {
	delete _dialog;
	delete _content;
	delete _ok_button;
	delete _cancel_button;
}

void Option_Dialog::initialize() {
	if (_dialog) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate dialog
	_dialog = new Fl_Double_Window(0, 0, 0, 0, _title);
	_content = new Fl_Group(0, 0, 0, 0);
	_content->begin();
	initialize_content();
	_content->end();
	_dialog->begin();
	_ok_button = new Default_Button(0, 0, 0, 0, "OK");
	_cancel_button = new OS_Button(0, 0, 0, 0, "Cancel");
	_dialog->end();
	// Initialize dialog
	_dialog->box(OS_BG_BOX);
	_dialog->resizable(NULL);
	_dialog->callback((Fl_Callback *)cancel_cb, this);
	_dialog->set_modal();
	// Initialize dialog's children
	_ok_button->tooltip("OK (Enter)");
	_ok_button->callback((Fl_Callback *)close_cb, this);
	_cancel_button->shortcut(FL_Escape);
	_cancel_button->tooltip("Cancel (Esc)");
	_cancel_button->callback((Fl_Callback *)cancel_cb, this);
	Fl_Group::current(prev_current);
}

void Option_Dialog::refresh() {
	_canceled = false;
	_dialog->copy_label(_title);
	// Refresh widget positions and sizes
	fl_font(OS_FONT, OS_FONT_SIZE);
	int dy = 10;
	dy += refresh_content(_width - 20, dy) + 16;
#ifdef _WIN32
	_ok_button->resize(_width - 184, dy, 80, 22);
	_cancel_button->resize(_width - 90, dy, 80, 22);
#else
	_cancel_button->resize(_width - 184, dy, 80, 22);
	_ok_button->resize(_width - 90, dy, 80, 22);
#endif
	dy += _cancel_button->h() + 10;
	_dialog->size_range(_width, dy, _width, dy);
	_dialog->size(_width, dy);
	_dialog->redraw();
}

void Option_Dialog::reveal(const Fl_Widget *p) {
	int x = p->x() + (p->w() - _dialog->w()) / 2;
	int y = p->y() + (p->h() - _dialog->h()) / 2;
	_dialog->position(x, y);
	_ok_button->take_focus();
	_dialog->show();
	while (_dialog->shown()) { Fl::wait(); }
}

void Option_Dialog::close_cb(Fl_Widget *, Option_Dialog *od) {
	od->finish();
	od->_dialog->hide();
}

void Option_Dialog::cancel_cb(Fl_Widget *, Option_Dialog *od) {
	od->_canceled = true;
	od->_dialog->hide();
}

Tilemap_Options_Dialog::Tilemap_Options_Dialog(const char *t) : Option_Dialog(280, t), _tilemap_header(NULL),
	_format(NULL), _attrmap_heading(NULL), _attrmap(NULL), _attrmap_name(NULL), _attrmap_chooser(NULL),
	_attrmap_import_chooser(NULL), _attrmap_filename(), _importing(false) {}

Tilemap_Options_Dialog::~Tilemap_Options_Dialog() {
	delete _tilemap_header;
	delete _format;
	delete _attrmap_heading;
	delete _attrmap;
	delete _attrmap_name;
	delete _attrmap_chooser;
	delete _attrmap_import_chooser;
}

void Tilemap_Options_Dialog::update_icons() {
	initialize();
	Image::make_deimage(_attrmap);
}

void Tilemap_Options_Dialog::use_tilemap(const char *filename) {
	initialize();
	const char *name = fl_filename_name(filename);
	char buffer[FL_PATH_MAX] = {};
	strcpy(buffer, name);
	strcat(buffer, ":");
	_tilemap_header->copy_label(buffer);

	if (format_has_attrmap(format())) {
		strcpy(buffer, filename);
		fl_filename_setext(buffer, sizeof(buffer), ATTRMAP_EXT);
		_attrmap_filename = buffer;
		_attrmap_heading->activate();
		_attrmap->activate();
		_attrmap_name->activate();
		const char *basename = fl_filename_name(buffer);
		_attrmap_name->copy_label(basename);
	}
	else {
		_attrmap_filename.clear();
		_attrmap_heading->deactivate();
		_attrmap->deactivate();
		_attrmap_name->deactivate();
		_attrmap_name->copy_label(NO_FILE_SELECTED_LABEL);
	}

	_ok_button->activate();
}

void Tilemap_Options_Dialog::initialize_content() {
	// Populate content group
	_tilemap_header = new Label(0, 0, 0, 0);
	_format = new Dropdown(0, 0, 0, 0, "Format:");
	_attrmap_heading = new Label(0, 0, 0, 0, "Attrmap:");
	_attrmap = new Toolbar_Button(0, 0, 0, 0);
	_attrmap_name = new Label_Button(0, 0, 0, 0, NO_FILE_SELECTED_LABEL);
	_attrmap_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	_attrmap_import_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	// Initialize content group's children
	for (int i = 0; i < NUM_FORMATS; i++) {
		_format->add(format_name((Tilemap_Format)i));
	}
	_format->callback((Fl_Callback *)format_cb, this);
	_attrmap->callback((Fl_Callback *)attrmap_cb, this);
	_attrmap->image(OPEN_ICON);
	_attrmap_name->callback((Fl_Callback *)attrmap_cb, this);
	_attrmap_chooser->title("Open Attrmap");
	_attrmap_chooser->filter("Attrmap Files\t*.attrmap\n");
	_attrmap_import_chooser->title("Import Attrmap");
	_attrmap_import_chooser->filter("Importable Files\t*.{c,csv}\n");
}

int Tilemap_Options_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = (wgt_h + wgt_m) * 2 + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	_tilemap_header->resize(win_m, dy, ww, wgt_h);
	dy += wgt_h + wgt_m;
	int wgt_off = win_m + text_width(_format->label(), 2);
	int wgt_w = format_max_name_width() + wgt_h;
	_format->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_attrmap_heading->label(), 4);
	wgt_off = win_m;
	_attrmap_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _attrmap_heading->w();
	_attrmap->resize(wgt_off, dy, wgt_h, wgt_h);
	wgt_off += _attrmap->w();
	_attrmap_name->resize(wgt_off, dy, ww-wgt_w-wgt_h, wgt_h);

	return ch;
}

void Tilemap_Options_Dialog::format_cb(Dropdown *, Tilemap_Options_Dialog *tod) {
	if (format_has_attrmap(tod->format())) {
		tod->_attrmap_heading->activate();
		tod->_attrmap->activate();
		tod->_attrmap_name->activate();
		const char *filename = (tod->_importing ? tod->_attrmap_import_chooser : tod->_attrmap_chooser)->filename();
		if (filename && strlen(filename)) {
			const char *basename = fl_filename_name(filename);
			tod->_attrmap_filename = filename;
			tod->_attrmap_name->copy_label(basename);
			tod->_ok_button->activate();
		}
		else {
			tod->_attrmap_filename.clear();
			tod->_attrmap_name->copy_label(NO_FILE_SELECTED_LABEL);
			tod->_ok_button->deactivate();
		}
	}
	else {
		tod->_attrmap_filename.clear();
		tod->_attrmap_heading->deactivate();
		tod->_attrmap->deactivate();
		tod->_attrmap_name->deactivate();
		tod->_attrmap_name->copy_label(NO_FILE_SELECTED_LABEL);
		tod->_ok_button->activate();
	}
	tod->_dialog->redraw();
}

void Tilemap_Options_Dialog::attrmap_cb(Fl_Widget *, Tilemap_Options_Dialog *tod) {
	Fl_Native_File_Chooser *fc = tod->_importing ? tod->_attrmap_import_chooser : tod->_attrmap_chooser;
	int status = fc->show();
	if (status == 1) {
		tod->_attrmap_filename.clear();
		tod->_attrmap_name->label(NO_FILE_SELECTED_LABEL);
		tod->_ok_button->deactivate();
	}
	else {
		const char *filename = fc->filename();
		const char *basename = fl_filename_name(filename);
		tod->_attrmap_filename = filename;
		tod->_attrmap_name->copy_label(basename);
		tod->_ok_button->activate();
	}
	tod->_dialog->redraw();
}

New_Tilemap_Dialog::New_Tilemap_Dialog(const char *t) : Option_Dialog(280, t), _tilemap_width(NULL), _tilemap_height(NULL),
	_format(NULL) {}

New_Tilemap_Dialog::~New_Tilemap_Dialog() {
	delete _tilemap_width;
	delete _tilemap_height;
	delete _format;
}

void New_Tilemap_Dialog::initialize_content() {
	// Populate content group
	_tilemap_width = new OS_Spinner(0, 0, 0, 0, "Width:");
	_tilemap_height = new OS_Spinner(0, 0, 0, 0, "Height:");
	_format = new Dropdown(0, 0, 0, 0, "Format:");
	// Initialize content group's children
	_tilemap_width->range(1, 1024);
	_tilemap_height->range(1, 1024);
	for (int i = 0; i < NUM_FORMATS; i++) {
		_format->add(format_name((Tilemap_Format)i));
	}
}

int New_Tilemap_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = wgt_h + wgt_m + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	int wgt_off = win_m + std::max(text_width(_tilemap_width->label(), 2), text_width(_format->label(), 2));
	int wgt_w = text_width("9999", 2) + wgt_h / 2 + 4;
	_tilemap_width->resize(wgt_off, dy, wgt_w, wgt_h);
	int wgt_off2 = _tilemap_width->x() + _tilemap_width->w() + win_m + text_width("Height:", 2);
	_tilemap_height->resize(wgt_off2, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;
	wgt_w = format_max_name_width() + wgt_h;
	_format->resize(wgt_off, dy, wgt_w, wgt_h);

	return ch;
}

Print_Options_Dialog::Print_Options_Dialog(const char *t) : _title(t), _copied(false), _canceled(false), _dialog(NULL),
_show_heading(NULL), _grid(NULL), _rainbow_tiles(NULL), _palettes(NULL), _bold_palettes(NULL), _export_button(NULL), _copy_button(NULL),
_cancel_button(NULL) {}

Print_Options_Dialog::~Print_Options_Dialog() {
	delete _dialog;
	delete _show_heading;
	delete _grid;
	delete _rainbow_tiles;
	delete _palettes;
	delete _bold_palettes;
	delete _export_button;
	delete _copy_button;
	delete _cancel_button;
}

void Print_Options_Dialog::initialize() {
	if (_dialog) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate dialog
	_dialog = new Fl_Double_Window(0, 0, 0, 0, _title);
	_show_heading = new Label(0, 0, 0, 0, "Show:");
	_grid = new OS_Check_Button(0, 0, 0, 0, "Grid");
	_rainbow_tiles = new OS_Check_Button(0, 0, 0, 0, "Rainbow Tiles");
	_palettes = new OS_Check_Button(0, 0, 0, 0, "Palettes");
	_bold_palettes = new OS_Check_Button(0, 0, 0, 0, "Bold Palettes");
	_export_button = new Default_Button(0, 0, 0, 0, "Export...");
	_copy_button = new OS_Button(0, 0, 0, 0, "Copy");
	_cancel_button = new OS_Button(0, 0, 0, 0, "Cancel");
	_dialog->end();
	// Initialize dialog
	_dialog->box(OS_BG_BOX);
	_dialog->resizable(NULL);
	_dialog->callback((Fl_Callback *)cancel_cb, this);
	_dialog->set_modal();
	// Initialize dialog's children
	_export_button->tooltip("Export (Enter)");
	_export_button->callback((Fl_Callback *)close_cb, this);
	_copy_button->shortcut(FL_COMMAND + 'c');
	_copy_button->tooltip("Copy (" COMMAND_KEY_PLUS "C)");
	_copy_button->callback((Fl_Callback *)copy_cb, this);
	_cancel_button->shortcut(FL_Escape);
	_cancel_button->tooltip("Cancel (Esc)");
	_cancel_button->callback((Fl_Callback *)cancel_cb, this);
	Fl_Group::current(prev_current);
}

void Print_Options_Dialog::refresh() {
	_copied = _canceled = false;
	_dialog->copy_label(_title);
	// Refresh widget positions and sizes
	fl_font(OS_FONT, OS_FONT_SIZE);
	int btn_w = 80, wgt_h = 22, win_m = 10, wgt_m = 4;
	int dx = win_m, dy = win_m;
	int wgt_w = text_width(_show_heading->label(), 4);
	_show_heading->resize(dx, dy, wgt_w, wgt_h);
	dx += _show_heading->w() + wgt_m;
	wgt_w = text_width(_grid->label(), 2) + wgt_h;
	_grid->resize(dx, dy, wgt_w, wgt_h);
	dx += _grid->w() + wgt_m;
	wgt_w = text_width(_rainbow_tiles->label(), 2) + wgt_h;
	_rainbow_tiles->resize(dx, dy, wgt_w, wgt_h);
	dx += _rainbow_tiles->w() + wgt_m;
	wgt_w = text_width(_palettes->label(), 2) + wgt_h;
	_palettes->resize(dx, dy, wgt_w, wgt_h);
	dx += _palettes->w() + wgt_m;
	wgt_w = text_width(_bold_palettes->label(), 2) + wgt_h;
	_bold_palettes->resize(dx, dy, wgt_w, wgt_h);
	dx += _bold_palettes->w() + win_m;
	if (dx < 288) { dx = 288; }
	dy += wgt_h + 16;
#ifdef _WIN32
	_export_button->resize(dx - 278, dy, btn_w, wgt_h);
	_copy_button->resize(dx - 184, dy, btn_w, wgt_h);
	_cancel_button->resize(dx - 90, dy, btn_w, wgt_h);
#else
	_cancel_button->resize(dx - 278, dy, btn_w, wgt_h);
	_copy_button->resize(dx - 184, dy, btn_w, wgt_h);
	_export_button->resize(dx - 90, dy, btn_w, wgt_h);
#endif
	dy += wgt_h + win_m;
	_dialog->size_range(dx, dy, dx, dy);
	_dialog->size(dx, dy);
	_dialog->redraw();
}

void Print_Options_Dialog::show(const Fl_Widget *p) {
	initialize();
	refresh();
	int x = p->x() + (p->w() - _dialog->w()) / 2;
	int y = p->y() + (p->h() - _dialog->h()) / 2;
	_dialog->position(x, y);
	_export_button->take_focus();
	_dialog->show();
	while (_dialog->shown()) { Fl::wait(); }
}

void Print_Options_Dialog::close_cb(Fl_Widget *, Print_Options_Dialog *pd) {
	pd->_dialog->hide();
}

void Print_Options_Dialog::copy_cb(Fl_Widget *, Print_Options_Dialog *pd) {
	pd->_copied = true;
	pd->_dialog->hide();
}

void Print_Options_Dialog::cancel_cb(Fl_Widget *, Print_Options_Dialog *pd) {
	pd->_canceled = true;
	pd->_dialog->hide();
}

class Anchor_Button : public OS_Button {
private:
	int _anchor;
public:
	Anchor_Button(int a) : OS_Button(0, 0, 0, 0), _anchor(a) {
		type(FL_RADIO_BUTTON);
		align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	}
	inline int anchor(void) const { return _anchor; }
};

Resize_Dialog::Resize_Dialog(const char *t) : Option_Dialog(220, t), _tilemap_width(NULL), _tilemap_height(NULL), _anchor_buttons() {}

Resize_Dialog::~Resize_Dialog() {
	delete _tilemap_width;
	delete _tilemap_height;
	for (Anchor_Button *ab : _anchor_buttons) { delete ab; }
}

Resize_Dialog::Hor_Align Resize_Dialog::horizontal_anchor() const {
	if (_anchor_buttons[0]->value() || _anchor_buttons[3]->value() || _anchor_buttons[6]->value()) { return Hor_Align::LEFT; }
	if (_anchor_buttons[2]->value() || _anchor_buttons[5]->value() || _anchor_buttons[8]->value()) { return Hor_Align::RIGHT; }
	return Hor_Align::CENTER;
}

Resize_Dialog::Vert_Align Resize_Dialog::vertical_anchor() const {
	if (_anchor_buttons[0]->value() || _anchor_buttons[1]->value() || _anchor_buttons[2]->value()) { return Vert_Align::TOP; }
	if (_anchor_buttons[6]->value() || _anchor_buttons[7]->value() || _anchor_buttons[8]->value()) { return Vert_Align::BOTTOM; }
	return Vert_Align::MIDDLE;
}

int Resize_Dialog::anchor() const {
	for (int i = 0; i < 9; i++) {
		if (_anchor_buttons[i]->value()) { return i; }
	}
	return 4;
}

void Resize_Dialog::anchor(int a) {
	_anchor_buttons[a]->do_callback();
}

void Resize_Dialog::anchor_label(int x, int y, const char *l) {
	if (0 <= x && x < 3 && 0 <= y && y < 3) {
		_anchor_buttons[y * 3 + x]->label(l);
	}
}

void Resize_Dialog::initialize_content() {
	// Populate content group
	_tilemap_width = new Default_Spinner(0, 0, 0, 0, "Width:");
	_tilemap_height = new Default_Spinner(0, 0, 0, 0, "Height:");
	for (int i = 0; i < 9; i++) {
		Anchor_Button *ab = new Anchor_Button(i);
		ab->callback((Fl_Callback *)anchor_button_cb, this);
		_anchor_buttons[i] = ab;
	}
	// Initialize content group's children
	_tilemap_width->range(1, 1024);
	_tilemap_height->range(1, 9999);
	anchor(Preferences::get("resize-anchor", 4));
}

int Resize_Dialog::refresh_content(int ww, int dy) {
	int wgt_w = 0, wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = (wgt_h + wgt_m) * 2 + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	int wgt_off = win_m + std::max(text_width(_tilemap_width->label(), 2), text_width(_tilemap_height->label(), 2));

	wgt_w = text_width("9999", 2) + wgt_h / 2 + 4;
	_tilemap_width->resize(wgt_off, dy, wgt_w, wgt_h);
	_tilemap_height->resize(wgt_off, _tilemap_width->y() + _tilemap_width->h() + wgt_m, wgt_w, wgt_h);
	wgt_off += wgt_w + 20;

	wgt_w = wgt_h = 24;
	wgt_m = 2;
	for (int ay = 0; ay < 3; ay++) {
		for (int ax = 0; ax < 3; ax++) {
			_anchor_buttons[ay*3+ax]->resize(wgt_off + (wgt_w + wgt_m) * ax, dy, wgt_w, wgt_h);
		}
		dy += wgt_h + wgt_m;
	}

	return ch;
}

void Resize_Dialog::anchor_button_cb(Anchor_Button *ab, Resize_Dialog *rd) {
	ab->setonly();
	for (Anchor_Button *abi : rd->_anchor_buttons) {
		abi->label(NULL);
	}
	int a = ab->anchor();
	int y = a / 3, x = a % 3;
	rd->anchor_label(x - 1, y - 1, "@7>"); // top-left
	rd->anchor_label(x,     y - 1, "@8>"); // top
	rd->anchor_label(x + 1, y - 1, "@9>"); // top-right
	rd->anchor_label(x - 1,  y,    "@4>"); // left
	rd->anchor_label(x,      y,    "@-2square"); // center
	rd->anchor_label(x + 1,  y,    "@>");  // right
	rd->anchor_label(x - 1, y + 1, "@1>"); // bottom-left
	rd->anchor_label(x,     y + 1, "@2>"); // bottom
	rd->anchor_label(x + 1, y + 1, "@3>"); // bottom-right
	rd->_dialog->redraw();
}

Shift_Dialog::Shift_Dialog(const char *t) : Option_Dialog(220, t), _shift_x(NULL), _shift_y(NULL), _slide_x(NULL), _slide_y(NULL) {}

Shift_Dialog::~Shift_Dialog() {
	delete _shift_x;
	delete _shift_y;
	delete _slide_x;
	delete _slide_y;
}

void Shift_Dialog::limit_shift(int w, int h) {
	initialize();
	_shift_x->range(-w, w);
	_shift_x->value(0);
	_shift_y->range(-h, h);
	_shift_y->value(0);
	_slide_x->range(-w, w);
	_slide_x->value(0);
	_slide_y->range(-h, h);
	_slide_y->value(0);
}

void Shift_Dialog::initialize_content() {
	// Populate content group
	_shift_x = new Default_Spinner(0, 0, 0, 0, "X:");
	_shift_y = new Default_Spinner(0, 0, 0, 0, "Y:");
	_slide_x = new Default_Slider(0, 0, 0, 0);
	_slide_y = new Default_Slider(0, 0, 0, 0);
	// Initialize content group's children
	_shift_x->default_value(0);
	_shift_x->callback((Fl_Callback *)shift_x_cb, this);
	_shift_y->default_value(0);
	_shift_y->callback((Fl_Callback *)shift_y_cb, this);
	_slide_x->step(1);
	_slide_x->default_value(0);
	_slide_x->callback((Fl_Callback *)slide_x_cb, this);
	_slide_y->step(1);
	_slide_y->default_value(0);
	_slide_y->callback((Fl_Callback *)slide_y_cb, this);
}

int Shift_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = wgt_h + wgt_m + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	int wgt_off = win_m + std::max(text_width(_shift_x->label(), 2), text_width(_shift_y->label(), 2));
	int wgt_w = text_width("9999", 2) + wgt_h / 2 + 4;
	int dy2 = dy + wgt_h + wgt_m;
	_shift_x->resize(wgt_off, dy, wgt_w, wgt_h);
	_shift_y->resize(wgt_off, dy2, wgt_w, wgt_h);
	wgt_off += wgt_w + 6;
	wgt_w = ww + win_m - wgt_off;
	double sw = (wgt_h / 2.0) / wgt_w;
	_slide_x->resize(wgt_off, dy, wgt_w, wgt_h);
	_slide_x->slider_size(sw);
	_slide_y->resize(wgt_off, dy2, wgt_w, wgt_h);
	_slide_y->slider_size(sw);

	return ch;
}

void Shift_Dialog::shift_x_cb(Default_Spinner *, Shift_Dialog *sd) {
	sd->_slide_x->value(sd->_shift_x->value());
	sd->_slide_x->redraw();
}

void Shift_Dialog::shift_y_cb(Default_Spinner *, Shift_Dialog *sd) {
	sd->_slide_y->value(sd->_shift_y->value());
	sd->_slide_y->redraw();
}

void Shift_Dialog::slide_x_cb(Default_Slider *, Shift_Dialog *sd) {
	sd->_shift_x->value(sd->_slide_x->value());
	sd->_shift_x->redraw();
}

void Shift_Dialog::slide_y_cb(Default_Slider *, Shift_Dialog *sd) {
	sd->_shift_y->value(sd->_slide_y->value());
	sd->_shift_y->redraw();
}

Shift_Tileset_Dialog::Shift_Tileset_Dialog(const char *t) : Option_Dialog(220, t), _shift(NULL), _slide(NULL) {}

Shift_Tileset_Dialog::~Shift_Tileset_Dialog() {
	delete _shift;
	delete _slide;
}

void Shift_Tileset_Dialog::limit_shift(int n) {
	initialize();
	_shift->range(-n, n);
	_shift->value(0);
	_slide->range(-n, n);
	_slide->value(0);
}

void Shift_Tileset_Dialog::initialize_content() {
	// Populate content group
	_shift = new Default_Hex_Spinner(0, 0, 0, 0, "Shift: $");
	_slide = new Default_Slider(0, 0, 0, 0);
	// Initialize content group's children
	_shift->format("%X");
	_shift->default_value(0);
	_shift->callback((Fl_Callback *)shift_cb, this);
	_slide->step(1);
	_slide->default_value(0);
	_slide->callback((Fl_Callback *)slide_cb, this);
}

int Shift_Tileset_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10;
	_content->resize(win_m, dy, ww, wgt_h);

	int wgt_off = win_m + text_width(_shift->label(), 2);
	int wgt_w = std::max(text_width("-AAA", 2), text_width("-FFF", 2)) + wgt_h / 2 + 4;
	_shift->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += wgt_w + 6;
	wgt_w = ww + win_m - wgt_off;
	_slide->resize(wgt_off, dy, wgt_w, wgt_h);
	_slide->slider_size((wgt_h / 2.0) / wgt_w);

	return wgt_h;
}

void Shift_Tileset_Dialog::shift_cb(Default_Hex_Spinner *, Shift_Tileset_Dialog *sd) {
	sd->_slide->value(sd->_shift->value());
	sd->_slide->redraw();
}

void Shift_Tileset_Dialog::slide_cb(Default_Slider *, Shift_Tileset_Dialog *sd) {
	sd->_shift->value((int)sd->_slide->value());
	sd->_shift->redraw();
}

Reformat_Dialog::Reformat_Dialog(const char *t) : Option_Dialog(280, t), _format_header(NULL), _format(NULL), _force(NULL) {}

Reformat_Dialog::~Reformat_Dialog() {
	delete _format_header;
	delete _format;
	delete _force;
}

void Reformat_Dialog::format(Tilemap_Format fmt) {
	initialize();
	_format->value((int)fmt);
	const char *name = format_name(fmt);
	char buffer[256] = {};
	strcpy(buffer, "Convert ");
	strcat(buffer, name);
	strcat(buffer, " to:");
	_format_header->copy_label(buffer);
}

void Reformat_Dialog::initialize_content() {
	// Populate content group
	_format_header = new Label(0, 0, 0, 0);
	_format = new Dropdown(0, 0, 0, 0, "Format:");
	_force = new OS_Check_Button(0, 0, 0, 0, "Force (discards unformattable data)");
	// Initialize content group's children
	for (int i = 0; i < NUM_FORMATS; i++) {
		_format->add(format_name((Tilemap_Format)i));
	}
}

int Reformat_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = (wgt_h + wgt_m) * 2 + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	_format_header->resize(win_m, dy, ww, wgt_h);
	dy += wgt_h + wgt_m;
	int wgt_off = win_m + text_width(_format->label(), 2);
	int wgt_w = format_max_name_width() + wgt_h;
	_format->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;
	_force->resize(win_m, dy, ww, wgt_h);

	return ch;
}

Group_Width_Dialog::Group_Width_Dialog(const char *t) : Option_Dialog(194, t), _group_width(NULL) {}

Group_Width_Dialog::~Group_Width_Dialog() {
	delete _group_width;
}

void Group_Width_Dialog::initialize_content() {
	// Populate content group
	_group_width = new Default_Spinner(0, 0, 0, 0, "Width:");
	// Initialize content group's children
	_group_width->align(FL_ALIGN_LEFT);
	_group_width->default_value(16);
	_group_width->range(1, 1024);
}

int Group_Width_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10;
	_content->resize(win_m, dy, ww, wgt_h);

	int wgt_off = win_m + text_width(_group_width->label(), 2);
	int wgt_w = text_width("9999", 2) + wgt_h / 2 + 4;
	_group_width->resize(wgt_off, dy, wgt_w, wgt_h);

	return wgt_h;
}

Add_Tileset_Dialog::Add_Tileset_Dialog(const char *t) : Option_Dialog(270, t), _tileset_header(NULL), _start_id(NULL),
	_offset(NULL), _length(NULL) {}

Add_Tileset_Dialog::~Add_Tileset_Dialog() {
	delete _tileset_header;
	delete _start_id;
	delete _offset;
	delete _length;
}

void Add_Tileset_Dialog::limit_tileset_options(const char *filename) {
	initialize();
	const char *name = fl_filename_name(filename);
	char buffer[FL_PATH_MAX] = {};
	strcpy(buffer, name);
	strcat(buffer, ":");
	_tileset_header->copy_label(buffer);
}

void Add_Tileset_Dialog::initialize_content() {
	// Populate content group
	_tileset_header = new Label(0, 0, 0, 0);
	_start_id = new Default_Hex_Spinner(0, 0, 0, 0, "Start at ID: $");
	_offset = new Default_Hex_Spinner(0, 0, 0, 0, "Offset: $");
	_length = new Default_Hex_Spinner(0, 0, 0, 0, "Length: $");
	// Initialize content group's children
	_start_id->format("%03X");
	_start_id->range(0x00, MAX_NUM_TILES-1);
	_start_id->default_value(0x000);
	_offset->align(FL_ALIGN_LEFT);
	_offset->format("%X");
	_offset->range(0x0, 0x400);
	_offset->default_value(0x0);
	_length->align(FL_ALIGN_LEFT);
	_length->format("%X");
	_length->range(0x0, 0x400);
	_length->default_value(0x0);
}

int Add_Tileset_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = (wgt_h + wgt_m) * 2 + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	_tileset_header->resize(win_m, dy, ww, wgt_h);
	dy += wgt_h + wgt_m;
	int wgt_off = win_m + text_width(_start_id->label(), 3);
	int wgt_w = std::max(text_width("AAA", 2), text_width("FFF", 2)) + wgt_h / 2 + 4;
	_start_id->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;
	wgt_off = win_m + text_width(_offset->label(), 3);
	_offset->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off = _offset->x() + _offset->w() + win_m + text_width(_length->label(), 3);
	_length->resize(wgt_off, dy, wgt_w, wgt_h);

	return ch;
}

Image_To_Tiles_Dialog::Image_To_Tiles_Dialog(const char *t) : Option_Dialog(360, t), _tileset_heading(NULL), _tilemap_heading(NULL),
	_tileset_spacer(NULL), _tilemap_spacer(NULL), _palette_spacer(NULL), _input_heading(NULL), _output_heading(NULL), _image(NULL),
	_tileset(NULL), _image_name(NULL), _tileset_name(NULL), _unique_tiles(NULL), _flip_tiles(NULL), _no_extra_blank_tiles(NULL),
	_tilemap_name(NULL), _format(NULL), _start_id(NULL), _use_blank(NULL), _blank_id(NULL), _palette(NULL), _palette_name(NULL),
	_palette_format(NULL), _start_index_label(NULL), _start_index(NULL), _color_zero(NULL), _color_zero_rgb(NULL), _color_zero_swatch(NULL),
	_image_chooser(NULL), _tileset_chooser(NULL), _image_filename(), _tileset_filename(), _tilemap_filename(), _attrmap_filename(),
	_palette_filename(), _tilepal_filename(), _prepared_image(false), _picked_palette(false) {}

Image_To_Tiles_Dialog::~Image_To_Tiles_Dialog() {
	delete _tileset_heading;
	delete _tilemap_heading;
	delete _tileset_spacer;
	delete _tilemap_spacer;
	delete _palette_spacer;
	delete _input_heading;
	delete _output_heading;
	delete _image;
	delete _tileset;
	delete _image_name;
	delete _tileset_name;
	delete _unique_tiles;
	delete _flip_tiles;
	delete _no_extra_blank_tiles;
	delete _tilemap_name;
	delete _format;
	delete _start_id;
	delete _use_blank;
	delete _blank_id;
	delete _palette;
	delete _palette_name;
	delete _palette_format;
	delete _start_index_label;
	delete _start_index;
	delete _color_zero;
	delete _color_zero_rgb;
	delete _color_zero_swatch;
	delete _image_chooser;
	delete _tileset_chooser;
}

Fl_Color Image_To_Tiles_Dialog::fl_color_zero() const {
	const char *s = _color_zero_rgb->value();
	char rgb[7] = {};
	if (size_t n = strlen(s); n < 6) {
		memset(rgb, '0', 6 - n);
	}
	strncat(rgb, s, 6);

	char buffer[3] = {};
	buffer[0] = rgb[0];
	buffer[1] = rgb[1];
	uchar r = (uchar)strtoul(buffer, NULL, 16);
	buffer[0] = rgb[2];
	buffer[1] = rgb[3];
	uchar g = (uchar)strtoul(buffer, NULL, 16);
	buffer[0] = rgb[4];
	buffer[1] = rgb[5];
	uchar b = (uchar)strtoul(buffer, NULL, 16);

	return fl_rgb_color(NORMRGB(r), NORMRGB(g), NORMRGB(b));
}

void Image_To_Tiles_Dialog::update_image_name() {
	if (_image_filename.empty()) {
		_image_name->label(NO_FILE_SELECTED_LABEL);
	}
	else {
		const char *basename = fl_filename_name(image_filename());
		_image_name->copy_label(basename);
	}
}

void Image_To_Tiles_Dialog::update_output_names() {
	if (_tileset_filename.empty()) {
		_tileset_name->label(NO_FILE_SELECTED_LABEL);
		_tilemap_filename.clear();
		_attrmap_filename.clear();
		_palette_filename.clear();
		_tilepal_filename.clear();
		_tilemap_name->label("Output: " NO_FILES_DETERMINED_LABEL);
	}
	else {
		_tileset_name->copy_label(fl_filename_name(tileset_filename()));

		char output_filename[FL_PATH_MAX] = {};
		strcpy(output_filename, tileset_filename());
		fl_filename_setext(output_filename, sizeof(output_filename), format_extension(format()));
		_tilemap_filename = output_filename;

		strcpy(output_filename, tileset_filename());
		fl_filename_setext(output_filename, sizeof(output_filename), ATTRMAP_EXT);
		_attrmap_filename = output_filename;

		strcpy(output_filename, tileset_filename());
		const char *palette_ext = palette_extension(palette_format());
		if (palette_ext) {
			fl_filename_setext(output_filename, sizeof(output_filename), palette_ext);
		}
		_palette_filename = output_filename;

		strcpy(output_filename, tileset_filename());
		fl_filename_setext(output_filename, sizeof(output_filename), TILEPAL_EXT);
		_tilepal_filename = output_filename;

		char tilemap_name[FL_PATH_MAX] = {};
		strcpy(tilemap_name, "Output: ");
		strcat(tilemap_name, fl_filename_name(tilemap_filename()));
		if (format_has_attrmap(format())) {
			strcat(tilemap_name, " / ");
			strcat(tilemap_name, fl_filename_name(attrmap_filename()));
		}
		_tilemap_name->copy_label(tilemap_name);
	}

	if (format_can_make_palettes(format())) {
		_palette->activate();
		_palette->do_callback();
		if (_palette_filename.empty()) {
			_palette_name->label("Output: " NO_FILES_DETERMINED_LABEL);
		}
		else {
			char pal_name[FL_PATH_MAX] = {};
			strcpy(pal_name, "Output: ");
			strcat(pal_name, fl_filename_name(palette_filename()));
			if (format_has_per_tile_palettes(format())) {
				strcat(pal_name, " / ");
				strcat(pal_name, fl_filename_name(tilepal_filename()));
			}
			_palette_name->copy_label(pal_name);
		}
	}
	else {
		_palette->clear();
		_palette_format->value(0);
		_palette_name->label("N/A");
		_palette->deactivate();
		_palette->do_callback();
	}
}

void Image_To_Tiles_Dialog::update_flip_tiles() {
	if (format_can_flip(format()) && unique_tiles()) {
		if (!_flip_tiles->active()) {
			_flip_tiles->value(1);
		}
		_flip_tiles->activate();
	}
	else {
		_flip_tiles->deactivate();
		_flip_tiles->value(0);
	}
}

void Image_To_Tiles_Dialog::update_start_index() {
	if (format_can_make_palettes(format())) {
		_start_index_label->activate();
		_start_index->activate();
		int nx = format_palettes_size(format());
		if (nx == 1) {
			nx = format_palette_size(format());
			_start_index_label->label("Start at Index:");
		}
		else {
			_start_index_label->label("Start at Palette:");
		}
		_start_index->format(nx > 0x10 ? "%02X" : "%X");
		if (_start_index->value() >= nx) { _start_index->value(0); }
		_start_index->range(0, nx - 1);
	}
	else {
		_start_index->deactivate();
		_start_index_label->label("Start at Palette:");
		_start_index->value(0);
	}
}

void Image_To_Tiles_Dialog::update_ok_button() {
	if (_image_filename.empty() || _tileset_filename.empty()) {
		_ok_button->deactivate();
	}
	else {
		_ok_button->activate();
	}
}

void Image_To_Tiles_Dialog::update_color_zero_swatch() {
	Fl_Color c = _color_zero_swatch->active() ? fl_color_zero() : FL_BACKGROUND_COLOR;
	_color_zero_swatch->color(c);
	_color_zero_swatch->selection_color(c);
	_color_zero_swatch->redraw();
}

void Image_To_Tiles_Dialog::initialize_content() {
	// Populate content group
	_tileset_heading = new Label(0, 0, 0, 0, "Tileset");
	_tilemap_heading = new Label(0, 0, 0, 0, "Tilemap");
	_tileset_spacer = new Spacer(0, 0, 0, 0);
	_tilemap_spacer = new Spacer(0, 0, 0, 0);
	_palette_spacer = new Spacer(0, 0, 0, 0);
	_input_heading = new Label(0, 0, 0, 0, "Input:");
	_output_heading = new Label(0, 0, 0, 0, "Output:");
	_image = new Toolbar_Button(0, 0, 0, 0);
	_tileset = new Toolbar_Button(0, 0, 0, 0);
	_image_name = new Label_Button(0, 0, 0, 0, NO_FILE_SELECTED_LABEL);
	_tileset_name = new Label_Button(0, 0, 0, 0, NO_FILE_SELECTED_LABEL);
	_unique_tiles = new OS_Check_Button(0, 0, 0, 0, "Unique tiles");
	_flip_tiles = new OS_Check_Button(0, 0, 0, 0, "Flip tiles");
	_no_extra_blank_tiles = new OS_Check_Button(0, 0, 0, 0, "Avoid extra blank tiles at the end");
	_tilemap_name = new Label(0, 0, 0, 0, "Output: " NO_FILES_DETERMINED_LABEL);
	_format = new Dropdown(0, 0, 0, 0, "Format:");
	_start_id = new Default_Hex_Spinner(0, 0, 0, 0, "Start at ID: $");
	_use_blank = new OS_Check_Button(0, 0, 0, 0, "Blank tiles use ID: ");
	_blank_id = new Default_Hex_Spinner(0, 0, 0, 0, "$");
	_palette = new OS_Check_Button(0, 0, 0, 0, "Palette");
	_palette_name = new Label(0, 0, 0, 0, "Output: " NO_FILES_DETERMINED_LABEL);
	_palette_format = new Dropdown(0, 0, 0, 0, "Format:");
	_start_index_label = new Label(0, 0, 0, 0, "Start at Palette:");
	_start_index = new Default_Hex_Spinner(0, 0, 0, 0, "$");
	_color_zero = new OS_Check_Button(0, 0, 0, 0, "Color 0: ");
	_color_zero_rgb = new OS_Hex_Input(0, 0, 0, 0, "#");
	_color_zero_swatch = new Fl_Button(0, 0, 0, 0);
	_image_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	_tileset_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	// Initialize content group's children
	_input_heading->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	_output_heading->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	_image->callback((Fl_Callback *)image_cb, this);
	_image->image(INPUT_ICON);
	_tileset->callback((Fl_Callback *)tileset_cb, this);
	_tileset->image(OUTPUT_ICON);
	_image_name->callback((Fl_Callback *)image_cb, this);
	_tileset_name->callback((Fl_Callback *)tileset_cb, this);
	for (int i = 0; i < NUM_FORMATS; i++) {
		_format->add(format_name((Tilemap_Format)i));
	}
	_format->callback((Fl_Callback *)format_cb, this);
	_palette->value(1);
	_palette->callback((Fl_Callback *)palette_cb, this);
	for (int i = 0; i < NUM_PALETTE_FORMATS; i++) {
		_palette_format->add(palette_name((Palette_Format)i));
	}
	_unique_tiles->value(1);
	_unique_tiles->callback((Fl_Callback *)unique_tiles_cb, this);
	_flip_tiles->value(1);
	_palette_format->value(0);
	_palette_format->callback((Fl_Callback *)palette_format_cb, this);
	_color_zero->callback((Fl_Callback *)color_zero_cb, this);
	_color_zero_rgb->value("FF00FF");
	_color_zero_rgb->maximum_size(6);
	_color_zero_rgb->callback((Fl_Callback *)color_zero_rgb_cb, this);
	_color_zero_rgb->when(FL_WHEN_ENTER_KEY);
	_color_zero_swatch->box(OS_SWATCH_BOX);
	_color_zero_swatch->down_box(OS_SWATCH_BOX);
	_color_zero_swatch->callback((Fl_Callback *)color_zero_swatch_cb, this);
	_start_id->format("%03X");
	_start_id->range(0x000, MAX_NUM_TILES-1);
	_start_id->default_value(0x000);
	_use_blank->callback((Fl_Callback *)use_blank_cb, this);
	_blank_id->format("%03X");
	_blank_id->range(0x000, MAX_NUM_TILES-1);
	_blank_id->default_value(0x07F);
	_start_index->format("%X");
	_start_index->range(0x0, 0xFF);
	_start_index->default_value(0);
	_image_chooser->title("Read Image");
	_image_chooser->filter("Image Files\t*.{png,gif,bmp}\n");
	_tileset_chooser->title("Write Tileset");
	_tileset_chooser->filter("PNG Files\t*.png\nBMP Files\t*.bmp\n");
	_tileset_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);
}

int Image_To_Tiles_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4, grp_m = 6;
	int ch = (wgt_h + wgt_m) * 12 + grp_m * 2 + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	int wgt_w = text_width(_tileset_heading->label(), 4);
	int wgt_off = win_m;
	_tileset_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _tileset_heading->w();
	_tileset_spacer->resize(wgt_off, dy+wgt_h/2-1, ww-wgt_w, 2);
	dy += wgt_h + wgt_m;

	wgt_w = std::max(text_width(_input_heading->label(), 4), text_width(_output_heading->label(), 4));
	wgt_off = win_m;
	_input_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _input_heading->w();
	_image->resize(wgt_off, dy, wgt_h, wgt_h);
	wgt_off += _image->w();
	_image_name->resize(wgt_off, dy, ww-wgt_w-wgt_h, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m;
	_output_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _output_heading->w();
	_tileset->resize(wgt_off, dy, wgt_h, wgt_h);
	wgt_off += _tileset->w();
	_tileset_name->resize(wgt_off, dy, ww-wgt_w-wgt_h, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m;
	wgt_w = text_width(_unique_tiles->label(), 2) + wgt_h;
	_unique_tiles->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _unique_tiles->w() + win_m;
	wgt_w = text_width(_flip_tiles->label(), 2) + wgt_h;
	_flip_tiles->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m;
	_no_extra_blank_tiles->resize(wgt_off, dy, ww, wgt_h);
	dy += wgt_h + wgt_m + grp_m;

	wgt_w = text_width(_tilemap_heading->label(), 4);
	_tilemap_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _tilemap_heading->w();
	_tilemap_spacer->resize(wgt_off, dy+wgt_h/2-1, ww-wgt_w, 2);
	dy += wgt_h + wgt_m;

	_tilemap_name->resize(win_m, dy, ww, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m + text_width(_format->label(), 3);
	wgt_w = format_max_name_width() + wgt_h;
	_format->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_w = std::max(text_width("AAA", 2), text_width("FFF", 2)) + wgt_h / 2 + 4;
	wgt_off = win_m + text_width(_start_id->label(), 3);
	_start_id->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_w = text_width(_use_blank->label(), 3) + _use_blank->labelsize() + 4;
	wgt_off += _start_id->w() + win_m;
	_use_blank->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _use_blank->w() + text_width(_blank_id->label(), 1);
	wgt_w = std::max(text_width("AAA", 2), text_width("FFF", 2)) + wgt_h / 2 + 4;
	_blank_id->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m + grp_m;

	wgt_w = _palette->labelsize() + 4 + text_width(_palette->label(), 3);
	wgt_off = win_m;
	_palette->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _palette->w();
	_palette_spacer->resize(wgt_off, dy+wgt_h/2-1, ww-wgt_w, 2);
	dy += wgt_h + wgt_m;

	_palette_name->resize(win_m, dy, ww, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m + text_width(_palette_format->label(), 3);
	wgt_w = palette_max_name_width() + wgt_h;
	_palette_format->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_w = text_width("Start at Palette:", 3);
	wgt_off = win_m;
	_start_index_label->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_w = std::max(text_width("AA", 2), text_width("FF", 2)) + wgt_h / 2 + 4;
	wgt_off += text_width("Start at Palette: $", 3);
	_start_index->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_w = _color_zero->labelsize() + 4 + text_width(_color_zero->label());
	wgt_off += _start_index->w() + win_m;
	_color_zero->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _color_zero->w() + 4 + text_width(_color_zero_rgb->label(), 1);
	wgt_w = std::max(text_width("AAAAAA", 2), text_width("FFFFFF", 2));
	_color_zero_rgb->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _color_zero_rgb->w() + wgt_m;
	_color_zero_swatch->resize(wgt_off, dy, wgt_h, wgt_h);

	if (!_prepared_image) {
		_image_filename.clear();
	}
	_prepared_image = false;
	_picked_palette = false;
	_tileset_filename.clear();
	update_image_name();
	update_flip_tiles();
	update_start_index();
	update_output_names();
	update_ok_button();

	_palette->do_callback();
	_color_zero->do_callback();
	_use_blank->do_callback();

	return ch;
}

void Image_To_Tiles_Dialog::image_cb(Fl_Widget *, Image_To_Tiles_Dialog *itd) {
	int status = itd->_image_chooser->show();
	if (status == 1) {
		itd->_image_filename.clear();
		itd->_image_name->label(NO_FILE_SELECTED_LABEL);
	}
	else {
		const char *filename = itd->_image_chooser->filename();
		itd->_image_filename = filename;
		const char *basename = fl_filename_name(filename);
		itd->_image_name->copy_label(basename);
	}
	itd->update_ok_button();
	itd->_dialog->redraw();
}

void Image_To_Tiles_Dialog::tileset_cb(Fl_Widget *, Image_To_Tiles_Dialog *itd) {
	int status = itd->_tileset_chooser->show();
	if (status == 1) {
		itd->_tileset_filename.clear();
	}
	else {
		char filename[FL_PATH_MAX] = {};
		const char *default_ext = itd->_tileset_chooser->filter_value() == 1 ? ".bmp" : ".png";
		add_dot_ext(itd->_tileset_chooser->filename(), default_ext, filename);
		itd->_tileset_filename.assign(filename);
	}
	itd->update_output_names();
	itd->update_ok_button();
	itd->_dialog->redraw();
}

void Image_To_Tiles_Dialog::unique_tiles_cb(OS_Check_Button *, Image_To_Tiles_Dialog *itd) {
	itd->update_flip_tiles();
}

void Image_To_Tiles_Dialog::format_cb(Dropdown *, Image_To_Tiles_Dialog *itd) {
	if (!itd->_picked_palette) {
		itd->_palette_format->value((int)(format_uses_rgb_asm_pal(itd->format()) ? Palette_Format::RGB : Palette_Format::INDEXED));
	}
	itd->update_flip_tiles();
	itd->update_start_index();
	itd->update_output_names();
	itd->_dialog->redraw();
}

void Image_To_Tiles_Dialog::palette_cb(OS_Check_Button *, Image_To_Tiles_Dialog *itd) {
	if (itd->palette()) {
		itd->_palette_format->activate();
		itd->_palette_name->activate();
		itd->_start_index_label->activate();
		itd->_start_index->activate();
		itd->_color_zero->activate();
	}
	else {
		itd->_palette_format->deactivate();
		itd->_palette_name->deactivate();
		itd->_start_index_label->deactivate();
		itd->_start_index->deactivate();
		itd->_color_zero->deactivate();
		itd->_color_zero->clear();
	}
	itd->_palette_format->redraw();
	itd->_palette_name->redraw();
	itd->_start_index_label->redraw();
	itd->_start_index->redraw();
	itd->_color_zero->redraw();
	itd->_color_zero->do_callback();
}

void Image_To_Tiles_Dialog::palette_format_cb(Dropdown *, Image_To_Tiles_Dialog *itd) {
	itd->_picked_palette = true;
	itd->update_output_names();
}

void Image_To_Tiles_Dialog::color_zero_cb(OS_Check_Button *, Image_To_Tiles_Dialog *itd) {
	if (itd->color_zero()) {
		itd->_color_zero_rgb->activate();
		itd->_color_zero_swatch->activate();
	}
	else {
		itd->_color_zero_rgb->deactivate();
		itd->_color_zero_swatch->deactivate();
	}
	itd->_color_zero_rgb->do_callback();
	itd->_color_zero_rgb->redraw();
	itd->_color_zero_swatch->redraw();
}

void Image_To_Tiles_Dialog::color_zero_rgb_cb(OS_Hex_Input *, Image_To_Tiles_Dialog *itd) {
	if (size_t n = strlen(itd->_color_zero_rgb->value()); n < 6) {
		char buffer[7] = {};
		memset(buffer, '0', 6 - n);
		strcat(buffer, itd->_color_zero_rgb->value());
		itd->_color_zero_rgb->value(buffer);
	}
	itd->update_color_zero_swatch();
}

#ifdef _WIN32
static DWORD fl_to_win_color(Fl_Color c) {
	uchar r, g, b;
	Fl::get_color(c, r, g, b);
	return RGB(r, g, b);
}
#endif

void Image_To_Tiles_Dialog::color_zero_swatch_cb(Fl_Button *, Image_To_Tiles_Dialog *itd) {
#ifdef _WIN32
	static COLORREF customColors[16] = {};

	CHOOSECOLOR cc = {};
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = fl_xid(itd->_dialog);
	cc.lpCustColors = (LPDWORD)customColors;
	cc.rgbResult = fl_to_win_color(itd->fl_color_zero());
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc)) {
		uchar r = GetRValue(cc.rgbResult), g = GetGValue(cc.rgbResult), b = GetBValue(cc.rgbResult);
		char buffer[10] = {};
		sprintf(buffer, "%02X%02X%02X", r, g, b);
		itd->_color_zero_rgb->value(buffer);
		itd->update_color_zero_swatch();
	}
#else
	itd->update_color_zero_swatch();
#endif
}

void Image_To_Tiles_Dialog::use_blank_cb(OS_Check_Button *, Image_To_Tiles_Dialog *itd) {
	if (itd->use_blank()) {
		itd->_blank_id->activate();
	}
	else {
		itd->_blank_id->deactivate();
	}
	itd->_blank_id->redraw();
}
