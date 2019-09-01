#pragma warning(push, 0)
#include <FL/Fl_Preferences.H>
#pragma warning(pop)

#include "version.h"
#include "preferences.h"

Fl_Preferences global_prefs(Fl_Preferences::USER, PROGRAM_AUTHOR, PROGRAM_NAME);

int Preferences::get(const char *key, int default_) {
	int value;
	global_prefs.get(key, value, default_);
	return value;
}

void Preferences::set(const char *key, int value) {
	global_prefs.set(key, value);
}

std::string Preferences::get_string(const char *key) {
	char *value;
	global_prefs.get(key, value, "");
	std::string s(value ? value : "");
	delete value;
	return s;
}

void Preferences::set_string(const char *key, std::string &value) {
	global_prefs.set(key, value.c_str());
}
