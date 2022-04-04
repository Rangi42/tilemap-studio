#ifndef OPTION_DIALOGS_H
#define OPTION_DIALOGS_H

#include <string>

#include "config.h"
#include "utils.h"
#include "widgets.h"
#include "palette-format.h"

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Native_File_Chooser.H>
#pragma warning(pop)

#define NO_FILE_SELECTED_LABEL "No file selected"
#define NO_FILES_SELECTED_LABEL "No file(s) selected"
#define NO_FILES_DETERMINED_LABEL "No file(s) determined"

class Option_Dialog {
protected:
	int _width;
	const char *_title;
	bool _canceled;
	Fl_Double_Window *_dialog;
	Fl_Group *_content;
	Default_Button *_ok_button;
	OS_Button *_cancel_button;
public:
	Option_Dialog(int w, const char *t = NULL);
	virtual ~Option_Dialog();
	inline bool canceled(void) const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
protected:
	void initialize(void);
	void refresh(void);
	void reveal(const Fl_Widget *p);
	virtual void initialize_content(void) = 0;
	virtual int refresh_content(int ww, int dy) = 0;
	virtual void finish(void) {}
public:
	inline bool initialized(void) const { return !!_dialog; }
	inline void show(const Fl_Widget *p) { initialize(); refresh(); reveal(p); }
private:
	static void close_cb(Fl_Widget *, Option_Dialog *od);
	static void cancel_cb(Fl_Widget *, Option_Dialog *od);
};

class Tilemap_Options_Dialog : public Option_Dialog {
private:
	Label *_tilemap_header;
	Dropdown *_format;
	Label *_attrmap_heading;
	Toolbar_Button *_attrmap;
	Label_Button *_attrmap_name;
	Fl_Native_File_Chooser *_attrmap_chooser, *_attrmap_import_chooser;
	std::string _attrmap_filename;
	bool _importing;
public:
	Tilemap_Options_Dialog(const char *t);
	~Tilemap_Options_Dialog();
	inline Tilemap_Format format(void) const { return (Tilemap_Format)_format->value(); }
	inline void format(Tilemap_Format fmt) { initialize(); _format->value((int)fmt); }
	inline const char *attrmap_filename(void) const { return _attrmap_filename.c_str(); }
	inline void importing(bool b) { _importing = b; }
	void update_icons(void);
	void use_tilemap(const char *filename);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
private:
	static void format_cb(Dropdown *d, Tilemap_Options_Dialog *tod);
	static void attrmap_cb(Fl_Widget *w, Tilemap_Options_Dialog *tod);
};

class New_Tilemap_Dialog : public Option_Dialog {
private:
	OS_Spinner *_tilemap_width, *_tilemap_height;
	Dropdown *_format;
public:
	New_Tilemap_Dialog(const char *t);
	~New_Tilemap_Dialog();
	inline size_t tilemap_width(void) const { return (size_t)_tilemap_width->value(); }
	inline void tilemap_width(size_t n) { initialize(); _tilemap_width->value((double)n); }
	inline size_t tilemap_height(void) const { return (size_t)_tilemap_height->value(); }
	inline void tilemap_height(size_t n) { initialize(); _tilemap_height->value((double)n); }
	inline Tilemap_Format format(void) const { return (Tilemap_Format)_format->value(); }
	inline void format(Tilemap_Format fmt) { initialize(); _format->value((int)fmt); }
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
};

class Anchor_Button;

class Print_Options_Dialog {
private:
	const char *_title;
	bool _copied, _canceled;
	Fl_Double_Window *_dialog;
	Label *_show_heading;
	OS_Check_Button *_grid, *_rainbow_tiles, *_palettes, *_bold_palettes;
	Default_Button *_export_button;
	OS_Button *_copy_button, *_cancel_button;
public:
	Print_Options_Dialog(const char *t = NULL);
	~Print_Options_Dialog();
	inline bool copied(void) const { return _copied; }
	inline void copied(bool c) { _copied = c; }
	inline bool canceled(void) const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
	inline bool grid(void) const { return !!_grid->value(); }
	inline void grid(bool g) { initialize(); _grid->value(g); }
	inline bool rainbow_tiles(void) const { return !!_rainbow_tiles->value(); }
	inline void rainbow_tiles(bool r) { initialize(); _rainbow_tiles->value(r); }
	inline bool palettes(void) const { return !!_palettes->value(); }
	inline void palettes(bool p) { initialize(); _palettes->value(p); }
	inline bool bold_palettes(void) const { return !!_bold_palettes->value(); }
	inline void bold_palettes(bool b) { initialize(); _bold_palettes->value(b); }
private:
	void initialize(void);
	void refresh(void);
public:
	inline bool initialized(void) const { return !!_dialog; }
	void show(const Fl_Widget *p);
private:
	static void close_cb(Fl_Widget *, Print_Options_Dialog *pd);
	static void copy_cb(Fl_Widget *, Print_Options_Dialog *pd);
	static void cancel_cb(Fl_Widget *, Print_Options_Dialog *pd);
};

class Resize_Dialog : public Option_Dialog {
public:
	enum class Hor_Align { LEFT, CENTER, RIGHT };
	enum class Vert_Align { TOP, MIDDLE, BOTTOM };
private:
	Default_Spinner *_tilemap_width, *_tilemap_height;
	Anchor_Button *_anchor_buttons[9];
public:
	Resize_Dialog(const char *t);
	~Resize_Dialog();
	inline size_t tilemap_width(void) const { return (size_t)_tilemap_width->value(); }
	inline size_t tilemap_height(void) const { return (size_t)_tilemap_height->value(); }
	inline void tilemap_size(size_t w, size_t h) {
		initialize();
		_tilemap_width->default_value(w);
		_tilemap_height->default_value(h);
	}
	Hor_Align horizontal_anchor(void) const;
	Vert_Align vertical_anchor(void) const;
	int anchor(void) const;
	void anchor(int a);
	void anchor_label(int x, int y, const char *l);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
private:
	static void anchor_button_cb(Anchor_Button *ab, Resize_Dialog *rd);
};

class Shift_Dialog : public Option_Dialog {
private:
	Default_Spinner *_shift_x, *_shift_y;
	Default_Slider *_slide_x, *_slide_y;
public:
	Shift_Dialog(const char *t);
	~Shift_Dialog();
	inline int shift_x(void) const { return (int)_shift_x->value(); }
	inline int shift_y(void) const { return (int)_shift_y->value(); }
	void limit_shift(int w, int h);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
private:
	static void shift_x_cb(Default_Spinner *d, Shift_Dialog *sd);
	static void shift_y_cb(Default_Spinner *d, Shift_Dialog *sd);
	static void slide_x_cb(Default_Slider *d, Shift_Dialog *sd);
	static void slide_y_cb(Default_Slider *d, Shift_Dialog *sd);
};

class Shift_Tileset_Dialog : public Option_Dialog {
private:
	Default_Hex_Spinner *_shift;
	Default_Slider *_slide;
public:
	Shift_Tileset_Dialog(const char *t);
	~Shift_Tileset_Dialog();
	inline int shift(void) const { return (int)_shift->value(); }
	void limit_shift(int n);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
private:
	static void shift_cb(Default_Hex_Spinner *d, Shift_Tileset_Dialog *sd);
	static void slide_cb(Default_Slider *d, Shift_Tileset_Dialog *sd);
};

class Reformat_Dialog : public Option_Dialog {
private:
	Label *_format_header;
	Dropdown *_format;
	OS_Check_Button *_force;
public:
	Reformat_Dialog(const char *t);
	~Reformat_Dialog();
	inline Tilemap_Format format(void) const { return (Tilemap_Format)_format->value(); }
	void format(Tilemap_Format fmt);
	inline bool force(void) const { return !!_force->value(); }
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
};

class Group_Width_Dialog : public Option_Dialog {
private:
	Default_Spinner *_group_width;
public:
	Group_Width_Dialog(const char *t);
	~Group_Width_Dialog();
	inline size_t group_width(void) const { return (size_t)_group_width->value(); }
	inline void group_width(size_t n) { initialize(); _group_width->value((double)n); }
	inline void default_group_width(size_t n) { initialize(); _group_width->default_value((double)n); }
	inline void limit_group_width(size_t n) { initialize(); _group_width->range(1, (double)n); }
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
};

class Add_Tileset_Dialog : public Option_Dialog {
private:
	Label *_tileset_header;
	Default_Hex_Spinner *_start_id, *_offset, *_length;
public:
	Add_Tileset_Dialog(const char *t);
	~Add_Tileset_Dialog();
	inline int start_id(void) const { return _start_id->value(); }
	inline void start_id(int n) { initialize(); _start_id->value(n); }
	inline int offset(void) const { return _offset->value(); }
	inline void offset(int n) { initialize(); _offset->value(n); }
	inline int length(void) const { return _length->value(); }
	inline void length(int n) { initialize(); _length->value(n); }
	void limit_tileset_options(const char *filename);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
};

class Image_To_Tiles_Dialog : public Option_Dialog {
private:
	Label *_tileset_heading, * _tilemap_heading;
	Spacer *_tileset_spacer, *_tilemap_spacer, *_palette_spacer;
	Label * _input_heading, * _output_heading;
	Toolbar_Button *_image, *_tileset;
	Label_Button *_image_name, *_tileset_name;
	OS_Check_Button *_unique_tiles, *_flip_tiles, *_no_extra_blank_tiles;
	Label *_tilemap_name;
	Dropdown *_format;
	Default_Hex_Spinner *_start_id;
	OS_Check_Button *_use_blank;
	Default_Hex_Spinner *_blank_id;
	OS_Check_Button *_palette;
	Label *_palette_name;
	Dropdown *_palette_format;
	Label *_start_index_label;
	Default_Hex_Spinner *_start_index;
	OS_Check_Button *_color_zero;
	OS_Hex_Input *_color_zero_rgb;
	Fl_Button *_color_zero_swatch;
	Fl_Native_File_Chooser *_image_chooser, *_tileset_chooser;
	std::string _image_filename, _tileset_filename, _tilemap_filename, _attrmap_filename, _palette_filename, _tilepal_filename;
	bool _prepared_image;
	bool _picked_palette;
public:
	Image_To_Tiles_Dialog(const char *t);
	~Image_To_Tiles_Dialog();
	inline const char *image_filename(void) const { return _image_filename.c_str(); }
	inline const char *tileset_filename(void) const { return _tileset_filename.c_str(); }
	inline const char *tilemap_filename(void) const { return _tilemap_filename.c_str(); }
	inline const char *attrmap_filename(void) const { return _attrmap_filename.c_str(); }
	inline const char *palette_filename(void) const { return _palette_filename.c_str(); }
	inline const char *tilepal_filename(void) const { return _tilepal_filename.c_str(); }
	inline bool unique_tiles(void) const { return !!_unique_tiles->value(); }
	inline bool flip_tiles(void) const { return !!_flip_tiles->value(); }
	inline bool no_extra_blank_tiles(void) const { return !!_no_extra_blank_tiles->value(); }
	inline Tilemap_Format format(void) const { return (Tilemap_Format)_format->value(); }
	inline void format(Tilemap_Format fmt) { initialize(); _format->value((int)fmt); }
	inline bool palette(void) const { return !!_palette->value(); }
	inline Palette_Format palette_format(void) const { return (Palette_Format)_palette_format->value(); }
	inline bool color_zero(void) const { return !!_color_zero->value(); }
	Fl_Color fl_color_zero(void) const;
	inline uint16_t start_id(void) const { return (uint16_t)_start_id->value(); }
	inline void start_id(uint16_t n) { initialize(); _start_id->value(n); }
	inline bool use_blank(void) const { return !!_use_blank->value(); }
	inline uint16_t blank_id(void) const { return (uint16_t)_blank_id->value(); }
	inline uint8_t start_index(void) const { return (uint8_t)_start_index->value(); }
	inline void start_index(uint8_t n) { initialize(); _start_index->value(n); }
	inline void reshow(const Fl_Widget *p) { _canceled = false; reveal(p); }
	inline void prepare_image(const char *filename) { _image_filename = filename; _prepared_image = true; }
private:
	void update_image_name(void);
	void update_output_names(void);
	void update_flip_tiles(void);
	void update_start_index(void);
	void update_ok_button(void);
	void update_color_zero_swatch(void);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy);
private:
	static void image_cb(Fl_Widget *w, Image_To_Tiles_Dialog *itd);
	static void tileset_cb(Fl_Widget *w, Image_To_Tiles_Dialog *itd);
	static void unique_tiles_cb(OS_Check_Button *cb, Image_To_Tiles_Dialog *itd);
	static void format_cb(Dropdown *dd, Image_To_Tiles_Dialog *itd);
	static void palette_cb(OS_Check_Button *cb, Image_To_Tiles_Dialog *itd);
	static void palette_format_cb(Dropdown *dd, Image_To_Tiles_Dialog *itd);
	static void color_zero_cb(OS_Check_Button *cb, Image_To_Tiles_Dialog *itd);
	static void color_zero_rgb_cb(OS_Hex_Input *cb, Image_To_Tiles_Dialog *itd);
	static void color_zero_swatch_cb(Fl_Button *w, Image_To_Tiles_Dialog *itd);
	static void use_blank_cb(OS_Check_Button *cb, Image_To_Tiles_Dialog *itd);
};

#endif
