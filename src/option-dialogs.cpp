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
#include "config.h"
#include "icons.h"

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

void Option_Dialog::show(const Fl_Widget *p) {
	initialize();
	refresh();
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

New_Tilemap_Dialog::New_Tilemap_Dialog(const char *t) : Option_Dialog(194, t), _tilemap_width(NULL), _tilemap_height(NULL) {}

New_Tilemap_Dialog::~New_Tilemap_Dialog() {
	delete _tilemap_width;
	delete _tilemap_height;
}

void New_Tilemap_Dialog::initialize_content() {
	// Populate content group
	_tilemap_width = new OS_Spinner(0, 0, 0, 0, "Width:");
	_tilemap_height = new OS_Spinner(0, 0, 0, 0, "Height:");
	// Initialize content group's children
	_tilemap_width->align(FL_ALIGN_LEFT);
	_tilemap_width->range(1, 999);
	_tilemap_height->align(FL_ALIGN_LEFT);
	_tilemap_height->range(1, 9999);
}

int New_Tilemap_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = wgt_h + wgt_m + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	int wgt_off = win_m + MAX(text_width(_tilemap_width->label(), 2), text_width(_tilemap_height->label(), 2));
	int wgt_w = text_width("999", 2) + wgt_h;
	_tilemap_width->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;
	_tilemap_height->resize(wgt_off, dy, wgt_w, wgt_h);

	return ch;
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
	for (int i = 0; i < 9; i++) { delete _anchor_buttons[i]; }
}

Resize_Dialog::Hor_Align Resize_Dialog::horizontal_anchor() const {
	if (_anchor_buttons[0]->value() || _anchor_buttons[3]->value() || _anchor_buttons[6]->value()) { return LEFT; }
	if (_anchor_buttons[2]->value() || _anchor_buttons[5]->value() || _anchor_buttons[8]->value()) { return RIGHT; }
	return CENTER;
}

Resize_Dialog::Vert_Align Resize_Dialog::vertical_anchor() const {
	if (_anchor_buttons[0]->value() || _anchor_buttons[1]->value() || _anchor_buttons[2]->value()) { return TOP; }
	if (_anchor_buttons[6]->value() || _anchor_buttons[7]->value() || _anchor_buttons[8]->value()) { return BOTTOM; }
	return MIDDLE;
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
	_tilemap_width = new OS_Spinner(0, 0, 0, 0, "Width:");
	_tilemap_height = new OS_Spinner(0, 0, 0, 0, "Height:");
	for (int i = 0; i < 9; i++) {
		Anchor_Button *ab = new Anchor_Button(i);
		ab->callback((Fl_Callback *)anchor_button_cb, this);
		_anchor_buttons[i] = ab;
	}
	// Initialize content group's children
	_tilemap_width->align(FL_ALIGN_LEFT);
	_tilemap_width->range(1, 999);
	_tilemap_height->align(FL_ALIGN_LEFT);
	_tilemap_height->range(1, 9999);
	anchor(Preferences::get("resize-anchor", 4));
}

int Resize_Dialog::refresh_content(int ww, int dy) {
	int wgt_w = 0, wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = (wgt_h + wgt_m) * 2 + wgt_h;
	_content->resize(win_m, dy, ww, ch);

	int wgt_off = win_m + MAX(text_width(_tilemap_width->label(), 2), text_width(_tilemap_height->label(), 2));

	wgt_w = text_width("999", 2) + wgt_h;
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
	for (int i = 0; i < 9; i++) {
		rd->_anchor_buttons[i]->label(NULL);
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

Tilemap_Width_Dialog::Tilemap_Width_Dialog(const char *t) : Option_Dialog(194, t), _tilemap_width(NULL) {}

Tilemap_Width_Dialog::~Tilemap_Width_Dialog() {
	delete _tilemap_width;
}

void Tilemap_Width_Dialog::initialize_content() {
	// Populate content group
	_tilemap_width = new OS_Spinner(0, 0, 0, 0, "Width:");
	// Initialize content group's children
	_tilemap_width->align(FL_ALIGN_LEFT);
	_tilemap_width->range(1, 999);
}

int Tilemap_Width_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10;
	_content->resize(win_m, dy, ww, wgt_h);

	int wgt_off = win_m + text_width(_tilemap_width->label(), 2);
	int wgt_w = text_width("999", 2) + wgt_h;
	_tilemap_width->resize(wgt_off, dy, wgt_w, wgt_h);

	return wgt_h;
}

Image_To_Tiles_Dialog::Image_To_Tiles_Dialog(const char *t) : Option_Dialog(320, t), _image_heading(NULL),
	_tilemap_heading(NULL), _tileset_heading(NULL), _image(NULL), _tilemap(NULL), _tileset(NULL), _image_name(NULL),
	_tilemap_name(NULL), _tileset_name(NULL), _format(NULL), _start_id(NULL), _use_7f(NULL), _image_chooser(NULL),
	_tilemap_chooser(NULL), _tileset_chooser(NULL) {}

Image_To_Tiles_Dialog::~Image_To_Tiles_Dialog() {
	delete _image_heading;
	delete _tilemap_heading;
	delete _tileset_heading;
	delete _image;
	delete _tilemap;
	delete _tileset;
	delete _image_name;
	delete _tilemap_name;
	delete _tileset_name;
	delete _format;
	delete _start_id;
	delete _use_7f;
	delete _image_chooser;
	delete _tilemap_chooser;
	delete _tileset_chooser;
}

void Image_To_Tiles_Dialog::update_ok_button() {
	if (_image_filename.empty() || _tilemap_filename.empty() || _tileset_filename.empty()) {
		_ok_button->deactivate();
	}
	else {
		_ok_button->activate();
	}
}

void Image_To_Tiles_Dialog::initialize_content() {
	// Populate content group
	_image_heading = new Label(0, 0, 0, 0, "Image:");
	_tilemap_heading = new Label(0, 0, 0, 0, "Tilemap:");
	_tileset_heading = new Label(0, 0, 0, 0, "Tileset:");
	_image = new Toolbar_Button(0, 0, 0, 0);
	_tilemap = new Toolbar_Button(0, 0, 0, 0);
	_tileset = new Toolbar_Button(0, 0, 0, 0);
	_image_name = new Label_Button(0, 0, 0, 0, "No file selected");
	_tilemap_name = new Label_Button(0, 0, 0, 0, "No file selected");
	_tileset_name = new Label_Button(0, 0, 0, 0, "No file selected");
	_format = new Dropdown(0, 0, 0, 0, "Format:");
	_start_id = new OS_Hex_Spinner(0, 0, 0, 0, "Start at ID:");
	_use_7f = new OS_Check_Button(0, 0, 0, 0, "Use $7F for Blank Spaces");
	_image_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	_tilemap_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	_tileset_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	// Initialize content group's children
	_image->callback((Fl_Callback *)image_cb, this);
	_image->image(INPUT_ICON);
	_tilemap->callback((Fl_Callback *)tilemap_cb, this);
	_tilemap->image(OUTPUT_ICON);
	_tileset->callback((Fl_Callback *)tileset_cb, this);
	_tileset->image(OUTPUT_ICON);
	_image_name->callback((Fl_Callback *)image_cb, this);
	_tilemap_name->callback((Fl_Callback *)tilemap_cb, this);
	_tileset_name->callback((Fl_Callback *)tileset_cb, this);
	_format->add("Plain");                                // PLAIN
	_format->add("Run-length encoded (RLE)");             // RLE
	_format->add("GSC Town Map ($FF end)");               // FF_END
	_format->add("Pok\xc3\xa9gear card (RLE + $FF end)"); // RLE_FF_END
	_format->add("RBY Town Map (RLE nybbles + $00 end)"); // RLE_NYBBLES
	_format->add("PC Town Map (X\\/Y flip)");             // XY_FLIP
	_format->add("SGB border (tile + attribute)");        // TILE_ATTR
	_start_id->value(0x00);
	_image_chooser->title("Read Image");
	_image_chooser->filter("Image Files\t*.{png,bmp}\n");
	_tilemap_chooser->title("Write Tilemap");
	_tilemap_chooser->filter("Tilemap Files\t*.{tilemap,rle,bin,map}\n");
	_tilemap_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);
	_tileset_chooser->title("Write Tileset");
	_tileset_chooser->filter("PNG Files\t*.png\n");
	_tileset_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);
}

int Image_To_Tiles_Dialog::refresh_content(int ww, int dy) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int ch = (wgt_h + wgt_m) * 4 + wgt_h;
	_content->resize(win_m, dy, ww, wgt_h * 5 + wgt_m * 4);

	int wgt_w = MAX(MAX(text_width(_image_heading->label(), 4),
		text_width(_tilemap_heading->label(), 4)),
		text_width(_tileset_heading->label(), 4));
	int wgt_off = win_m;

	_image_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _image_heading->w();
	_image->resize(wgt_off, dy, wgt_h, wgt_h);
	wgt_off += _image->w();
	_image_name->resize(wgt_off, dy, ww-wgt_w-wgt_h, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m;
	_tilemap_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _tilemap_heading->w();
	_tilemap->resize(wgt_off, dy, wgt_h, wgt_h);
	wgt_off += _tilemap->w();
	_tilemap_name->resize(wgt_off, dy, ww-wgt_w-wgt_h, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m;
	_tileset_heading->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_off += _tileset_heading->w();
	_tileset->resize(wgt_off, dy, wgt_h, wgt_h);
	wgt_off += _tileset->w();
	_tileset_name->resize(wgt_off, dy, ww-wgt_w-wgt_h, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_off = win_m + text_width(_format->label(), 3);
	wgt_w = ww - wgt_off + win_m;
	_format->resize(wgt_off, dy, wgt_w, wgt_h);
	dy += wgt_h + wgt_m;

	wgt_w = MAX(text_width("AA", 2), text_width("FF", 2)) + wgt_h;
	wgt_off = win_m + text_width(_start_id->label(), 3);
	_start_id->resize(wgt_off, dy, wgt_w, wgt_h);
	wgt_w = text_width(_use_7f->label(), 3) + wgt_h;
	wgt_off += _start_id->w() + win_m;
	_use_7f->resize(wgt_off, dy, wgt_w, wgt_h);

	_image_filename.clear();
	_tilemap_filename.clear();
	_tileset_filename.clear();
	_image_name->label("No file selected");
	_tilemap_name->label("No file selected");
	_tileset_name->label("No file selected");
	_ok_button->deactivate();

	return ch;
}

void Image_To_Tiles_Dialog::image_cb(Fl_Widget *, Image_To_Tiles_Dialog *itd) {
	int status = itd->_image_chooser->show();
	if (status == 1) {
		itd->_image_filename.clear();
		itd->_image_name->label("No file selected");
	}
	else {
		const char *filename = itd->_image_chooser->filename();
		const char *basename = fl_filename_name(filename);
		itd->_image_filename = filename;
		itd->_image_name->copy_label(basename);
	}
	itd->update_ok_button();
	itd->_dialog->redraw();
}

void Image_To_Tiles_Dialog::tilemap_cb(Fl_Widget *, Image_To_Tiles_Dialog *itd) {
	int status = itd->_tilemap_chooser->show();
	if (status == 1) {
		itd->_tilemap_filename.clear();
		itd->_tilemap_name->label("No file selected");
	}
	else {
		const char *filename = itd->_tilemap_chooser->filename();
		const char *basename = fl_filename_name(filename);
		itd->_tilemap_filename = filename;
		itd->_tilemap_name->copy_label(basename);
	}
	itd->update_ok_button();
	itd->_dialog->redraw();
}

void Image_To_Tiles_Dialog::tileset_cb(Fl_Widget *, Image_To_Tiles_Dialog *itd) {
	int status = itd->_tileset_chooser->show();
	if (status == 1) {
		itd->_tileset_filename.clear();
		itd->_tileset_name->label("No file selected");
	}
	else {
		const char *filename = itd->_tileset_chooser->filename();
		const char *basename = fl_filename_name(filename);
		itd->_tileset_filename = filename;
		itd->_tileset_name->copy_label(basename);
	}
	itd->update_ok_button();
	itd->_dialog->redraw();
}
