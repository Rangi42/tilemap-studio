#include <string>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "themes.h"
#include "widgets.h"
#include "modal-dialog.h"

#include "warning.xpm"
#include "error.xpm"
#include "success.xpm"
#include "app.xpm"

Fl_Pixmap Modal_Dialog::SUCCESS_SHIELD_ICON(SUCCESS_XPM);
Fl_Pixmap Modal_Dialog::WARNING_SHIELD_ICON(WARNING_XPM);
Fl_Pixmap Modal_Dialog::ERROR_SHIELD_ICON(ERROR_XPM);
Fl_Pixmap Modal_Dialog::PROGRAM_ICON(APP_XPM);

Modal_Dialog::Modal_Dialog(Fl_Window *top, const char *t, Icon c, bool cancel) : _icon_type(c),
	_title(t), _subject(), _message(), _min_w(0), _max_w(1000), _canceled(cancel),
	_top_window(top), _dialog(NULL), _icon(NULL), _heading(NULL), _body(NULL), _ok_button(NULL), _cancel_button(NULL) {}

Modal_Dialog::~Modal_Dialog() {
	_top_window = NULL;
	delete _dialog;
}

void Modal_Dialog::initialize() {
	if (_dialog) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate dialog
	_dialog = new Fl_Double_Window(0, 0, 0, 0, _title.c_str());
	_icon = new Fl_Box(0, 0, 0, 0);
	_heading = new Label(0, 0, 0, 0, _subject.c_str());
	_body = new Label(0, 0, 0, 0);
	_ok_button = new Default_Button(0, 0, 0, 0, "OK");
	_cancel_button = _canceled ? new OS_Button(0, 0, 0, 0, "Cancel") : NULL;
	_canceled = false;
	_dialog->end();
	// Initialize dialog
	_dialog->box(OS_BG_BOX);
	_dialog->resizable(NULL);
	_dialog->callback((Fl_Callback *)close_cb, this);
	_dialog->set_modal();
	// Initialize dialog's children
	_icon->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	_heading->labelsize(OS_FONT_SIZE + 4);
	_heading->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	_body->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	_ok_button->tooltip("OK (Enter)");
	_ok_button->callback((Fl_Callback *)close_cb, this);
	if (_cancel_button) {
		_cancel_button->shortcut(FL_Escape);
		_cancel_button->tooltip("Cancel (Esc)");
		_cancel_button->callback((Fl_Callback *)cancel_cb, this);
	}
	Fl_Group::current(prev_current);
}

void Modal_Dialog::refresh() {
	_canceled = false;
	// Refresh widget labels
	_heading->label(_subject.c_str());
	_dialog->label(_title.c_str());
	_body->label(_message.c_str());
	// Refresh icon
	switch (_icon_type) {
	case Icon::NO_ICON:
		_icon->image(NULL);
		break;
	case Icon::SUCCESS_ICON:
		_icon->image(SUCCESS_SHIELD_ICON);
		break;
	case Icon::WARNING_ICON:
		_icon->image(WARNING_SHIELD_ICON);
		break;
	case Icon::ERROR_ICON:
		_icon->image(ERROR_SHIELD_ICON);
		break;
	case Icon::APP_ICON:
		_icon->image(PROGRAM_ICON);
		break;
	}
	// Refresh widget positions and sizes
	int bwd = (_icon_type == Icon::NO_ICON ? 0 : 60) + 20;
	fl_font(_heading->labelfont(), _heading->labelsize());
	int hw = _max_w - bwd, hh = 0;
	fl_measure(_heading->label(), hw, hh);
	fl_font(_body->labelfont(), _body->labelsize());
	int bw = _max_w - bwd, bh = 0;
	fl_measure(_body->label(), bw, bh);
	int w = std::max(std::max(bw, hw) + bwd + OS_FONT_SIZE, _min_w), h = 10;
	int ww = w - 20;
	int heading_h = 25;
	int btn_w = 80, btn_h = 22;
	if (_icon_type == Icon::NO_ICON) {
		_icon->resize(0, 0, 0, 0);
		if (_subject.empty()) {
			_heading->resize(0, 0, 0, 0);
		}
		else {
			_heading->resize(10, h, ww, heading_h);
			h += _heading->h() + 10;
		}
		_body->resize(10, h, ww, bh);
		h += _body->h() + 10;
	}
	else {
		_icon->resize(10, h, 50, 50);
		if (_subject.empty()) {
			_heading->resize(0, 0, 0, 0);
		}
		else {
			_heading->resize(70, h, ww-60, heading_h);
			h += _heading->h() + 10;
		}
		_body->resize(70, h, ww-60, bh);
		h += _body->h() + 10;
		h = std::max(h, 70);
	}
#ifdef _WIN32
	if (_cancel_button) {
		_ok_button->resize(w-btn_w-14-btn_w-10, h, btn_w, btn_h);
		_cancel_button->resize(w-btn_w-10, h, btn_w, btn_h);
	}
	else {
		_ok_button->resize(w-btn_w-10, h, btn_w, btn_h);
	}
#else
	if (_cancel_button) {
		_cancel_button->resize(w-btn_w-14-btn_w-10, h, btn_w, btn_h);
	}
	_ok_button->resize(w-btn_w-10, h, btn_w, btn_h);
#endif
	h += _ok_button->h() + 10;
	_dialog->size_range(w, h, w, h);
	_dialog->size(w, h);
	_dialog->redraw();
}

void Modal_Dialog::show(const Fl_Widget *p) {
	initialize();
	refresh();
	int x = p->x() + (p->w() - _dialog->w()) / 2;
	int y = p->y() + (p->h() - _dialog->h()) / 2;
	_dialog->position(x, y);
	_dialog->show();
#ifdef _WIN32
	// Flash taskbar button
	// <http://blogs.msdn.com/b/oldnewthing/archive/2008/05/12/8490184.aspx>
	HWND top_hwnd = fl_xid(_top_window);
	HWND fgw = GetForegroundWindow();
	if (fgw != top_hwnd && fgw != fl_xid(_dialog)) {
		FLASHWINFO fwi;
		fwi.cbSize = sizeof(fwi);
		fwi.hwnd = top_hwnd;
		fwi.dwFlags = FLASHW_ALL;
		fwi.dwTimeout = 0;
		fwi.uCount = 3;
		FlashWindowEx(&fwi);
	}
#endif
	while (_dialog->shown()) { Fl::wait(); }
}

void Modal_Dialog::close_cb(Fl_Widget *, Modal_Dialog *md) {
	md->_dialog->hide();
}

void Modal_Dialog::cancel_cb(Fl_Widget *w, Modal_Dialog *md) {
	md->_canceled = true;
	close_cb(w, md);
}
