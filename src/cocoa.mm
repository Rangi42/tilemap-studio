#import <Cocoa/Cocoa.h>

#pragma warning(push, 0)
#include <FL/x.H>
#pragma warning(pop)

#include "cocoa.h"

void cocoa_set_window_transparency(const Fl_Window *w, double alpha) {
	[fl_xid(w) setAlphaValue:alpha];
}

bool cocoa_is_maximized(const Fl_Window *w) {
	return [fl_xid(w) isZoomed];
}

void cocoa_maximize(const Fl_Window *w) {
	[fl_xid(w) setFrame:[[fl_xid(w) screen] frame] display:true];
}

bool cocoa_is_fullscreen(const Fl_Window *w) {
	return ([fl_xid(w) styleMask] & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen;
}

void cocoa_fullscreen(const Fl_Window *w, bool state) {
	if (state != cocoa_is_fullscreen(w)) {
		[fl_xid(w) toggleFullScreen:nil];
	}
}

bool cocoa_is_dark_mode() {
	NSString *dark = @"Dark";
	NSString *osxMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
	return [osxMode isEqualToString:dark];
}
