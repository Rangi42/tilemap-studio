#include <vector>
#include <random>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Image_Surface.H>
#pragma warning(pop)

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
