#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <unordered_map>
#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Native_File_Chooser.H>
#pragma warning(pop)

#include "utils.h"
#include "widgets.h"
#include "image.h"
#include "tile-buttons.h"
#include "tilemap.h"
#include "tileset.h"
#include "modal-dialog.h"
#include "option-dialogs.h"
#include "help-window.h"

#define NUM_RECENT 10

class Main_Window : public Fl_Double_Window {
private:
	// GUI containers
	Fl_Menu_Bar *_menu_bar;
	Toolbar *_toolbar;
	Fl_Group *_main_group, *_left_group, *_right_group;
	OS_Tabs *_tileset_tabs, *_tilemap_tabs;
	OS_Tab *_tileset_bank0_tab, *_tileset_bank1_tab, *_tileset_bank2_tab, *_tileset_bank3_tab;
	Workpane *_tileset_pane;
	OS_Tab *_tilemap_tiles_tab, *_tilemap_attributes_tab;
	Workspace *_tilemap_scroll;
	Toolbar *_status_bar;
	// GUI inputs
	DnD_Receiver *_tilemap_dnd_receiver, *_tileset_dnd_receiver;
	Fl_Menu_Item *_recent_tilemap_mis[NUM_RECENT], *_recent_tileset_mis[NUM_RECENT];
	Fl_Menu_Item *_classic_theme_mi = NULL, *_aero_theme_mi = NULL, *_metro_theme_mi = NULL, *_aqua_theme_mi = NULL,
		*_greybird_theme_mi = NULL, *_metal_theme_mi = NULL, *_blue_theme_mi = NULL, *_olive_theme_mi = NULL,
		*_rose_gold_theme_mi = NULL, *_dark_theme_mi = NULL;
	Fl_Menu_Item *_plain_format_mi = NULL, *_sgb_border_format_mi = NULL, *_rby_town_map_format_mi = NULL,
		*_gsc_town_map_format_mi = NULL, *_pc_town_map_format_mi = NULL, *_pokegear_card_format_mi = NULL;
	Fl_Menu_Item *_grid_mi, *_rainbow_tiles_mi = NULL;
	Toolbar_Button *_new_tb, *_open_tb, *_save_tb, *_print_tb, *_load_tb, *_add_tb, *_reload_tb, *_undo_tb, *_redo_tb;
	Toolbar_Toggle_Button *_grid_tb, *_rainbow_tiles_tb;
	Default_Spinner *_tilemap_width;
	Toolbar_Button *_resize_tb, *_reformat_tb;
	Toolbar_Button *_image_to_tiles_tb;
	Toolbar_Toggle_Button *_x_flip_tb, *_y_flip_tb;
	Tile_Button *_tile_buttons[NUM_TILES];
	Default_Spinner *_color;
	OS_Check_Button *_priority, *_obp1;
	// GUI outputs
	Label *_width_heading, *_tileset_name, *_tilemap_name, *_tile_heading;
	Tile_Swatch *_current_tile;
	Status_Bar_Field *_tilemap_dimensions, *_tilemap_format, *_hover_id, *_hover_xy, *_hover_landmark;
	// Conditional menu items
	Fl_Menu_Item *_reload_tilesets_mi = NULL, *_unload_tilesets_mi = NULL, *_close_mi = NULL, *_save_mi = NULL,
		*_save_as_mi = NULL, *_print_mi = NULL;
	Fl_Menu_Item *_undo_mi = NULL, *_redo_mi = NULL;
	Fl_Menu_Item *_tilemap_width_mi = NULL, *_resize_mi = NULL, *_reformat_mi = NULL;
	// Dialogs
	Fl_Native_File_Chooser *_tilemap_open_chooser, *_tilemap_save_chooser, *_tileset_load_chooser, *_png_chooser;
	Modal_Dialog *_error_dialog, *_warning_dialog, *_success_dialog, *_unsaved_dialog, *_about_dialog;
	New_Tilemap_Dialog *_new_tilemap_dialog;
	Tilemap_Width_Dialog *_tilemap_width_dialog;
	Resize_Dialog *_resize_dialog;
	Add_Tileset_Dialog *_add_tileset_dialog;
	Image_To_Tiles_Dialog *_image_to_tiles_dialog;
	Help_Window *_help_window;
	// Data
	std::string _tilemap_file;
	std::vector<std::string> _tileset_files;
	std::string _recent_tilemaps[NUM_RECENT], _recent_tilesets[NUM_RECENT];
	Tilemap _tilemap;
	std::vector<Tileset> _tilesets;
	Tile_Button *_selected = NULL;
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
	inline bool x_flip(void) const { return _x_flip_tb->active() && !!_x_flip_tb->value(); }
	inline bool y_flip(void) const { return _y_flip_tb->active() && !!_y_flip_tb->value(); }
	inline int sgb_color(void) const { return _color->active() ? (int)_color->value() : -1; }
	inline bool priority(void) const { return _priority->active() && !!_priority->value(); }
	inline bool obp1(void) const { return _obp1->active() && !!_obp1->value(); }
	inline const char *modified_filename(void) const {
		return unsaved() ? _tilemap_file.empty() ? NEW_TILEMAP_NAME : fl_filename_name(_tilemap_file.c_str()) : "";
	}
	inline void map_editable(bool e) { _map_editable = e; }
	int handle(int event);
	void update_status(Tile_Tessera *tt);
	void flood_fill(Tile_Tessera *tt);
	void substitute_tile(Tile_Tessera *tt);
	void swap_tiles(Tile_Tessera *tt);
	inline void new_tilemap(size_t width, size_t height) { open_tilemap(NULL, width, height); }
	void open_tilemap(const char *filename, size_t width = 0, size_t height = 0);
	void open_recent_tilemap(int n);
	inline void load_tileset(const char *filename) { unload_tilesets_cb(NULL, this); add_tileset(filename); }
	void add_tileset(const char *filename, int start = 0x00, int offset = 0, int length = 0);
	void load_recent_tileset(int n);
private:
	void store_recent_tilemap(void);
	void update_recent_tilemaps(void);
	void store_recent_tileset(void);
	void update_recent_tilesets(void);
	void update_tilemap_metadata(void);
	void update_tileset_metadata(void);
	void update_active_controls(void);
	void refresh_theme(void);
	void resize_tilemap(void);
	bool save_tilemap(bool force);
	void image_to_tiles(void);
private:
	// Drag-and-drop
	static void drag_and_drop_tilemap_cb(DnD_Receiver *dndr, Main_Window *mw);
	static void drag_and_drop_tileset_cb(DnD_Receiver *dndr, Main_Window *mw);
	// File menu
	static void new_cb(Fl_Widget *w, Main_Window *mw);
	static void open_cb(Fl_Widget *w, Main_Window *mw);
	static void open_recent_tilemap_cb(Fl_Menu_ *m, Main_Window *mw);
	static void clear_recent_tilemaps_cb(Fl_Menu_ *m, Main_Window *mw);
	static void close_cb(Fl_Widget *w, Main_Window *mw);
	static void save_cb(Fl_Widget *w, Main_Window *mw);
	static void save_as_cb(Fl_Widget *w, Main_Window *mw);
	static void load_tileset_cb(Fl_Widget *w, Main_Window *mw);
	static void add_tileset_cb(Fl_Widget *w, Main_Window *mw);
	static void reload_tilesets_cb(Fl_Widget *w, Main_Window *mw);
	static void load_recent_tileset_cb(Fl_Menu_ *m, Main_Window *mw);
	static void clear_recent_tilesets_cb(Fl_Menu_ *m, Main_Window *mw);
	static void unload_tilesets_cb(Fl_Widget *w, Main_Window *mw);
	static void print_cb(Fl_Widget *w, Main_Window *mw);
	static void exit_cb(Fl_Widget *w, Main_Window *mw);
	// Edit menu
	static void undo_cb(Fl_Widget *w, Main_Window *mw);
	static void redo_cb(Fl_Widget *w, Main_Window *mw);
	// View menu
	static void classic_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void aero_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void metro_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void aqua_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void greybird_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void metal_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void blue_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void olive_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void rose_gold_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void dark_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void grid_cb(Fl_Menu_ *m, Main_Window *mw);
	static void rainbow_tiles_cb(Fl_Menu_ *m, Main_Window *mw);
	// Tools menu
	static void tilemap_width_cb(Fl_Menu_ *m, Main_Window *mw);
	static void resize_cb(Fl_Menu_ *m, Main_Window *mw);
	static void reformat_cb(Fl_Menu_ *m, Main_Window *mw);
	static void image_to_tiles_cb(Fl_Widget *w, Main_Window *mw);
	// Format menu
	static void plain_format_cb(Fl_Menu_ *m, Main_Window *mw);
	static void sgb_border_format_cb(Fl_Menu_ *m, Main_Window *mw);
	static void rby_town_map_format_cb(Fl_Menu_ *m, Main_Window *mw);
	static void gsc_town_map_format_cb(Fl_Menu_ *m, Main_Window *mw);
	static void pc_town_map_format_cb(Fl_Menu_ *m, Main_Window *mw);
	static void pokegear_card_format_cb(Fl_Menu_ *m, Main_Window *mw);
	// Help menu
	static void help_cb(Fl_Widget *w, Main_Window *mw);
	static void about_cb(Fl_Widget *w, Main_Window *mw);
	// Toolbar buttons
	static void grid_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void rainbow_tiles_tb_cb(Toolbar_Button *tb, Main_Window *mw);
	static void tilemap_width_tb_cb(OS_Spinner *ss, Main_Window *mw);
	static void x_flip_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	static void y_flip_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	// Tilemap
	static void tilemap_tabs_cb(OS_Tabs *ts, Main_Window *mw);
	static void change_tile_cb(Tile_Tessera *tt, Main_Window *mw);
	// Tileset
	static void select_tile_cb(Tile_Button *tb, Main_Window *mw);
};

#endif
