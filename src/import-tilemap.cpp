#include <cstdio>
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
	bool state = false;
	for (int c = fgetc(file); c != EOF;) {
		if (state) {
			if (c == ',' || c == '\n' || c == '\r') {
				state = false;
			}
			else if (!isspace(c)) {
				return false;
			}
		}
		else {
			if (isdigit(c)) {
				uchar v = get_number(file, c);
				bytes.push_back(v);
				state = true;
				continue;
			}
			else if (c == ',') {
				bytes.push_back(0);
			}
			else if (!isspace(c)) {
				return false;
			}
		}
		c = fgetc(file);
	}
	return true;
}

static bool import_c_tiles(FILE *file, std::vector<uchar> &bytes) {
	enum class State { PRELUDE, ARRAY, COMMA, MAYBE_OPEN_COMMENT, MAYBE_CLOSE_COMMENT, BLOCK_COMMENT, LINE_COMMENT };
	State state = State::PRELUDE, state_before_comment = State::PRELUDE;
	for (int c = fgetc(file); c != EOF;) {
		switch (state) {
		case State::PRELUDE:
			if (c == '{') {
				state = State::ARRAY;
			}
			else if (c == '/') {
				state_before_comment = state;
				state = State::MAYBE_OPEN_COMMENT;
			}
			break;
		case State::ARRAY:
			if (isdigit(c)) {
				uchar v = get_number(file, c);
				bytes.push_back(v);
				state = State::COMMA;
				continue;
			}
			goto not_number_or_comma_in_array;
		case State::COMMA:
			if (c == ',') {
				state = State::ARRAY;
			}
			else {
not_number_or_comma_in_array:
				if (c == '}') {
					c = EOF;
					continue;
				}
				else if (c == '/') {
					state_before_comment = state;
					state = State::MAYBE_OPEN_COMMENT;
				}
				else if (!isspace(c)) {
					return false;
				}
			}
			break;
		case State::MAYBE_OPEN_COMMENT:
			if (c == '*') {
				state = State::BLOCK_COMMENT;
			}
			else if (c == '/') {
				state = State::LINE_COMMENT;
			}
			else {
				state = state_before_comment;
				continue;
			}
			break;
		case State::MAYBE_CLOSE_COMMENT:
			if (c == '/') {
				state = state_before_comment;
				break;
			}
			state = State::BLOCK_COMMENT;
			[[fallthrough]];
		case State::BLOCK_COMMENT:
			if (c == '*') {
				state = State::MAYBE_CLOSE_COMMENT;
			}
			break;
		case State::LINE_COMMENT:
			if (c == '\n' || c == '\r') {
				state = state_before_comment;
			}
			break;
		}
		c = fgetc(file);
	}
	return true;
}

static Tilemap::Result import_file_bytes(const char *f, std::vector<uchar> &bytes, bool attrmap) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return attrmap ? Tilemap::Result::ATTRMAP_BAD_FILE : Tilemap::Result::TILEMAP_BAD_FILE; }
	bool valid = (ends_with_ignore_case(f, ".csv") ? import_csv_tiles : import_c_tiles)(file, bytes);
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
