#ifndef COCOA_H
#define COCOA_H

#pragma warning(push, 0)
#include <FL/Fl_Window.H>
#pragma warning(pop)

void cocoa_set_window_transparency(const Fl_Window *w, double alpha);
void cocoa_set_appearance(const Fl_Window *w, bool dark);
bool cocoa_is_dark_mode();

#endif
