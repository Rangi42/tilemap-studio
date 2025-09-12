#import <Cocoa/Cocoa.h>

#pragma warning(push, 0)
#include <FL/x.H>
#pragma warning(pop)

#include "cocoa.h"

void cocoa_set_window_transparency(const Fl_Window *w, double alpha) {
	[fl_xid(w) setAlphaValue:alpha];
}

void cocoa_set_appearance(const Fl_Window *w, bool dark) {
	NSAppearance *appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
#ifdef MAC_OS_X_VERSION_10_14
	if (@available(macOS 10.14, *)) {
		if (dark) {
			appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
		}
	}
#endif
	[fl_xid(w) setAppearance: appearance];
}

bool cocoa_is_dark_mode() {
	return [[[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"] isEqualToString:@"Dark"];
}
