#ifndef COCOA_H
#define COCOA_H

#pragma warning(push, 0)
#include <FL/Fl_Window.H>
#pragma warning(pop)

void cocoa_set_window_transparency(const Fl_Window *w, double alpha);
bool cocoa_is_maximized(const Fl_Window *w);
void cocoa_maximize(const Fl_Window *w);
bool cocoa_is_fullscreen(const Fl_Window *w);
void cocoa_fullscreen(const Fl_Window *w, bool state);
bool cocoa_is_dark_mode();

#endif
