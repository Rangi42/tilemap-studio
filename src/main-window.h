#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <unordered_map>
#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Menu_Bar.H>
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

#define NUM_RECENT 10

class Main_Window : public Fl_Overlay_Window {
private:
	// GUI containers
	Fl_Menu_Bar *_menu_bar;
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
	Fl_Menu_Item *_grid_mi = NULL, *_rainbow_tiles_mi = NULL, *_bold_palettes_mi = NULL, *_auto_tileset_mi = NULL;
	Toolbar_Button *_new_tb, *_open_tb, *_save_tb, *_print_tb, *_load_tb, *_add_tb, *_reload_tb, *_undo_tb, *_redo_tb,
		*_zoom_in_tb, *_zoom_out_tb;
	Toolbar_Toggle_Button *_grid_tb, *_rainbow_tiles_tb, *_bold_palettes_tb;
	Default_Spinner *_tilemap_width;
	Toolbar_Button *_resize_tb, *_reformat_tb;
	Toolbar_Button *_image_to_tiles_tb;
	Toolbar_Toggle_Button *_x_flip_tb, *_y_flip_tb, *_priority_tb, *_obp1_tb;
	Tile_Button *_tile_buttons[MAX_NUM_TILES];
	Palette_Button *_palette_buttons[MAX_NUM_PALETTES];
	Default_Slider *_transparency;
	// GUI outputs
	Label *_width_heading, *_tileset_name, *_tilemap_name, *_tile_heading;
	Tile_Swatch *_current_tile, *_current_attributes;
	Status_Bar_Field *_tilemap_dimensions, *_tilemap_format, *_zoom_level, *_hover_id, *_hover_xy, *_hover_landmark;
	// Conditional menu items
	Fl_Menu_Item *_close_mi = NULL, *_save_mi = NULL, *_save_as_mi = NULL, *_print_mi = NULL;
	Fl_Menu_Item *_reload_tilesets_mi = NULL, *_unload_tilesets_mi = NULL;
	Fl_Menu_Item *_undo_mi = NULL, *_redo_mi = NULL;
	Fl_Menu_Item *_zoom_in_mi = NULL, *_zoom_out_mi = NULL;
	Fl_Menu_Item *_tilemap_width_mi = NULL, *_resize_mi = NULL, *_reformat_mi = NULL;
	// Dialogs
	Fl_Native_File_Chooser *_tilemap_open_chooser, *_tilemap_save_chooser, *_tileset_load_chooser, *_image_print_chooser;
	Modal_Dialog *_error_dialog, *_warning_dialog, *_success_dialog, *_unsaved_dialog, *_about_dialog;
	Tilemap_Options_Dialog *_tilemap_options_dialog;
	New_Tilemap_Dialog *_new_tilemap_dialog;
	Group_Width_Dialog *_tileset_width_dialog, *_tilemap_width_dialog;
	Print_Options_Dialog *_print_options_dialog;
	Resize_Dialog *_resize_dialog;
	Reformat_Dialog *_reformat_dialog;
	Add_Tileset_Dialog *_add_tileset_dialog;
	Image_To_Tiles_Dialog *_image_to_tiles_dialog;
	Help_Window *_help_window;
	// Data
	std::string _tilemap_file, _attrmap_file;
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
#ifndef _WIN32
	// Window icons
	Pixmap _icon_pixmap, _icon_mask;
#endif
public:
	Main_Window(int x, int y, int w, int h, const char *l = NULL);
	~Main_Window();
	void show(void);
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
		return unsaved() ? _tilemap_file.empty() ? NEW_TILEMAP_NAME : fl_filename_name(_tilemap_file.c_str()) : "";
	}
	inline bool map_editable(void) const { return _map_editable; }
	inline void map_editable(bool e) { _map_editable = e; }
	void draw_overlay(void);
	int handle(int event);
	void clear_flips(void);
	void update_icons(void);
	void update_zoom(int old_zoom);
	void update_selection_status(void);
	void update_status(Tile_Tessera *tt);
	void edit_tile(Tile_Tessera *tt);
	void flood_fill(Tile_Tessera *tt);
	void substitute_tile(Tile_Tessera *tt);
	void swap_tiles(Tile_Tessera *tt);
	inline void new_tilemap(size_t width, size_t height) { open_tilemap(NULL, width, height); }
	void open_tilemap(const char *filename, size_t width = 0, size_t height = 0);
	void open_recent_tilemap(int n);
	inline void load_tileset(const char *filename) { unload_tilesets_cb(NULL, this); add_tileset(filename); }
	void add_tileset(const char *filename, int start = 0x00, int offset = 0, int length = 0);
	void load_recent_tileset(int n);
	void load_corresponding_tileset(const char *filename);
private:
	void store_recent_tilemap(void);
	void update_recent_tilemaps(void);
	void store_recent_tileset(void);
	void update_recent_tilesets(void);
	void update_tilemap_metadata(void);
	void update_tileset_metadata(void);
	void update_active_controls(void);
	void update_tileset_width(int tw);
	void resize_tilemap(void);
	void reformat_tilemap(void);
	bool save_tilemap(bool force);
	void select_tile(uint16_t id);
	void highlight_tile(uint16_t id);
	void select_palette(int palette);
	bool image_to_tiles(void);
private:
	// Drag-and-drop
	static void drag_and_drop_tilemap_cb(DnD_Receiver *dndr, Main_Window *mw);
	static void drag_and_drop_tileset_cb(DnD_Receiver *dndr, Main_Window *mw);
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
	static void print_cb(Fl_Widget *w, Main_Window *mw);
	// Tileset menu
	static void load_tileset_cb(Fl_Widget *w, Main_Window *mw);
	static void add_tileset_cb(Fl_Widget *w, Main_Window *mw);
	static void reload_tilesets_cb(Fl_Widget *w, Main_Window *mw);
	static void load_recent_tileset_cb(Fl_Menu_ *m, Main_Window *mw);
	static void clear_recent_tilesets_cb(Fl_Menu_ *m, Main_Window *mw);
	static void unload_tilesets_cb(Fl_Widget *w, Main_Window *mw);
	// Edit menu
	static void undo_cb(Fl_Widget *w, Main_Window *mw);
	static void redo_cb(Fl_Widget *w, Main_Window *mw);
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
	// Tools menu
	static void tileset_width_cb(Fl_Menu_ *m, Main_Window *mw);
	static void tilemap_width_cb(Fl_Menu_ *m, Main_Window *mw);
	static void resize_cb(Fl_Menu_ *m, Main_Window *mw);
	static void reformat_cb(Fl_Menu_ *m, Main_Window *mw);
	static void image_to_tiles_cb(Fl_Widget *w, Main_Window *mw);
	// Options menu
	static void auto_load_tileset_cb(Fl_Menu_ *m, Main_Window *mw);
	// Help menu
	static void help_cb(Fl_Widget *w, Main_Window *mw);
	static void about_cb(Fl_Widget *w, Main_Window *mw);
	// Toolbar buttons
	static void grid_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void rainbow_tiles_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void bold_palettes_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void tilemap_width_tb_cb(OS_Spinner *ss, Main_Window *mw);
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
