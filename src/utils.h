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

#define RANGE(x) (x).begin(), (x).end()

#define HI_NYB(n) (uchar)(((n) & 0xF0) >> 4)
#define LO_NYB(n) (uchar)((n) & 0x0F)
#define LE16(n) (uchar)((n) & 0xFFUL), (uchar)(((n) & 0xFF00UL) >> 8)
#define LE32(n) (uchar)((n) & 0xFFUL), (uchar)(((n) & 0xFF00UL) >> 8), (uchar)(((n) & 0xFF0000UL) >> 16), (uchar)(((n) & 0xFF000000UL) >> 24)
#define BE16(n) (uchar)(((n) & 0xFF00UL) >> 8), (uchar)((n) & 0xFFUL)
#define BE32(n) (uchar)(((n) & 0xFF000000UL) >> 24), (uchar)(((n) & 0xFF0000UL) >> 16), (uchar)(((n) & 0xFF00UL) >> 8), (uchar)((n) & 0xFFUL)

typedef uint8_t size8_t;
typedef uint16_t size16_t;
typedef uint32_t size32_t;
typedef uint64_t size64_t;

bool starts_with_ignore_case(std::string_view s, std::string_view p);
bool ends_with_ignore_case(std::string_view s, std::string_view p);
void add_dot_ext(const char *f, const char *ext, char *s);
int text_width(const char *l, int pad);
int text_width(const char *l, int pad = 0);
bool file_exists(const char *f);
size_t file_size(const char *f);

#endif
