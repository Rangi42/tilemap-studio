#include <cstdio>
#include <cstring>
#include <vector>

#pragma warning(push, 0)
#include <FL/fl_utf8.h>
#pragma warning(pop)

#include "tilemap.h"
#include "version.h"

static uchar get_number(FILE *file, int &c) {
	bool hex = false;
	int b = c;
	if (b == '0') {
		b = fgetc(file);
		if (b == 'X' || b == 'x') {
			hex = true;
			b = fgetc(file);
		}
	}
	uchar v = 0;
	if (hex) {
		while (isxdigit(b)) {
			v = v * 16 + (uchar)(b - (b < 'A' ? '0' : b < 'a' ? 'A' - 0xA : 'a' - 0xA));
			b = fgetc(file);
		}
	}
	else {
		while (isdigit(b)) {
			v = v * 10 + (uchar)(b - '0');
			b = fgetc(file);
		}
	}
	c = b;
	return v;
}

static bool import_csv_tiles(FILE *file, std::vector<uchar> &bytes) {
	bool got_number = false;
	for (int c = fgetc(file); c != EOF;) {
		if (isdigit(c) && !got_number) {
			uchar v = get_number(file, c);
			bytes.push_back(v);
			got_number = true;
			continue;
		}
		else if (c == ',') {
			if (got_number) {
				got_number = false;
			}
			else {
				bytes.push_back(0);
			}
		}
		else if ((c == '\n' || c == '\r') && got_number) {
			got_number = false;
		}
		else if (!isspace(c)) {
			return false;
		}
		c = fgetc(file);
	}
	return true;
}

static void skip_c_line_comment(FILE *file) {
	for (int c = fgetc(file); c != '\r' && c != '\n' && c != EOF; c = fgetc(file));
}

static void skip_c_block_comment(FILE *file) {
	for (int c = fgetc(file); c != EOF; c = fgetc(file)) {
		while (c != '*' && c != EOF) { c = fgetc(file); }
		while (c == '*') { c = fgetc(file); }
		if (c == '/') { return; }
	}
}

static bool import_c_tiles(FILE *file, std::vector<uchar> &bytes) {
	enum class State { PRELUDE, ARRAY, COMMA };
	State state = State::PRELUDE;
	for (int c = fgetc(file); c != EOF;) {
		switch (state) {
		case State::PRELUDE:
			if (c == '{') {
				state = State::ARRAY;
			}
			else if (c == '/') {
			maybe_comment:
				c = fgetc(file);
				if (c == '*') {
					skip_c_block_comment(file);
				}
				else if (c == '/') {
					skip_c_line_comment(file);
				}
				else {
					continue;
				}
			}
			break;
		case State::ARRAY:
			if (isdigit(c)) {
				uchar v = get_number(file, c);
				bytes.push_back(v);
				state = State::COMMA;
			}
			else {
				goto not_number_or_comma_in_array;
			}
			[[fallthrough]];
		case State::COMMA:
			if (c == ',') {
				state = State::ARRAY;
			}
			else {
			not_number_or_comma_in_array:
				if (c == '/') {
					goto maybe_comment;
				}
				else if (c == '}') {
					return true;
				}
				else if (!isspace(c)) {
					return false;
				}
			}
			break;
		}
		c = fgetc(file);
	}
	return false;
}

static bool check_read(FILE *file, uchar *expected, size_t n) {
	std::vector<uchar> buffer(n);
	size_t r = fread(buffer.data(), 1, n, file);
	return r == n && (!expected || !memcmp(buffer.data(), expected, n));
}

static uint16_t read_uint16(FILE *file) {
	int lo = fgetc(file);
	int hi = fgetc(file);
	return (uint16_t)(((hi & 0xFF00) >> 8) | (lo & 0xFF));
}

static bool import_rmp_tiles(FILE *file, std::vector<uchar> &bytes) {
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
	if (!check_read(file, expected_header, sizeof(expected_header))) { return false; }
	uchar expected_unused_and_strings[235 + 9 * 2] = {0};
	if (!check_read(file, expected_unused_and_strings, sizeof(expected_unused_and_strings))) { return false; }

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
	if (!check_read(file, expected_layer_header, sizeof(expected_layer_header))) { return false; }

	uint16_t name_length = read_uint16(file);
	fseek(file, name_length, SEEK_CUR);

	size_t n = width * height * 2;
	bytes.resize(n);
	if (size_t r = fread(bytes.data(), 1, n, file); r != n) { return false; }

	// GBA_4BPP tile IDs must be 0x3FF or below
	for (size_t i = 1; i < n; i += 2) {
		if (bytes[i] > 0x03) { return false; }
	}
	return true;
}

static Tilemap::Result import_file_bytes(const char *f, std::vector<uchar> &bytes, bool attrmap) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return attrmap ? Tilemap::Result::ATTRMAP_BAD_FILE : Tilemap::Result::TILEMAP_BAD_FILE; }
	bool valid = (ends_with_ignore_case(f, ".rmp") ? import_rmp_tiles : ends_with_ignore_case(f, ".csv") ? import_csv_tiles :
		import_c_tiles)(file, bytes);
	fclose(file);
	if (!valid) { return attrmap ? Tilemap::Result::ATTRMAP_INVALID : Tilemap::Result::TILEMAP_INVALID; }
	return Tilemap::Result::TILEMAP_OK;
}

Tilemap::Result Tilemap::import_tiles(const char *tf, const char *af) {
	std::vector<uchar> tbytes, abytes;
	Result result = import_file_bytes(tf, tbytes, false);
	if (result != Result::TILEMAP_OK) { return (_result = result); }
	if (af && af[0]) {
		result = import_file_bytes(af, abytes, true);
		if (result != Result::TILEMAP_OK) { return (_result = result); }
	}
	_modified = true;
	return (_result = make_tiles(tbytes, abytes));
}
