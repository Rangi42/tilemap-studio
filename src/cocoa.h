#ifndef COCOA_H
#define COCOA_H

// From $SDKROOT/System/Library/Frameworks/AppKit.framework/Versions/C/Headers/NSText.h
#ifdef __OBJC__
#import <AppKit/NSText.h>
#else
enum {
    NSEnterCharacter                = 0x0003,
    NSBackspaceCharacter            = 0x0008,
    NSTabCharacter                  = 0x0009,
    NSNewlineCharacter              = 0x000a,
    NSFormFeedCharacter             = 0x000c,
    NSCarriageReturnCharacter       = 0x000d,
    NSBackTabCharacter              = 0x0019,
    NSDeleteCharacter               = 0x007f,
    NSLineSeparatorCharacter        = 0x2028,
    NSParagraphSeparatorCharacter   = 0x2029
};
#endif

#pragma warning(push, 0)
#include <FL/Fl_Window.H>
#pragma warning(pop)

enum cocoa_appearance {
	COCOA_APPEARANCE_AQUA,
	COCOA_APPEARANCE_DARK_AQUA
};

void cocoa_set_window_transparency(const Fl_Window *w, double alpha);
bool cocoa_is_maximized(const Fl_Window *w);
void cocoa_maximize(const Fl_Window *w);
bool cocoa_is_fullscreen(const Fl_Window *w);
void cocoa_fullscreen(const Fl_Window *w, bool state);
void cocoa_set_appearance(const Fl_Window *w, enum cocoa_appearance appearance_id);
bool cocoa_is_dark_mode();

#endif
