#ifndef WIDGETS_H
#define WIDGETS_H

#include <string>

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Help_View.H>
#pragma warning(pop)

#include "hex-spinner.h"

#define OS_SUBMENU(l) {l, 0, NULL, NULL, FL_SUBMENU, FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR}
#define OS_MENU_ITEM(l, s, c, d, f) {l "    ", s, c, d, f, FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR}
#define OS_NULL_MENU_ITEM(s, c, d, f) {"", s, c, d, f, FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR}

class DnD_Receiver : public Fl_Box {
public:
	static void deferred_callback(DnD_Receiver *dndr);
private:
	std::string _text;
public:
	DnD_Receiver(int x, int y, int w, int h, const char *l = NULL);
	inline const std::string &text(void) const { return _text; }
	int handle(int event);
};

class Label : public Fl_Box {
public:
	Label(int x, int y, int w, int h, const char *l = NULL);
};

class Spacer : public Fl_Box {
public:
	Spacer(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Input : public Fl_Input {
public:
	OS_Input(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Button : public Fl_Button {
public:
	OS_Button(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class Default_Button : public Fl_Button {
public:
	Default_Button(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class OS_Check_Button : public Fl_Check_Button {
public:
	OS_Check_Button(int x, int y, int w, int h, const char *l = NULL);
	void draw(void);
protected:
	int handle(int event);
};

class Label_Button : public OS_Button {
public:
	Label_Button(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class OS_Spinner : public Fl_Spinner {
public:
	OS_Spinner(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Hex_Spinner : public Hex_Spinner {
public:
	OS_Hex_Spinner(int x, int y, int w, int h, const char *l = NULL);
};

class Default_Spinner : public OS_Spinner {
private:
	double _default_value;
public:
	Default_Spinner(int x, int y, int w, int h, const char *l = NULL);
	inline double default_value(void) const { return _default_value; }
	inline void default_value(double v) { _default_value = v; value(_default_value); }
protected:
	int handle(int event);
};

class Default_Hex_Spinner : public OS_Hex_Spinner {
private:
	int _default_value;
public:
	Default_Hex_Spinner(int x, int y, int w, int h, const char *l = NULL);
	inline int default_value(void) const { return _default_value; }
	inline void default_value(int v) { _default_value = v; value(_default_value); }
protected:
	int handle(int event);
};

class HTML_View : public Fl_Help_View {
public:
	HTML_View(int x, int y, int w, int h, const char *l = NULL);
};

class Dropdown : public Fl_Choice {
public:
	Dropdown(int x, int y, int w, int h, const char *l = NULL);
	void draw(void);
};

class OS_Tabs : public Fl_Tabs {
public:
	OS_Tabs(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class OS_Tab : public Fl_Group {
public:
	OS_Tab(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Scroll : public Fl_Scroll {
public:
	OS_Scroll(int x, int y, int w, int h, const char *l = NULL);
};

class Droppable {
private:
	DnD_Receiver *_dnd_receiver;
public:
	Droppable();
	inline void dnd_receiver(DnD_Receiver *dndr) { _dnd_receiver = dndr; }
	int handle(int event);
};

class Workpane : public Fl_Group, public Droppable {
public:
	Workpane(int x, int y, int w, int h, const char *l = NULL);
	int handle(int event);
};

class Workspace : public OS_Scroll, public Droppable {
private:
	int _content_w, _content_h;
	int _ox, _oy, _cx, _cy;
public:
	Workspace(int x, int y, int w, int h, const char *l = NULL);
	inline void contents(int w, int h) { _content_w = w; _content_h = h; }
	int handle(int event);
};

class Toolbar : public Fl_Group {
private:
	Fl_Group _spacer;
public:
	Toolbar(int x, int y, int w, int h, const char *l = NULL);
	void resize(int x, int y, int w, int h);
	void draw(void);
	void end(void) { resize(x(), y(), w(), h()); }
};

class Toolbar_Button : public Fl_Button {
public:
	Toolbar_Button(int x, int y, int w, int h, const char *l = NULL);
protected:
	void draw(void);
	int handle(int event);
};

class Toolbar_Toggle_Button : public Toolbar_Button {
public:
	Toolbar_Toggle_Button(int x, int y, int w, int h, const char *l = NULL);
};

class Toolbar_Radio_Button : public Toolbar_Button {
public:
	Toolbar_Radio_Button(int x, int y, int w, int h, const char *l = NULL);
};

class Status_Bar_Field : public Fl_Box {
private:
	const char *_default_label;
public:
	Status_Bar_Field(int x, int y, int w, int h, const char *l = NULL);
	~Status_Bar_Field();
	inline const char *default_label(void) const { return _default_label; }
	inline void reset_label(void) { copy_label(_default_label); }
};

#endif
