#ifndef HEX_SPINNER_H
#define HEX_SPINNER_H

#pragma warning(push, 0)
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Repeat_Button.H>
#pragma warning(pop)

class Hex_Input : public Fl_Int_Input {
public:
	Hex_Input(int x, int y, int w, int h, const char *l = NULL);
	int handle(int event);
private:
	int handle_key(void);
	int handle_paste_text(void);
};

class Hex_Spinner : public Fl_Group {
	// Based on Fl_Spinner
private:
	int _value, _minimum, _maximum, _step;
	const char *_format;
protected:
	Hex_Input _input;
	Fl_Repeat_Button _up_button, _down_button;
public:
	Hex_Spinner(int x, int y, int w, int h, const char *l = NULL);
	inline int value(void) const { return _value; }
	inline void value(int v) { _value = v; update(); }
	inline int maximum(void) const { return _maximum; }
	inline void maximum(int m) { _maximum = m; }
	inline int minimum(void) const { return _minimum; }
	inline void minimum(int m) { _minimum = m; }
	inline void range(int a, int b) { _minimum = a; _maximum = b; }
	inline int step(void) const { return _step; }
	inline void step(int s) { _step = s; update(); }
	inline const char *format(void) const { return _format; }
	inline void format(const char *f) { _format = f; update(); }
	inline Fl_Color textcolor(void) const { return _input.textcolor(); }
	inline void textcolor(Fl_Color c) { _input.textcolor(c); }
	inline Fl_Font textfont(void) const { return _input.textfont(); }
	inline void textfont(Fl_Font f) { _input.textfont(f); }
	inline Fl_Fontsize textsize(void) const { return _input.textsize(); }
	inline void textsize(Fl_Fontsize s) { _input.textsize(s); }
	inline Fl_Color color(void) const { return _input.color(); }
	inline void color(Fl_Color c) { _input.color(c); }
	inline Fl_Color selection_color(void) const { return _input.selection_color(); }
	inline void selection_color(Fl_Color c) { _input.selection_color(c); }
	void resize(int x, int y, int w, int h);
	int handle(int event);
private:
	void update(void);
	static void input_cb(Hex_Input *w, Hex_Spinner *hs);
	static void up_button_cb(Fl_Repeat_Button *w, Hex_Spinner *hs);
	static void down_button_cb(Fl_Repeat_Button *w, Hex_Spinner *hs);
};

#endif
