#pragma warning(push, 0)
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include <FL/fl_utf8.h>
#pragma warning(pop)

#include "version.h"
#include "preferences.h"
#include "utils.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

Fl_Preferences *Preferences::_preferences = NULL;

static void get_program_file_dir(char *path, size_t n, const char *argv0) {
	char name[FL_PATH_MAX] = {};
#ifdef _WIN32
	wchar_t buffer[MAX_PATH] = {};
	if (GetModuleFileName(NULL, buffer, _countof(buffer))) {
		fl_utf8fromwc(name, FL_PATH_MAX, buffer, MAX_PATH);
	}
	else {
		strncpy(name, argv0, FL_PATH_MAX);
	}
#else
	if (readlink("/proc/self/exe", name, FL_PATH_MAX-1) == -1) {
		strncpy(name, argv0, FL_PATH_MAX);
	}
#endif
	name[FL_PATH_MAX-1] = '\0';
	fl_filename_absolute(path, (int)n, name);
	char *pivot = strrchr(path, *DIR_SEP);
	if (pivot) {
		*pivot = '\0';
	}
	else {
		strcpy(path, ".");
	}
}

void Preferences::initialize(const char *argv0) {
	// Get the directory of tilemapstudio.exe
	char dirname[FL_PATH_MAX] = {};
	get_program_file_dir(dirname, _countof(dirname), argv0);

	char prefs[FL_PATH_MAX] = {};

	// Use tilemapstudio.prefs if it exists
	strcpy(prefs, dirname);
	strcat(prefs, DIR_SEP PROGRAM_EXE_NAME PREFS_EXT);
	if (file_exists(prefs)) {
		_preferences = new Fl_Preferences(dirname, PROGRAM_AUTHOR, PROGRAM_EXE_NAME);
		return;
	}

	// Use Tilemap Studio.prefs if it exists
	strcpy(prefs, dirname);
	strcat(prefs, DIR_SEP PROGRAM_NAME PREFS_EXT);
	if (file_exists(prefs)) {
		_preferences = new Fl_Preferences(dirname, PROGRAM_AUTHOR, PROGRAM_NAME);
		return;
	}

	// Use the user's FLTK preferences
	_preferences = new Fl_Preferences(Fl_Preferences::USER, PROGRAM_AUTHOR, PROGRAM_NAME);
}

void Preferences::close() {
	_preferences->flush();
	delete _preferences;
	_preferences = NULL;
}

int Preferences::get(const char *key, int default_) {
	int value;
	_preferences->get(key, value, default_);
	return value;
}

void Preferences::set(const char *key, int value) {
	_preferences->set(key, value);
}

std::string Preferences::get_string(const char *key) {
	char *value;
	_preferences->get(key, value, "");
	std::string s(value ? value : "");
	free(value);
	return s;
}

void Preferences::set_string(const char *key, const std::string &value) {
	_preferences->set(key, value.c_str());
}
