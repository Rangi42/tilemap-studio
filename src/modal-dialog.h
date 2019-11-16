#ifndef MODAL_DIALOG_H
#define MODAL_DIALOG_H

#include <string>

#pragma warning(push, 0)
#include <FL/Fl_Pixmap.H>
#pragma warning(pop)

#include "widgets.h"

class Modal_Dialog {
public:
	enum class Icon { NO_ICON, SUCCESS_ICON, WARNING_ICON, ERROR_ICON, APP_ICON };
	static Fl_Pixmap SUCCESS_SHIELD_ICON, WARNING_SHIELD_ICON, ERROR_SHIELD_ICON, PROGRAM_ICON;
private:
	Icon _icon_type;
	std::string _title, _subject, _message;
	int _min_w, _max_w;
	bool _canceled;
	Fl_Window *_top_window;
	Fl_Double_Window *_dialog;
	Fl_Box *_icon;
	Label *_heading;
	Label *_body;
	Default_Button *_ok_button;
	OS_Button *_cancel_button;
public:
	Modal_Dialog(Fl_Window *top, const char *t = NULL, Icon c = Icon::NO_ICON, bool cancel = false);
	~Modal_Dialog();
private:
	void initialize(void);
	void refresh(void);
public:
	inline void icon(Icon c) { _icon_type = c; }
	inline void title(const std::string &t) { _title = t; }
	inline void subject(const std::string &s) { _subject = s; }
	inline void message(const std::string &m) { _message = m; }
	inline void width_range(int min_w, int max_w) { _min_w = min_w; _max_w = max_w; }
	inline bool canceled(void) const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
	void show(const Fl_Widget *p);
private:
	static void close_cb(Fl_Widget *, Modal_Dialog *md);
	static void cancel_cb(Fl_Widget *, Modal_Dialog *md);
};

#endif
