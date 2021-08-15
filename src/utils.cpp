#include <cstring>
#include <cctype>
#include <algorithm>
#include <sys/stat.h>

#pragma warning(push, 0)
#include <FL/fl_draw.H>
#include <FL/filename.H>
#include <FL/fl_utf8.h>
#pragma warning(pop)

#include "utils.h"

static bool cmp_ignore_case(const char &a, const char &b) {
	return tolower(a) == tolower(b);
}

bool starts_with_ignore_case(std::string_view s, std::string_view p) {
	if (s.size() < p.size()) { return false; }
	std::string_view ss = s.substr(0, p.size());
	return std::equal(RANGE(ss), RANGE(p), cmp_ignore_case);
}

bool ends_with_ignore_case(std::string_view s, std::string_view p) {
	if (s.size() < p.size()) { return false; }
	std::string_view ss = s.substr(s.size() - p.size());
	return std::equal(RANGE(ss), RANGE(p), cmp_ignore_case);
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
	return !fl_access(f, 4); // R_OK
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

size_t file_size(FILE *f) {
#ifdef __CYGWIN__
#define fstat64 fstat
#elif defined(_WIN32)
#define fileno _fileno
#define fstat64 _fstat32i64
#endif
	struct stat64 s;
	int r = fstat64(fileno(f), &s);
	return r ? 0 : (size_t)s.st_size;
}
