#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Native_File_Chooser.H>
#pragma warning(pop)

#include "utils.h"
#include "widgets.h"
#include "image.h"
#include "tile-selection.h"
#include "tile-buttons.h"
#include "tilemap.h"
#include "tileset.h"
#include "modal-dialog.h"
#include "option-dialogs.h"
#include "help-window.h"

#define NEW_TILEMAP_NAME "New Tilemap"
#define IMPORTED_TILEMAP_NAME "Imported Tilemap"

#define NUM_RECENT 10

struct Image_to_Tiles_Result {
	const char *tilemap_filename;
	const char *attrmap_filename;
	const char *tileset_filename;
	Tilemap_Format fmt;
	size_t width;
	uint16_t start_id;
	bool success;
};

class Main_Window : public Fl_Overlay_Window {
private:
	// GUI containers
	Fl_Sys_Menu_Bar *_menu_bar;
	Toolbar *_toolbar;
	Fl_Group *_main_group, *_left_group, *_right_group;
	Bounded_Group *_top_group;
	OS_Tabs *_left_tabs;
	OS_Tab *_tiles_tab, *_palettes_tab;
	Workspace *_tiles_scroll;
	Workpane *_palettes_pane;
	Workspace *_tilemap_scroll;
	Toolbar *_status_bar;
	// GUI inputs
	DnD_Receiver *_tilemap_dnd_receiver, *_tileset_dnd_receiver;
	Fl_Menu_Item *_recent_tilemap_mis[NUM_RECENT], *_recent_tileset_mis[NUM_RECENT];
	Fl_Menu_Item *_classic_theme_mi = NULL, *_aero_theme_mi = NULL, *_metro_theme_mi = NULL, *_aqua_theme_mi = NULL,
		*_greybird_theme_mi = NULL, *_ocean_theme_mi = NULL, *_blue_theme_mi = NULL, *_olive_theme_mi = NULL,
		*_rose_gold_theme_mi = NULL, *_dark_theme_mi = NULL, *_brushed_metal_theme_mi = NULL, *_high_contrast_theme_mi = NULL;
	Fl_Menu_Item *_grid_mi = NULL, *_rainbow_tiles_mi = NULL, *_bold_palettes_mi = NULL, *_auto_tileset_mi = NULL,
		*_transparent_mi = NULL, *_full_screen_mi = NULL;
	Toolbar_Button *_new_tb, *_open_tb, *_save_tb, *_print_tb, *_load_tb, *_add_tb, *_reload_tb, *_undo_tb, *_redo_tb,
		*_zoom_in_tb, *_zoom_out_tb;
	Toolbar_Toggle_Button *_grid_tb, *_rainbow_tiles_tb, *_bold_palettes_tb;
	Default_Spinner *_tilemap_width;
	Toolbar_Button *_resize_tb, *_shift_tb, *_reformat_tb;
	Toolbar_Button *_tileset_width_tb, *_shift_tileset_tb;
	Toolbar_Button *_image_to_tiles_tb;
	Toolbar_Toggle_Button *_x_flip_tb, *_y_flip_tb, *_priority_tb, *_obp1_tb;
	Tile_Button *_tile_buttons[MAX_NUM_TILES];
	Palette_Button *_palette_buttons[MAX_NUM_PALETTES];
	Default_Slider *_transparency;
	// GUI outputs
	Label *_width_heading, *_tileset_name, *_tilemap_name, *_tile_heading;
	Tile_Swatch *_current_tile, *_current_attributes;
	Label *_tilemap_dimensions, *_tilemap_format, *_zoom_level, *_hover_id, *_hover_xy, *_hover_landmark;
	// Conditional menu items
	Fl_Menu_Item *_close_mi = NULL, *_save_mi = NULL, *_save_as_mi = NULL, *_export_mi = NULL, *_print_mi = NULL;
	Fl_Menu_Item *_reload_tilesets_mi = NULL, *_unload_tilesets_mi = NULL;
	Fl_Menu_Item *_undo_mi = NULL, *_redo_mi = NULL;
	Fl_Menu_Item *_erase_selection_mi = NULL, *_x_flip_selection_mi = NULL, *_y_flip_selection_mi = NULL,
		*_shift_selected_ids_mi = NULL, *_copy_selection_mi = NULL, *_select_all_mi = NULL;
	Fl_Menu_Item *_zoom_in_mi = NULL, *_zoom_out_mi = NULL;
	Fl_Menu_Item *_tilemap_width_mi = NULL, *_crop_to_selection_mi = NULL, *_resize_mi = NULL, *_shift_mi = NULL,
		*_transpose_mi = NULL, *_shift_tile_ids_mi = NULL, *_reformat_mi = NULL;
	Fl_Menu_Item *_shift_tileset_mi = NULL;
	// Dialogs
	Fl_Native_File_Chooser *_tilemap_open_chooser, *_tilemap_save_chooser, *_tilemap_import_chooser, *_tilemap_export_chooser,
		*_tileset_load_chooser, *_image_print_chooser;
	Modal_Dialog *_error_dialog, *_success_dialog, *_unsaved_dialog, *_about_dialog;
	Tilemap_Options_Dialog *_tilemap_options_dialog;
	New_Tilemap_Dialog *_new_tilemap_dialog;
	Group_Width_Dialog *_tileset_width_dialog, *_tilemap_width_dialog;
	Print_Options_Dialog *_print_options_dialog;
	Resize_Dialog *_resize_dialog;
	Shift_Dialog *_shift_dialog;
	Shift_Tileset_Dialog *_shift_tileset_dialog, *_shift_tile_ids_dialog;
	Reformat_Dialog *_reformat_dialog;
	Add_Tileset_Dialog *_add_tileset_dialog;
	Image_To_Tiles_Dialog *_image_to_tiles_dialog;
	Help_Window *_help_window;
	// Data
	std::string _tilemap_file, _attrmap_file, _tilemap_basename;
	std::vector<std::string> _tileset_files;
	std::string _recent_tilemaps[NUM_RECENT], _recent_tilesets[NUM_RECENT];
	Tilemap _tilemap;
	std::vector<Tileset> _tilesets;
	int _tileset_width = 16;
	Tile_Selection _selection;
	Palette_Button *_selected_palette = NULL;
	// Work properties
	bool _map_editable = false;
	// Window size cache
	int _wx, _wy, _ww, _wh;
#ifdef __X11__
	// Window icons
	Pixmap _icon_pixmap, _icon_mask;
#endif
public:
	Main_Window(int x, int y, int w, int h, const char *l = NULL);
	~Main_Window();
	void show(void);
	bool maximized(void) const;
	void maximize(void);
	void apply_transparency(void);
	inline bool transparent(void) const { return _transparent_mi && !!_transparent_mi->value(); }
	inline bool full_screen(void) const { return _full_screen_mi && !!_full_screen_mi->value(); }
	inline bool unsaved(void) const { return _tilemap.modified(); }
	inline uint16_t tile_id(void) const { return _selection.selected() ? _selection.id() : 0x000; }
	inline bool x_flip(void) const { return _x_flip_tb->active() && !!_x_flip_tb->value(); }
	inline bool y_flip(void) const { return _y_flip_tb->active() && !!_y_flip_tb->value(); }
	inline int palette(void) const { return _selected_palette && Config::show_attributes() ? (int)_selected_palette->palette() : -1; }
	inline bool priority(void) const { return _priority_tb->visible() && !!_priority_tb->value(); }
	inline bool obp1(void) const { return _obp1_tb->visible() && !!_obp1_tb->value(); }
	inline int tileset_width(void) const { return _tileset_width; }
	inline Tile_Selection &selection(void) { return _selection; }
	inline const char *modified_filename(void) const {
		return unsaved() ? _tilemap_file.empty() ? _tilemap_basename.c_str() : fl_filename_name(_tilemap_file.c_str()) : "";
	}
	inline bool map_editable(void) const { return _map_editable; }
	inline void map_editable(bool e) { _map_editable = e; }
	inline bool dropping(void) const { return _tilemap_scroll->dropping() || _tiles_scroll->dropping() || _palettes_pane->dropping(); }
	void draw_overlay(void);
	int handle(int event);
	void clear_flips(void);
	void update_icons(void);
	void update_zoom(int old_zoom);
	void update_selection_status(void);
	void update_selection_controls(void);
	void update_status(Tile_Tessera *tt);
	void edit_tile(Tile_Tessera *tt);
	void flood_fill(Tile_Tessera *tt);
	void substitute_tile(Tile_Tessera *tt);
	void swap_tiles(Tile_Tessera *tt);
	void erase_selection(void);
	void x_flip_selection(void);
	void y_flip_selection(void);
	void shift_selected_ids(int d, int n);
	void copy_selection(void) const;
	void select_all(void);
	void new_tilemap(size_t width, size_t height);
	void open_tilemap(const char *filename);
	void open_recent_tilemap(int n);
	inline void load_tileset(const char *filename, bool warn = false) {
		unload_tilesets_cb(NULL, this); add_tileset(filename, 0x000, 0, 0, warn);
	}
	inline void unload_tilesets(void) {
		for (Tileset &t : _tilesets) { t.clear(); } _tilesets.clear(); _tileset_files.clear(); update_tileset_metadata();
	}
	void add_tileset(const char *filename, int start = 0x000, int offset = 0, int length = 0, bool quiet = false);
	void load_recent_tileset(int n);
	void load_corresponding_tileset(const char *filename = NULL);
	void open_converted_tilemap(Image_to_Tiles_Result output);
	void open_or_import_or_convert(const char *filename);
	void drag_and_drop_tilemap(const char *filename);
private:
	void store_recent_tilemap(void);
	void update_recent_tilemaps(void);
	void store_recent_tileset(void);
	void update_recent_tilesets(void);
	void update_tilemap_metadata(void);
	void update_tileset_metadata(void);
	void update_active_controls(void);
	void update_tileset_width(int tw);
	void resize_tilemap(size_t w, size_t h, int px, int py);
	void shift_tilemap(void);
	void shift_tileset(void);
	void transpose_tilemap(void);
	void shift_tile_ids(void);
	void reformat_tilemap(void);
	void save_tilemap(bool force);
	void import_tilemap(const char *filename);
	void setup_tilemap(const char *basename, int old_tileset_size, const char *tileset_filename = NULL);
	void export_tilemap(const char *filename);
	void select_tile(uint16_t id);
	void highlight_tile(uint16_t id);
	void select_palette(int palette);
	Image_to_Tiles_Result image_to_tiles(void);
private:
	// Drag-and-drop
	static void drag_and_drop_tilemap_cb(DnD_Receiver *dndr, Main_Window *mw);
	static void drag_and_drop_tileset_cb(DnD_Receiver *dndr, Main_Window *mw);
	static void common_drop_tilemap_cb(const char *filename, Main_Window *mw);
	// Window
	static void exit_cb(Fl_Widget *w, Main_Window *mw);
	// Tilemap menu
	static void new_cb(Fl_Widget *w, Main_Window *mw);
	static void open_cb(Fl_Widget *w, Main_Window *mw);
	static void open_recent_tilemap_cb(Fl_Menu_ *m, Main_Window *mw);
	static void clear_recent_tilemaps_cb(Fl_Menu_ *m, Main_Window *mw);
	static void close_cb(Fl_Widget *w, Main_Window *mw);
	static void save_cb(Fl_Widget *w, Main_Window *mw);
	static void save_as_cb(Fl_Widget *w, Main_Window *mw);
	static void import_cb(Fl_Widget *w, Main_Window *mw);
	static void export_cb(Fl_Widget *w, Main_Window *mw);
	static void print_cb(Fl_Widget *w, Main_Window *mw);
	// Tileset menu
	static void load_tileset_cb(Fl_Widget *w, Main_Window *mw);
	static void add_tileset_cb(Fl_Widget *w, Main_Window *mw);
	static void reload_tilesets_cb(Fl_Widget *w, Main_Window *mw);
	static void load_recent_tileset_cb(Fl_Menu_ *m, Main_Window *mw);
	static void clear_recent_tilesets_cb(Fl_Menu_ *m, Main_Window *mw);
	static void unload_tilesets_cb(Fl_Widget *w, Main_Window *mw);
	static void auto_load_tileset_cb(Fl_Menu_ *m, Main_Window *mw);
	// Edit menu
	static void undo_cb(Fl_Widget *w, Main_Window *mw);
	static void redo_cb(Fl_Widget *w, Main_Window *mw);
	static void erase_selection_cb(Fl_Menu_ *w, Main_Window *mw);
	static void x_flip_selection_cb(Fl_Menu_ *w, Main_Window *mw);
	static void y_flip_selection_cb(Fl_Menu_ *w, Main_Window *mw);
	static void shift_selected_ids_cb(Fl_Menu_ *w, Main_Window *mw);
	static void copy_selection_cb(Fl_Menu_ *w, Main_Window *mw);
	static void select_all_cb(Fl_Menu_ *w, Main_Window *mw);
	// View menu
	static void classic_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void aero_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void metro_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void aqua_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void greybird_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void ocean_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void blue_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void olive_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void rose_gold_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void dark_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void brushed_metal_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void high_contrast_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void zoom_in_cb(Fl_Widget *w, Main_Window *mw);
	static void zoom_out_cb(Fl_Widget *w, Main_Window *mw);
	static void zoom_reset_cb(Fl_Widget *w, Main_Window *mw);
	static void grid_cb(Fl_Menu_ *m, Main_Window *mw);
	static void rainbow_tiles_cb(Fl_Menu_ *m, Main_Window *mw);
	static void bold_palettes_cb(Fl_Menu_ *m, Main_Window *mw);
	static void transparent_cb(Fl_Menu_ *m, Main_Window *mw);
	// Tools menu
	static void tilemap_width_cb(Fl_Menu_ *m, Main_Window *mw);
	static void crop_to_selection_cb(Fl_Menu_ *m, Main_Window *mw);
	static void resize_cb(Fl_Menu_ *m, Main_Window *mw);
	static void shift_cb(Fl_Menu_ *m, Main_Window *mw);
	static void transpose_cb(Fl_Menu_ *m, Main_Window *mw);
	static void shift_tile_ids_cb(Fl_Menu_ *m, Main_Window *mw);
	static void reformat_cb(Fl_Menu_ *m, Main_Window *mw);
	static void tileset_width_cb(Fl_Widget *w, Main_Window *mw);
	static void shift_tileset_cb(Fl_Widget *w, Main_Window *mw);
	static void image_to_tiles_cb(Fl_Widget *w, Main_Window *mw);
	// Help menu
	static void help_cb(Fl_Widget *w, Main_Window *mw);
	static void about_cb(Fl_Widget *w, Main_Window *mw);
	// Toolbar buttons
	static void grid_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void rainbow_tiles_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void bold_palettes_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void tilemap_width_tb_cb(OS_Spinner *ss, Main_Window *mw);
	static void full_screen_cb(Fl_Menu_ *m, Main_Window *mw);
	static void x_flip_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	static void y_flip_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	static void priority_cb(OS_Check_Button *cb, Main_Window *mw);
	static void obp1_cb(OS_Check_Button *cb, Main_Window *mw);
	static void transparency_cb(Default_Slider *ds, Main_Window *mw);
	// Tileset
	static void change_tab_cb(OS_Tabs *ts, Main_Window *mw);
	static void select_tile_cb(Tile_Button *tb, Main_Window *mw);
	static void select_palette_cb(Palette_Button *pb, Main_Window *mw);
	// Tilemap
	static void change_tile_cb(Tile_Tessera *tt, Main_Window *mw);
};

#endif
