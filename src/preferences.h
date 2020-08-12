#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <string>

#pragma warning(push, 0)
#include <FL/Fl_Preferences.H>
#pragma warning(pop)

#define PREFS_EXT ".prefs"

class Preferences {
private:
	static Fl_Preferences *_preferences;
public:
	static void initialize(const char *argv0);
	static void close(void);
	static int get(const char *key, int default_ = 0);
	static void set(const char *key, int value);
	static std::string get_string(const char *key);
	static void set_string(const char *key, const std::string &value);
};

#endif
