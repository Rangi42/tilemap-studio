#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

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

static uchar get_asm_number(const std::string &data, size_t &i) {
	uchar v = 0;
	size_t len = data.length();
	char c = data[i];
	if (c == '#') {
		if (++i == len) { return v; }
		c = data[i];
	}
	if (c == '$') {
		while (++i < len) {
			c = data[i];
			if (!isxdigit(c)) { break; }
			v = v * 16 + (uchar)(c - (c < 'A' ? '0' : c < 'a' ? 'A' - 0xA : 'a' - 0xA));
		}
	}
	else if (c == '&') {
		while (++i < len) {
			c = data[i];
			if (!isdigit(c) || c == '8' || c == '9') { break; }
			v = v * 8 + (uchar)(c - '0');
		}
	}
	else if (c == '%') {
		while (++i < len) {
			c = data[i];
			if (c != '0' && c != '1') { break; }
			v = v * 2 + (uchar)(c - '0');
		}
	}
	else if (isdigit(c)) {
		do {
			v = v * 10 + (uchar)(c - '0');
			if (++i == len) { break; }
			c = data[i];
		} while (isdigit(c));
	}
	return v;
}

static bool import_asm_tiles(std::ifstream &ifs, std::vector<uchar> &bytes) {
	std::regex rx(
		"^"
		R"([ \t]*)" // space
		R"((?:[A-Za-z0-9_\.@#\$]*(?:\b|[ \t:]+))?)" // label (alphanumeric or . @ # $ followed by colons)
		R"((?:\.?(?:[Dd][Bb]|[Bb][Yy][Tt][Ee]?)\b([^;]*))?)" // db (rgbasm), .db (wla-dx), .byte or .byt (ca65)
		R"((?:;.*)?)" // ; comment
		"$"
	);
	while (ifs.good()) {
		std::string line;
		std::getline(ifs, line);
		std::smatch sm;
		std::regex_match(line, sm, rx);
		size_t n = sm.size();
		if (n != 2) { return false; }
		const std::string &data = sm[1];
		bool got_number = false;
		size_t len = data.length();
		for (size_t i = 0; i < len; i++) {
			char c = data[i];
			if ((isdigit(c) || c == '$' || c == '&' || c == '%' || c == '#') && !got_number) {
				uchar v = get_asm_number(data, i);
				i--;
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
			else if (!isspace(c)) {
				return false;
			}
		}
	}
	return true;
}

static bool import_rmp_tiles(FILE *file, std::vector<uchar> &bytes) {
	size_t n = read_rmp_size(file);
	if (n == 0) { return false; }
	bytes.resize(n);
	if (fread(bytes.data(), 1, n, file) != n) { return false; }
	// GBA_4BPP tile IDs must be 0x3FF or below
	for (size_t i = 1; i < n; i += 2) {
		if (bytes[i] > 0x03) { return false; }
	}
	return true;
}

static Tilemap::Result import_file_bytes(const char *f, std::vector<uchar> &bytes, bool attrmap) {
	bool valid = false;
	if (ends_with_ignore_case(f, ".asm") || ends_with_ignore_case(f, ".s") || ends_with_ignore_case(f, ".inc") ||
		ends_with_ignore_case(f, ".z80") || ends_with_ignore_case(f, ".sm83") || ends_with_ignore_case(f, ".gbz80")) {
		std::ifstream ifs;
		open_ifstream(ifs, f);
		if (!ifs.good()) { return attrmap ? Tilemap::Result::ATTRMAP_BAD_FILE : Tilemap::Result::TILEMAP_BAD_FILE; }
		valid = import_asm_tiles(ifs, bytes);
	}
	else {
		FILE *file = fl_fopen(f, "rb");
		if (!file) { return attrmap ? Tilemap::Result::ATTRMAP_BAD_FILE : Tilemap::Result::TILEMAP_BAD_FILE; }
		valid = (ends_with_ignore_case(f, ".rmp") ? import_rmp_tiles : ends_with_ignore_case(f, ".csv") ? import_csv_tiles :
			import_c_tiles)(file, bytes);
		fclose(file);
	}
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
