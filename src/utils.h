#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <stdint.h>
#include <limits>
#include <cmath>
#include <string>
#include <string_view>
#include <algorithm>

#ifdef _WIN32
#define DIR_SEP "\\"
#else
#define DIR_SEP "/"
#endif

#define STRINGIFY(x) _STRINGIFY_HELPER(x)
#define _STRINGIFY_HELPER(x) #x

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

typedef uint8_t size8_t;
typedef uint16_t size16_t;
typedef uint32_t size32_t;
typedef uint64_t size64_t;

extern const std::string whitespace;

bool starts_with(std::string_view s, std::string_view p);
bool ends_with(std::string_view s, std::string_view p);
bool ends_with(std::wstring_view s, std::wstring_view p);
void trim(std::string &s, const std::string &t = whitespace);
void remove_dot_ext(const char *f, char *s);
void add_dot_ext(const char *f, const char *ext, char *s);
int text_width(const char *l, int pad);
int text_width(const char *l, int pad = 0);
bool file_exists(const char *f);
size_t file_size(const char *f);

#endif
