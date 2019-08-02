#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#endif

#pragma warning(push, 0)
#include <FL/fl_draw.H>
#include <FL/filename.H>
#pragma warning(pop)

#include "utils.h"

const std::string whitespace(" \f\n\r\t\v");

bool starts_with(const std::string &s, const std::string &p) {
	return !s.compare(0, p.size(), p);
}

bool ends_with(const std::string &s, const std::string &p) {
	return s.size() >= p.size() && !s.compare(s.size() - p.size(), p.size(), p);
}

bool ends_with(const std::wstring &s, const std::wstring &p) {
	return s.size() >= p.size() && !s.compare(s.size() - p.size(), p.size(), p);
}

void trim(std::string &s, const std::string &t) {
	std::string::size_type p = s.find_first_not_of(t);
	s.erase(0, p);
	p = s.find_last_not_of(t);
	s.erase(p + 1);
}

void remove_comment(std::string &s, char c) {
	size_t p = s.find(c);
	if (p != std::string::npos) {
		s.erase(p);
	}
}

void remove_dot_ext(const char *f, char *s) {
	strcpy(s, fl_filename_name(f));
	char *dot = strchr(s, '.');
	if (dot) { *dot = '\0'; }
}

void add_dot_ext(const char *f, const char *ext, char *s) {
	strcpy(s, f);
	const char *e = fl_filename_ext(s);
	if (!e || !strlen(e)) {
		strcat(s, ext);
	}
}

int text_width(const char *l, int pad) {
	int lw = 0, lh = 0;
	fl_measure(l, lw, lh, 0);
	return lw + 2 * pad;
}

bool file_exists(const char *f) {
#ifdef _WIN32
	return !_access_s(f, 4);
#else
	return !access(f, 4);
#endif
}

size_t file_size(const char *f) {
#ifdef __CYGWIN__
#define stat64 stat
#elif defined(_WIN32)
#define stat64 _stat32i64
#endif
	struct stat64 s;
	int r = stat64(f, &s);
	return r ? 0 : (size_t)s.st_size;
}
