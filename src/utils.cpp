#include <cstring>
#include <cctype>
#include <algorithm>
#include <vector>
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
	struct stat s;
	int r = fl_stat(f, &s);
	return r ? 0 : (size_t)s.st_size;
}

size_t file_size(FILE *f) {
#ifdef __CYGWIN__
#define stat64 stat
#define fstat64 fstat
#elif defined(_WIN32)
#define fileno _fileno
#define stat64 _stat32i64
#define fstat64 _fstat32i64
#endif
	struct stat64 s;
	int r = fstat64(fileno(f), &s);
	return r ? 0 : (size_t)s.st_size;
}

void open_ifstream(std::ifstream &ifs, const char *f) {
#ifdef _WIN32
	wchar_t wf[FL_PATH_MAX] = {};
	fl_utf8towc(f, strlen(f), wf, sizeof(wf));
	ifs.open(wf);
#else
	ifs.open(f);
#endif
}

bool check_read(FILE *file, uchar *expected, size_t n) {
	std::vector<uchar> buffer(n);
	size_t r = fread(buffer.data(), 1, n, file);
	return r == n && (!expected || !memcmp(buffer.data(), expected, n));
}

uint16_t read_uint16(FILE *file) {
	int lo = fgetc(file);
	int hi = fgetc(file);
	return (uint16_t)(((hi & 0xFF00) >> 8) | (lo & 0xFF));
}

size_t read_rmp_size(FILE *file) {
	// <https://github.com/chadaustin/sphere/blob/master/sphere/docs/internal/map.rmp.txt>
	uchar expected_header[21] = {
		'.', 'r', 'm', 'p', // magic number
		LE16(1),            // version
		0,                  // type (obsolete)
		1,                  // num layers
		0,                  // reserved
		LE16(0),            // num entities
		LE16(0),            // start x
		LE16(0),            // start y
		0,                  // start layer
		0,                  // start direction (north)
		LE16(9),            // num strings
		LE16(0)             // num zones
	};
	if (!check_read(file, expected_header, sizeof(expected_header))) { return 0; }

	fseek(file, 235, SEEK_CUR); // skip unused

	uchar expected_strings[9 * 2] = {};
	if (!check_read(file, expected_strings, sizeof(expected_strings))) { return 0; }

	uint16_t width = read_uint16(file);
	uint16_t height = read_uint16(file);

	uchar expected_layer_header[26] = {
		LE16(0),          // flags
		LE32(0x3F800000), // parallax x (1.0f)
		LE32(0x3F800000), // parallax y (1.0f)
		LE32(0),          // scrolling x (0.0f)
		LE32(0),          // scrolling y (0.0f)
		LE32(0),          // num segments
		0,                // reflective
		0, 0, 0           // reserved
	};
	if (!check_read(file, expected_layer_header, sizeof(expected_layer_header))) { return 0; }

	uint16_t name_length = read_uint16(file);
	fseek(file, name_length, SEEK_CUR);

	return (size_t)(width * height * 2);
}
