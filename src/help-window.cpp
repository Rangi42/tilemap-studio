#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Widget.H>
#pragma warning(pop)

#include "themes.h"
#include "widgets.h"
#include "help-window.h"

Help_Window::Help_Window(int x, int y, int w, int h, const char *t) : _dx(x), _dy(y), _width(w), _height(h), _title(t),
	_content(NULL), _window(NULL), _body(NULL), _ok_button(NULL), _spacer(NULL) {}

Help_Window::~Help_Window() {
	delete _window;
	delete _body;
	delete _ok_button;
	delete _spacer;
}

void Help_Window::initialize() {
	if (_window) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate window
	_window = new Fl_Double_Window(_dx, _dy, _width, _height, _title);
	_body = new HTML_View(10, 10, _width-20, _height-52);
	_ok_button = new Default_Button(_width-90, _height-32, 80, 22, "OK");
	_spacer = new Fl_Box(10, 10, _width-110, _height-52);
	_window->end();
	// Initialize window
	_window->box(OS_BG_BOX);
	_window->resizable(_spacer);
	_window->callback((Fl_Callback *)close_cb, this);
	// Initialize window's children
	_ok_button->tooltip("OK (Enter)");
	_ok_button->callback((Fl_Callback *)close_cb, this);
	Fl_Group::current(prev_current);
}

void Help_Window::refresh() {
	_window->label(_title ? _title : "Help");
	_body->value(_content ? _content : "");
}

void Help_Window::show(const Fl_Widget *p) {
	initialize();
	refresh();
	Fl_Window *prev_grab = Fl::grab();
	_window->position(p->x() + _dx, p->y() + _dy);
	Fl::grab(NULL);
	_window->show();
	Fl::grab(prev_grab);
}

void Help_Window::close_cb(Fl_Widget *, Help_Window *hw) {
	hw->_window->hide();
}
