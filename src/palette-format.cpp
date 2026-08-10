#include <vector>
#include <random>
#include <string>
#include <sstream>
#include <cctype>
#include <cstring>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#pragma warning(pop)

#include "utils.h"
#include "image.h"
#include "option-dialogs.h"

static const char *palette_names[NUM_PALETTE_FORMATS] = {
	"Indexed in tileset image",
	"Pixel image (PNG)",
	"Pixel image (BMP)",
	"Assembly (RGB)",
	"PaintShop Pro (JASC-PAL)",
	"Adobe Color Table (ACT)",
	"Adobe Color Swatch (ACO)",
	"Adobe Swatch Exchange (ASE)",
	"Animator Pro (COL)",
	"Microsoft (RIFF)",
	"paint.net (TXT)",
	"GIMP (GPL)",
	"CorelDRAW (XML)",
	"superfamiconv (JSON)",
	"Fractint (MAP)",
	"Lospec (HEX)"
};

const char *palette_name(Palette_Format pal_fmt) {
	return palette_names[(int)pal_fmt];
}

int palette_max_name_width() {
	int mw = 0;
	for (const char *pal_name : palette_names) {
		mw = std::max(mw, text_width(pal_name, 6));
	}
	return mw;
}

static const char *palette_extensions[NUM_PALETTE_FORMATS] = {
	NULL, ".pal.png", ".pal.bmp", ".pal", ".pal", ".act", ".aco", ".ase",
	".col", ".riff", ".txt", ".gpl", ".xml", ".json", ".map", ".hex"
};

const char *palette_extension(Palette_Format pal_fmt) {
	return palette_extensions[(int)pal_fmt];
}

static bool write_graphic_palette(const char *f, const Palettes &palettes, size_t nc) {
	int w = (int)nc, h = (int)palettes.size();
	if (w % 16 == 0) { w /= 16; h *= 16; }
	Fl_Image_Surface *surface = new Fl_Image_Surface(w, h);
	surface->set_current();

	fl_rectf(0, 0, w, h, FL_BLACK);
	int i = 0;
	for (const Palette &palette : palettes) {
		int j = 0;
		for (Fl_Color c : palette) {
			fl_color(c);
			fl_point(j % w, i + j / w);
			j++;
		}
		i++;
	}

	Fl_RGB_Image *img = surface->image();
	delete surface;
	Fl_Display_Device::display_device()->set_current();

	Image::Result result = Image::write_image(f, img);
	delete img;

	return result == Image::Result::IMAGE_OK;
}

static inline int hex_char(int c) {
	return c + (c < 0xA ? '0' : 'a' - 0xA);
}

static void write_guid(FILE *file) {
	static std::random_device rd;
	static std::mt19937_64 gen(rd());
	static std::uniform_int_distribution<> dis(0x0, 0xF);
	static std::uniform_int_distribution<> dis2(0x8, 0xB);
	// GUID version 4 variant 1: xxxxxxxx-xxxx-4xxx-Xxxx-xxxxxxxxxxxx
	for (int i = 0; i < 36; i++) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			fputc('-', file); // group separator
		}
		else if (i == 14) {
			fputc('4', file); // version (4 bits)
		}
		else if (i == 19) {
			fputc(hex_char(dis2(gen)), file); // variant (2 bits)
		}
		else {
			fputc(hex_char(dis(gen)), file);
		}
	}
}

bool write_palette(const char *f, const Palettes &palettes, Palette_Format pal_fmt, size_t nc) {
	if (pal_fmt == Palette_Format::INDEXED) {
		// The indexed tileset image is already written
		return true;
	}

	if (pal_fmt == Palette_Format::PNG || pal_fmt == Palette_Format::BMP) {
		// The file extension determines the image format
		return write_graphic_palette(f, palettes, nc);
	}

	FILE *file = fl_fopen(f, "wb");
	if (!file) { return false; }

	size_t n = palettes.size() * nc;
	if (pal_fmt == Palette_Format::RGB) {
		// <https://github.com/pret/pokecrystal/blob/master/macros/gfx.asm#:~:text=MACRO%20RGB>
		int p = 0;
		for (const Palette &palette : palettes) {
			fprintf(file, "; palette %d\n", p++);
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "\tRGB %02d, %02d, %02d\n", (int)(r / 8), (int)(g / 8), (int)(b / 8));
			}
		}
	}
	else if (pal_fmt == Palette_Format::JASC) {
		// <https://www.selapa.net/swatches/colors/fileformats.php#psp_pal>
		fprintf(file, "JASC-PAL\r\n0100\r\n%zu\r\n", n);
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "%d %d %d\r\n", (int)r, (int)g, (int)b);
			}
		}
	}
	else if (pal_fmt == Palette_Format::ACT) {
		// <https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577411_pgfId-1070626>
		uchar rgb[3] = {};
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				Fl::get_color(c, rgb[0], rgb[1], rgb[2]);
				fwrite(rgb, 1, sizeof(rgb), file);
			}
		}
		memset(rgb, 0, sizeof(rgb));
		for (size_t i = n; i < MAX_PALETTE_LENGTH; i++) {
			fwrite(rgb, 1, sizeof(rgb), file);
		}
		uchar footer[4] = {BE16(n), BE16(0)};
		fwrite(footer, 1, sizeof(footer), file);
	}
	else if (pal_fmt == Palette_Format::ACO) {
		// <https://www.cyotek.com/blog/writing-photoshop-color-swatch-aco-files-using-csharp>
		// <https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577411_pgfId-1055819>
		uchar header[4] = {BE16(1), BE16(n)};
		fwrite(header, 1, sizeof(header), file);
		uchar rgb[10] = {};
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				Fl::get_color(c, rgb[2], rgb[4], rgb[6]);
				rgb[3] = rgb[2]; rgb[5] = rgb[4]; rgb[7] = rgb[6];
				fwrite(rgb, 1, sizeof(rgb), file);
			}
		}
	}
	else if (pal_fmt == Palette_Format::ASE) {
		// <https://www.cyotek.com/blog/writing-adobe-swatch-exchange-ase-files-using-csharp>
		// <http://www.selapa.net/swatches/colors/fileformats.php#adobe_ase>
		uchar header[12] = {
			'A', 'S', 'E', 'F', // magic number
			BE16(1),            // major version
			BE16(0),            // minor version
			BE32(n)             // num blocks
		};
		fwrite(header, 1, sizeof(header), file);
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				float rf = r / 255.0f, gf = g / 255.0f, bf = b / 255.0f;
				uint32_t ri, gi, bi;
				memcpy(&ri, &rf, 4); memcpy(&gi, &gf, 4); memcpy(&bi, &bf, 4);
				uchar block[42] = {
					BE16(1),  // block type (color entry)
					BE32(36), // block length
					BE16(8),  // name length
					// block name (UTF-16 "#rrggbb")
					0, '#',
					0, (uchar)hex_char(HI_NYB(r)),
					0, (uchar)hex_char(LO_NYB(r)),
					0, (uchar)hex_char(HI_NYB(g)),
					0, (uchar)hex_char(LO_NYB(g)),
					0, (uchar)hex_char(HI_NYB(b)),
					0, (uchar)hex_char(LO_NYB(b)),
					0, 0,
					'R', 'G', 'B', ' ',           // color model
					BE32(ri), BE32(gi), BE32(bi), // color values
					BE16(0)                       // color type (global)
				};
				fwrite(block, 1, sizeof(block), file);
			}
		}
	}
	else if (pal_fmt == Palette_Format::COL) {
		// <https://www.fileformat.info/format/animator-col/corion.htm>
		uchar header[8] = {
			LE32(8 + MAX_PALETTE_LENGTH * 3), // file size
			LE16(0xB123),                     // magic number
			LE16(0),                          // version
		};
		fwrite(header, 1, sizeof(header), file);
		uchar rgb[3] = {};
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				Fl::get_color(c, rgb[0], rgb[1], rgb[2]);
				fwrite(rgb, 1, sizeof(rgb), file);
			}
		}
		memset(rgb, 0, sizeof(rgb));
		for (size_t i = n; i < MAX_PALETTE_LENGTH; i++) {
			fwrite(rgb, 1, sizeof(rgb), file);
		}
	}
	else if (pal_fmt == Palette_Format::RIFF) {
		// <https://www.cyotek.com/blog/writing-microsoft-riff-palette-pal-files-with-csharp>
		// <https://worms2d.info/Palette_file>
		uchar header[24] = {
			'R', 'I', 'F', 'F', // chunk ID
			LE32(16 + 4 * n),   // chunk size
			'P', 'A', 'L', ' ', // format
			'd', 'a', 't', 'a', // subchunk ID
			LE32(4 + 4 * n),    // subchunk size
			LE16(0x0300),       // version
			LE16(n),            // num colors
		};
		fwrite(header, 1, sizeof(header), file);
		uchar rgb[4] = {};
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				Fl::get_color(c, rgb[0], rgb[1], rgb[2]);
				fwrite(rgb, 1, sizeof(rgb), file);
			}
		}
	}
	else if (pal_fmt == Palette_Format::TXT) {
		// <https://www.getpaint.net/doc/latest/WorkingWithPalettes.html>
		fputs("; paint.net Palette File\n", file);
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "FF%02X%02X%02X\n", (int)r, (int)g, (int)b);
			}
		}
	}
	else if (pal_fmt == Palette_Format::GPL) {
		// <https://docs.gimp.org/2.10/en/gimp-concepts-palettes.html>
		// <http://www.selapa.net/swatches/colors/fileformats.php#gimp_gpl>
		const char *name = fl_filename_name(f);
		fprintf(file, "GIMP Palette\nName: %s\nColumns: 16\n#\n", name);
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "% 3d % 3d % 3d\t#%02x%02x%02x\n", (int)r, (int)g, (int)b, (int)r, (int)g, (int)b);
			}
		}
	}
	else if (pal_fmt == Palette_Format::XML) {
		// <https://community.coreldraw.com/sdk/w/articles/177/creating-color-palettes>
		fputs("<?xml version=\"1.0\"?>\r\n<palette name=\"tiles\" guid=\"", file);
		write_guid(file);
		fputs("\">\r\n  <colors>\r\n", file);
		for (const Palette &palette : palettes) {
			fputs("    <page>\r\n", file);
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "      <color cs=\"RGB\" tints=\"%.9g,%.9g,%.9g\"/>\r\n", r / 255.0f, g / 255.0f, b / 255.0f);
			}
			fputs("    </page>\r\n", file);
		}
		fputs("  </colors>\r\n</palette>\r\n", file);
	}
	else if (pal_fmt == Palette_Format::JSON) {
		// <https://github.com/Optiroc/SuperFamiconv/blob/master/src/Palette.cpp#:~:text=Palette::to_json>
		fputs("{\r\n  \"palettes\":[\r\n", file);
		bool pp = false;
		for (const Palette &palette : palettes) {
			fputs(pp ? "," : "    ", file);
			fputc('[', file);
			bool pc = false;
			for (Fl_Color c : palette) {
				if (pc) { fputc(',', file); }
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "\r\n      \"#%02x%02x%02x\"", (int)r, (int)g, (int)b);
				pc = true;
			}
			fputs("\r\n    ]", file);
			pp = true;
		}
		fputs("\r\n  ],\r\n  \"palettes_native_rgb\":[\r\n", file);
		pp = false;
		for (const Palette &palette : palettes) {
			fputs(pp ? "," : "    ", file);
			fputc('[', file);
			bool pc = false;
			for (Fl_Color c : palette) {
				if (pc) { fputc(',', file); }
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "\r\n      [%d,%d,%d]", (int)(r / 8), (int)(g / 8), (int)(b / 8));
				pc = true;
			}
			fputs("\r\n    ]", file);
			pp = true;
		}
		fputs("\r\n  ]\r\n}", file);
	}
	else if (pal_fmt == Palette_Format::MAP) {
		// <http://eyecandyarchive.com/Fractint/docs/Fractint.txt#:~:text=3.2%20Palette%20Maps>
		// <https://softologyblog.wordpress.com/2019/03/23/automatic-color-palette-creation/>
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "%d %d %d\n", (int)r, (int)g, (int)b);
			}
		}
	}
	else if (pal_fmt == Palette_Format::HEX) {
		// <https://lospec.com/palette-list>
		for (const Palette &palette : palettes) {
			for (Fl_Color c : palette) {
				uchar r, g, b;
				Fl::get_color(c, r, g, b);
				fprintf(file, "%02x%02x%02x\r\n", (int)r, (int)g, (int)b);
			}
		}
	}

	fclose(file);
	return true;
}

bool write_tilepal(const char *f, const std::vector<size_t> &tileset, const std::vector<int> &tile_palettes) {
	FILE *file = fl_fopen(f, "wb");
	if (!file) { return false; }

	fputs("MACRO pertilepals\nrept _NARG / 2\n\tdn \\2, \\1\n\tshift 2\nendr\nENDM\n", file);
	size_t nc = 16;
	size_t nt = tileset.size();
	size_t np = std::max(nt, (size_t)(nc * 3));
	for (size_t i = 0; i < np; i++) {
		if (!(i % nc)) {
			fputs("\n\tpertilepals ", file);
		}
		if (i < nt) {
			size_t ti = tileset[i];
			int pi = ti < tile_palettes.size() ? tile_palettes[ti] : 0;
			fprintf(file, "%d", pi);
		}
		else {
			fputc('0', file);
		}
		if (i < np - 1 && i % nc != nc - 1) {
			fputs(", ", file);
		}
	}
	if (np % 2) {
		fputs(", 0", file);
	}
	fputc('\n', file);

	fclose(file);
	return true;
}

// ---- Palette reading ----

namespace Palette_Parse {

static inline void push_rgb(std::vector<uchar> &rgb, int r, int g, int b) {
	rgb.push_back((uchar)std::clamp(r, 0, 255));
	rgb.push_back((uchar)std::clamp(g, 0, 255));
	rgb.push_back((uchar)std::clamp(b, 0, 255));
}

// Expand a 5-bit channel (0-31) to 8 bits, matching NORMRGB in tile.h.
static inline uchar expand5(int c) {
	c &= 0x1F;
	return (uchar)((c << 3) | (c >> 2));
}

static bool hex_byte(const std::string &s, size_t i, int &out) {
	auto val = [](char c) -> int {
		if (c >= '0' && c <= '9') { return c - '0'; }
		if (c >= 'a' && c <= 'f') { return c - 'a' + 0xA; }
		if (c >= 'A' && c <= 'F') { return c - 'A' + 0xA; }
		return -1;
	};
	if (i + 1 >= s.size()) { return false; }
	int hi = val(s[i]), lo = val(s[i+1]);
	if (hi < 0 || lo < 0) { return false; }
	out = hi * 16 + lo;
	return true;
}

bool looks_like_text(const std::vector<uchar> &data) {
	size_t n = std::min(data.size(), (size_t)512);
	if (n == 0) { return false; }
	size_t printable = 0;
	for (size_t i = 0; i < n; i++) {
		uchar c = data[i];
		if (c == '\t' || c == '\r' || c == '\n' || (c >= 0x20 && c < 0x7F)) { printable++; }
	}
	return printable * 10 >= n * 9; // at least 90% printable ASCII
}

bool jasc(const std::string &text, std::vector<uchar> &rgb) {
	std::istringstream in(text);
	std::string line;
	if (!std::getline(in, line) || line.rfind("JASC-PAL", 0) != 0) { return false; }
	std::getline(in, line); // version, e.g. "0100"
	size_t count = 0;
	if (std::getline(in, line)) { count = (size_t)strtoul(line.c_str(), NULL, 10); }
	int r, g, b;
	while ((count == 0 || rgb.size() / 3 < count) && (in >> r >> g >> b)) {
		push_rgb(rgb, r, g, b);
	}
	return !rgb.empty();
}

bool bgr555(const std::vector<uchar> &data, std::vector<uchar> &rgb) {
	size_t n = data.size() / 2;
	for (size_t i = 0; i < n; i++) {
		uint16_t v = (uint16_t)(data[i*2] | (data[i*2+1] << 8));
		push_rgb(rgb, expand5(v & 0x1F), expand5((v >> 5) & 0x1F), expand5((v >> 10) & 0x1F));
	}
	return !rgb.empty();
}

bool act(const std::vector<uchar> &data, std::vector<uchar> &rgb) {
	size_t ncolors = data.size() / 3;
	// Optional 4-byte footer holds the color count and transparency index (big-endian).
	if (data.size() == MAX_PALETTE_LENGTH * 3 + 4) {
		size_t declared = (size_t)((data[MAX_PALETTE_LENGTH*3] << 8) | data[MAX_PALETTE_LENGTH*3+1]);
		if (declared > 0 && declared <= MAX_PALETTE_LENGTH) { ncolors = declared; }
	}
	for (size_t i = 0; i < ncolors; i++) {
		push_rgb(rgb, data[i*3], data[i*3+1], data[i*3+2]);
	}
	return !rgb.empty();
}

bool hex(const std::string &text, std::vector<uchar> &rgb) {
	std::istringstream in(text);
	std::string line;
	while (std::getline(in, line)) {
		size_t a = line.find_first_not_of(" \t\r\n#");
		if (a == std::string::npos || line[a] == ';') { continue; }
		int r, g, b;
		if (hex_byte(line, a, r) && hex_byte(line, a+2, g) && hex_byte(line, a+4, b)) {
			push_rgb(rgb, r, g, b);
		}
	}
	return !rgb.empty();
}

bool gpl(const std::string &text, std::vector<uchar> &rgb) {
	std::istringstream in(text);
	std::string line;
	while (std::getline(in, line)) {
		size_t a = line.find_first_not_of(" \t\r\n");
		if (a == std::string::npos || !isdigit((uchar)line[a])) { continue; } // skip header/name lines
		std::istringstream ls(line);
		int r, g, b;
		if (ls >> r >> g >> b) { push_rgb(rgb, r, g, b); }
	}
	return !rgb.empty();
}

bool paint_net(const std::string &text, std::vector<uchar> &rgb) {
	std::istringstream in(text);
	std::string line;
	while (std::getline(in, line)) {
		size_t a = line.find_first_not_of(" \t\r\n");
		if (a == std::string::npos || line[a] == ';') { continue; }
		int r, g, b; // lines are AARRGGBB; take RR GG BB
		if (hex_byte(line, a+2, r) && hex_byte(line, a+4, g) && hex_byte(line, a+6, b)) {
			push_rgb(rgb, r, g, b);
		}
	}
	return !rgb.empty();
}

bool fractint_map(const std::string &text, std::vector<uchar> &rgb) {
	std::istringstream in(text);
	std::string line;
	while (std::getline(in, line)) {
		size_t a = line.find_first_not_of(" \t\r\n");
		if (a == std::string::npos || !isdigit((uchar)line[a])) { continue; }
		std::istringstream ls(line);
		int r, g, b;
		if (ls >> r >> g >> b) { push_rgb(rgb, r, g, b); }
	}
	return !rgb.empty();
}

bool rgb_asm(const std::string &text, std::vector<uchar> &rgb) {
	std::istringstream in(text);
	std::string line;
	while (std::getline(in, line)) {
		size_t sc = line.find(';'); // strip line comment
		if (sc != std::string::npos) { line.resize(sc); }
		size_t p = std::string::npos; // locate the RGB macro token
		for (size_t i = 0; i + 2 < line.size(); i++) {
			if (toupper((uchar)line[i]) == 'R' && toupper((uchar)line[i+1]) == 'G' && toupper((uchar)line[i+2]) == 'B') {
				p = i + 3;
				break;
			}
		}
		if (p == std::string::npos) { continue; }
		int vals[3], nv = 0; // parse three 5-bit values (decimal or $hex)
		for (size_t i = p; i < line.size() && nv < 3; ) {
			char c = line[i];
			if (c == '$') {
				long v = 0; bool any = false;
				for (i++; i < line.size() && isxdigit((uchar)line[i]); i++) {
					char h = line[i];
					v = v * 16 + (h <= '9' ? h - '0' : toupper((uchar)h) - 'A' + 0xA);
					any = true;
				}
				if (any) { vals[nv++] = (int)v; }
			}
			else if (isdigit((uchar)c)) {
				long v = 0;
				for (; i < line.size() && isdigit((uchar)line[i]); i++) { v = v * 10 + (line[i] - '0'); }
				vals[nv++] = (int)v;
			}
			else { i++; }
		}
		if (nv == 3) { push_rgb(rgb, expand5(vals[0]), expand5(vals[1]), expand5(vals[2])); }
	}
	return !rgb.empty();
}

}

static bool read_graphic_palette(const char *f, std::vector<uchar> &rgb) {
	Fl_RGB_Image *img = ends_with_ignore_case(f, ".png") ?
		(Fl_RGB_Image *)new Fl_PNG_Image(f) : (Fl_RGB_Image *)new Fl_BMP_Image(f);
	if (!img || img->fail() || !img->data() || !img->data()[0]) { delete img; return false; }
	int w = img->w(), h = img->h(), d = img->d(), ld = img->ld();
	if (ld == 0) { ld = w * d; }
	const uchar *data = (const uchar *)img->data()[0];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			const uchar *px = data + y * ld + x * d;
			if (d >= 3) { rgb.push_back(px[0]); rgb.push_back(px[1]); rgb.push_back(px[2]); }
			else { rgb.push_back(px[0]); rgb.push_back(px[0]); rgb.push_back(px[0]); } // grayscale
		}
	}
	delete img;
	return !rgb.empty();
}

static bool slurp_file(const char *f, std::vector<uchar> &data) {
	FILE *file = fl_fopen(f, "rb");
	if (!file) { return false; }
	size_t n = file_size(file);
	data.resize(n);
	size_t r = n ? fread(data.data(), 1, n, file) : 0;
	fclose(file);
	return r == n;
}

Palette_Result read_palette(const char *f, Palettes &palettes, size_t palettes_size, size_t palette_size,
	Palette_Format *detected) {
	palettes.clear();
	if (palette_size == 0) { return Palette_Result::PALETTE_UNSUPPORTED; } // true-RGB formats have no indexed palette

	std::string sf(f ? f : "");
	std::vector<uchar> rgb; // flat r,g,b triples

	if (ends_with_ignore_case(sf, ".png") || ends_with_ignore_case(sf, ".bmp")) {
		if (!read_graphic_palette(f, rgb)) { return Palette_Result::PALETTE_BAD_FILE; }
		if (detected) { *detected = ends_with_ignore_case(sf, ".png") ? Palette_Format::PNG : Palette_Format::BMP; }
	}
	else if (ends_with_ignore_case(sf, ".aco") || ends_with_ignore_case(sf, ".ase") ||
		ends_with_ignore_case(sf, ".col") || ends_with_ignore_case(sf, ".riff") ||
		ends_with_ignore_case(sf, ".xml") || ends_with_ignore_case(sf, ".json")) {
		return Palette_Result::PALETTE_UNSUPPORTED; // container formats: not read yet
	}
	else {
		std::vector<uchar> data;
		if (!slurp_file(f, data)) { return Palette_Result::PALETTE_BAD_FILE; }
		if (data.empty()) { return Palette_Result::PALETTE_EMPTY; }
		std::string text(reinterpret_cast<const char *>(data.data()), data.size());

		bool ok = false;
		if (ends_with_ignore_case(sf, ".gbapal")) { ok = Palette_Parse::bgr555(data, rgb); } // read-only (no BGR555 writer)
		else if (ends_with_ignore_case(sf, ".act")) { ok = Palette_Parse::act(data, rgb); if (detected) { *detected = Palette_Format::ACT; } }
		else if (ends_with_ignore_case(sf, ".hex")) { ok = Palette_Parse::hex(text, rgb); if (detected) { *detected = Palette_Format::HEX; } }
		else if (ends_with_ignore_case(sf, ".gpl")) { ok = Palette_Parse::gpl(text, rgb); if (detected) { *detected = Palette_Format::GPL; } }
		else if (ends_with_ignore_case(sf, ".txt")) { ok = Palette_Parse::paint_net(text, rgb); if (detected) { *detected = Palette_Format::TXT; } }
		else if (ends_with_ignore_case(sf, ".map")) { ok = Palette_Parse::fractint_map(text, rgb); if (detected) { *detected = Palette_Format::MAP; } }
		else if (ends_with_ignore_case(sf, ".asm") || ends_with_ignore_case(sf, ".inc") ||
			ends_with_ignore_case(sf, ".s") || ends_with_ignore_case(sf, ".z80")) {
			ok = Palette_Parse::rgb_asm(text, rgb);
			if (detected) { *detected = Palette_Format::RGB; }
		}
		else if (ends_with_ignore_case(sf, ".pal")) {
			// Ambiguous extension: JASC text, RGB assembly, or SNES/GBA BGR555 binary.
			if (starts_with_ignore_case(text, "JASC-PAL")) { ok = Palette_Parse::jasc(text, rgb); if (detected) { *detected = Palette_Format::JASC; } }
			else if (Palette_Parse::looks_like_text(data)) {
				ok = Palette_Parse::rgb_asm(text, rgb);
				if (!ok) { rgb.clear(); }
				else if (detected) { *detected = Palette_Format::RGB; }
			}
			if (!ok) { rgb.clear(); ok = Palette_Parse::bgr555(data, rgb); } // read-only (no BGR555 writer)
		}
		else { return Palette_Result::PALETTE_BAD_EXT; }

		if (!ok) { return Palette_Result::PALETTE_BAD_FILE; }
	}

	if (rgb.size() < 3) { return Palette_Result::PALETTE_EMPTY; }

	size_t ncolors = rgb.size() / 3;
	for (size_t i = 0; i + palette_size <= ncolors && palettes.size() < palettes_size; i += palette_size) {
		Palette pal;
		pal.reserve(palette_size);
		for (size_t j = 0; j < palette_size; j++) {
			size_t k = (i + j) * 3;
			pal.push_back(fl_rgb_color(rgb[k], rgb[k+1], rgb[k+2]));
		}
		palettes.push_back(pal);
	}
	if (palettes.empty()) {
		// Fewer colors than a full palette: pad the single palette with black.
		Palette pal;
		pal.reserve(palette_size);
		for (size_t j = 0; j < palette_size; j++) {
			if (j < ncolors) {
				size_t k = j * 3;
				pal.push_back(fl_rgb_color(rgb[k], rgb[k+1], rgb[k+2]));
			}
			else { pal.push_back(FL_BLACK); }
		}
		palettes.push_back(pal);
	}
	return Palette_Result::PALETTE_OK;
}

const char *palette_error_message(Palette_Result result) {
	switch (result) {
	case Palette_Result::PALETTE_OK:
		return "OK.";
	case Palette_Result::PALETTE_BAD_FILE:
		return "Cannot parse file format.";
	case Palette_Result::PALETTE_BAD_EXT:
		return "Unknown file extension.";
	case Palette_Result::PALETTE_EMPTY:
		return "No colors found.";
	case Palette_Result::PALETTE_UNSUPPORTED:
		return "Unsupported palette format.";
	default:
		return "Unspecified error.";
	}
}
