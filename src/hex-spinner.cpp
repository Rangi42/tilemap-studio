#include <cctype>
#include <string>

#pragma warning(push, 0)
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#pragma warning(pop)

#include "hex-spinner.h"
#include "utils.h"

Hex_Input::Hex_Input(int x, int y, int w, int h, const char *l) : Fl_Int_Input(x, y, w, h, l) {}

int Hex_Input::handle_paste_text() {
	// Based on Fl_Input_::handletext() for FL_PASTE case
	if (readonly()) {
		fl_beep(FL_BEEP_ERROR);
		return 1;
	}
	if (!Fl::event_text() || !Fl::event_length()) {
		return 1;
	}
	const char *t = Fl::event_text();
	const char *e = t + Fl::event_length();
	while (e > t && isspace(*(e - 1) & 0xFF)) { e--; }
	if (e <= t) {
		return 1;
	}
	while (isspace(*t & 0xFF) && t < e) { t++; }
	const char *p = t;
	if (*p == '+' || *p == '-') { p++; }
	while (isxdigit((unsigned char)(*p) & 0xFF) && p < e) { p++; }
	if (p < e) {
		fl_beep(FL_BEEP_ERROR);
		return 1;
	}
	return replace(0, size(), t, (int)(e - t));
}

int Hex_Input::handle_key() {
	// Based on Fl_Input::handle_key() for FL_INT_INPUT type
	int del;
	if (!Fl::compose(del)) {
		return 0;
	}
	Fl::compose_reset();
	char a = Fl::event_text()[0];
	int ip = std::min(position(), mark());
	if (isxdigit((unsigned char)a) || (!ip && (a == '+' || a == '-'))) {
		if (readonly()) {
			fl_beep();
		}
		else {
			replace(position(), mark(), &a, 1);
		}
	}
	return 1;
}

int Hex_Input::handle(int event) {
	// Based on Fl_Input::handle()
	if (event == FL_PASTE) {
		return handle_paste_text();
	}
	if (event == FL_KEYBOARD) {
		if (active_r() && window() && this == Fl::belowmouse()) {
			window()->cursor(FL_CURSOR_NONE);
		}
		if (int v = handle_key()) {
			return v;
		}
	}
	return Fl_Input::handle(event);
}

Hex_Spinner::Hex_Spinner(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l),
	_value(1), _minimum(0x00), _maximum(0xFF), _step(1), _format("%02X"), _input(x, y, w - h / 2 - 2, h),
	_up_button(x + w - h / 2 - 2, y, h / 2 + 2, h / 2, "@-42<"),
	_down_button(x + w - h / 2 - 2, y + h - h / 2, h / 2 + 2, h / 2, "@-42>") {
	end();
	align(FL_ALIGN_LEFT);
	_input.value("01");
	_input.when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
	_input.callback((Fl_Callback *)input_cb, this);
	_up_button.callback((Fl_Callback *)up_button_cb, this);
	_down_button.callback((Fl_Callback *)down_button_cb, this);
}

void Hex_Spinner::resize(int x, int y, int w, int h) {
	Fl_Group::resize(x, y, w, h);
	_input.resize(x, y, w - h / 2 - 2, h);
	_up_button.resize(x + w - h / 2 - 2, y, h / 2 + 2, h / 2);
	_down_button.resize(x + w - h / 2 - 2, y + h - h / 2, h / 2 + 2, h / 2);
}

int Hex_Spinner::handle(int event) {
	switch (event) {
	case FL_KEYDOWN:
	case FL_SHORTCUT:
		if (Fl::event_key() == FL_Up) {
			_up_button.do_callback();
			return 1;
		}
		else if (Fl::event_key() == FL_Down) {
			_down_button.do_callback();
			return 1;
		}
		return 0;
	case FL_FOCUS:
		return _input.take_focus() ? 1 : 0;
	}
	return Fl_Group::handle(event);
}

void Hex_Spinner::update() {
	char s[255] = {};
	sprintf(s, _format, _value);
	_input.value(s);
}

void Hex_Spinner::input_cb(Hex_Input *, Hex_Spinner *hs) {
	int v = strtol(hs->_input.value(), NULL, 16);
	if (v < hs->_minimum) {
		hs->_value = hs->_minimum;
		hs->update();
	}
	else if (v > hs->_maximum) {
		hs->_value = hs->_maximum;
		hs->update();
	}
	else {
		hs->_value = v;
	}
	hs->set_changed();
	hs->do_callback();
}

void Hex_Spinner::up_button_cb(Fl_Repeat_Button *, Hex_Spinner *hs) {
	int v = hs->_value + hs->_step;
	if (v > hs->_maximum) { v = hs->_minimum; }
	hs->_value = v;
	hs->update();
	hs->set_changed();
	hs->do_callback();
}

void Hex_Spinner::down_button_cb(Fl_Repeat_Button *, Hex_Spinner *hs) {
	int v = hs->_value - hs->_step;
	if (v < hs->_minimum) { v = hs->_maximum; }
	hs->_value = v;
	hs->update();
	hs->set_changed();
	hs->do_callback();
}
