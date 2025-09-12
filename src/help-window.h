#ifndef HELP_WINDOW_H
#define HELP_WINDOW_H

#include <string>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#pragma warning(pop)

#include "widgets.h"

class Help_Window {
private:
	int _dx, _dy, _width, _height;
	const char *_title, *_content;
	Fl_Double_Window *_window;
	HTML_View *_body;
	Default_Button *_ok_button;
	Fl_Box *_spacer;
public:
	Help_Window(int x, int y, int w, int h, const char *t = NULL);
	~Help_Window();
	inline void title(const char *t) { _title = t; }
	inline void content(const char *c) { _content = c; }
private:
	void initialize(void);
	void refresh(void);
public:
	void show(const Fl_Widget *p);
	void redraw(void);
private:
	static void close_cb(Fl_Widget *w, Help_Window *hw);
};

#endif
