#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <string>

#pragma warning(push, 0)
#include <FL/Fl_Preferences.H>
#pragma warning(pop)

class Preferences {
public:
	static int get(const char *key, int default_ = 0);
	static void set(const char *key, int value);
	static std::string get_string(const char *key);
	static void set_string(const char *key, std::string &value);
};

#endif
