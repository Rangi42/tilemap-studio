#include <cstdlib>
#include <cwctype>
#include <queue>
#include <utility>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Multi_Label.H>
#pragma warning(pop)

#include "version.h"
#include "utils.h"
#include "themes.h"
#include "widgets.h"
#include "modal-dialog.h"
#include "option-dialogs.h"
#include "preferences.h"
#include "config.h"
#include "image.h"
#include "tilemap.h"
#include "tileset.h"
#include "tile.h"
#include "main-window.h"
#include "icons.h"

#ifdef _WIN32
#include "resource.h"
#else
#include <unistd.h>
#include <X11/xpm.h>
#include "app-icon.xpm"
#endif

Main_Window::Main_Window(int x, int y, int w, int h, const char *) : Fl_Double_Window(x, y, w, h, PROGRAM_NAME),
	_tile_buttons(), _tilemap_file(), _attrmap_file(), _tileset_files(), _recent_tilemaps(), _recent_tilesets(),
	_tilemap(), _tilesets(), _wx(x), _wy(y), _ww(w), _wh(h) {

	Tile_State::tilesets(&_tilesets);

	// Get global configs
	Tilemap_Format format_config = (Tilemap_Format)Preferences::get("format", Config::format());
	int zoom_config = Preferences::get("zoom", Config::zoom());
	int grid_config = Preferences::get("grid", Config::grid());
	int rainbow_tiles_config = Preferences::get("rainbow", Config::rainbow_tiles());
	int bold_palettes_config = Preferences::get("bold", Config::bold_palettes());
	Config::format(format_config);
	Config::zoom(zoom_config);
	Config::grid(!!grid_config);
	Config::rainbow_tiles(!!rainbow_tiles_config);
	Config::bold_palettes(!!bold_palettes_config);

	for (int i = 0; i < NUM_RECENT; i++) {
		_recent_tilemaps[i] = Preferences::get_string(Fl_Preferences::Name("recent-map%d", i));
		_recent_tilesets[i] = Preferences::get_string(Fl_Preferences::Name("recent-set%d", i));
	}

	// Populate window

	int wx = 0, wy = 0, ww = w, wh = h;

	// Initialize menu bar
	_menu_bar = new Fl_Menu_Bar(wx, wy, ww, 21);
	wy += _menu_bar->h();
	wh -= _menu_bar->h();

	// Initialize toolbar
	_toolbar = new Toolbar(wx, wy, w, 26);
	_new_tb = new Toolbar_Button(0, 0, 24, 24);
	_open_tb = new Toolbar_Button(0, 0, 24, 24);
	_save_tb = new Toolbar_Button(0, 0, 24, 24);
	_print_tb = new Toolbar_Button(0, 0, 24, 24);
	new Fl_Box(0, 0, 2, 24); new Spacer(0, 0, 2, 24); new Fl_Box(0, 0, 2, 24);
	_load_tb = new Toolbar_Button(0, 0, 24, 24);
	_add_tb = new Toolbar_Button(0, 0, 24, 24);
	_reload_tb = new Toolbar_Button(0, 0, 24, 24);
	new Fl_Box(0, 0, 2, 24); new Spacer(0, 0, 2, 24); new Fl_Box(0, 0, 2, 24);
	_undo_tb = new Toolbar_Button(0, 0, 24, 24);
	_redo_tb = new Toolbar_Button(0, 0, 24, 24);
	new Fl_Box(0, 0, 2, 24); new Spacer(0, 0, 2, 24); new Fl_Box(0, 0, 2, 24);
	_zoom_out_tb = new Toolbar_Button(0, 0, 24, 24);
	_zoom_in_tb = new Toolbar_Button(0, 0, 24, 24);
	new Fl_Box(0, 0, 2, 24); new Spacer(0, 0, 2, 24); new Fl_Box(0, 0, 2, 24);
	_grid_tb = new Toolbar_Toggle_Button(0, 0, 24, 24);
	_rainbow_tiles_tb = new Toolbar_Toggle_Button(0, 0, 24, 24);
	_bold_palettes_tb = new Toolbar_Toggle_Button(0, 0, 24, 24);
	new Fl_Box(0, 0, 2, 24); new Spacer(0, 0, 2, 24); new Fl_Box(0, 0, 2, 24);
	int wgt_w = text_width("Width:", 4);
	_width_heading = new Label(0, 0, wgt_w, 24, "Width:");
	wgt_w = text_width("9999", 2) + 15;
	_tilemap_width = new Default_Spinner(0, 0, wgt_w, 22);
	new Fl_Box(0, 0, 4, 24);
	_resize_tb = new Toolbar_Button(0, 0, 24, 24);
	_reformat_tb = new Toolbar_Button(0, 0, 24, 24);
	new Fl_Box(0, 0, 2, 24); new Spacer(0, 0, 2, 24); new Fl_Box(0, 0, 2, 24);
	_image_to_tiles_tb = new Toolbar_Button(0, 0, 24, 24);
	_toolbar->end();
	wy += _toolbar->h();
	wh -= _toolbar->h();
	begin();

	// Initialize status bar
	_status_bar = new Toolbar(wx, h-23, ww, 23);
	wh -= _status_bar->h();
	_zoom_level = new Status_Bar_Field(0, 0, text_width("Zoom: 10x", 4), 21, "Zoom: 2x");
	new Spacer(0, 0, 2, 21);
	_tilemap_dimensions = new Status_Bar_Field(0, 0, text_width("Tilemap: 9999 x 9999", 4), 21, "");
	new Spacer(0, 0, 2, 21);
	_tilemap_format = new Status_Bar_Field(0, 0, format_max_name_width() + 4, 21, "");
	new Spacer(0, 0, 2, 21);
	_hover_id = new Status_Bar_Field(0, 0, text_width("ID: $A:AA", 4), 21, "");
	new Spacer(0, 0, 2, 21);
	_hover_xy = new Status_Bar_Field(0, 0, text_width("X/Y (9999, 9999)", 4), 21, "");
	new Spacer(0, 0, 2, 21);
	_hover_landmark = new Status_Bar_Field(0, 0, text_width("Landmark (199, 199)", 4), 21, "");
	_status_bar->end();
	begin();

	// Main group
	int wgt_h = 22, win_m = 10, wgt_m = 4, tab_h = OS_TAB_HEIGHT;
	_main_group = new Fl_Group(wx, wy, ww, wh);
	wx += win_m; ww -= win_m * 2;
	wy += win_m; wh -= win_m * 2;
	// Left group
	_left_group = new Fl_Group(wx, wy, 283, wh);
	int gx = _left_group->x(), gy = _left_group->y(), gw = _left_group->w(), gh = _left_group->h();
	_tileset_name = new Label(gx, gy, gw, wgt_h);
	gy += _tileset_name->h(); gh -= _tileset_name->h() + wgt_m + wgt_h - tab_h;
	int qy = gy + wgt_m + wgt_h - tab_h;
	_left_tabs = new OS_Tabs(gx, qy, gw, gh);
	qy += tab_h; gh -= tab_h;
	_tiles_tab = new OS_Tab(gx, qy, gw, gh, "Tiles");
	_tiles_scroll = new Workspace(gx+5, qy+5, gw-10, gh-10);
	int ox = _tiles_scroll->x() + Fl::box_dx(_tiles_scroll->box());
	int oy = _tiles_scroll->y() + Fl::box_dy(_tiles_scroll->box());
	for (int i = 0; i < MAX_NUM_TILES; i++) {
		int tx = ox + (i % TILES_PER_ROW) * TILE_SIZE_2X, ty = oy + (i / TILES_PER_ROW) * TILE_SIZE_2X;
		Tile_Button *tb = new Tile_Button(tx, ty, (uint16_t)i);
		tb->callback((Fl_Callback *)select_tile_cb, this);
		_tile_buttons[i] = tb;
	}
	_tiles_scroll->end();
	_tiles_scroll->type(Fl_Scroll::VERTICAL_ALWAYS);
	_tiles_scroll->resizable(NULL);
	_tiles_tab->resizable(_tiles_scroll);
	_left_tabs->begin();
	int pw = PALETTES_PER_ROW * TILE_SIZE_2X + 2, ph = (MAX_NUM_PALETTES / PALETTES_PER_ROW) * TILE_SIZE_2X + 2;
	_palettes_tab = new OS_Tab(gx, qy, gw, ph+10, "Palettes");
	_palettes_pane = new Workpane(gx+5, qy+5, pw, ph);
	for (int i = 0; i < MAX_NUM_PALETTES; i++) {
		int tx = ox + (i % PALETTES_PER_ROW) * TILE_SIZE_2X, ty = oy + (i / PALETTES_PER_ROW) * TILE_SIZE_2X;
		Palette_Button *pb = new Palette_Button(tx, ty, i);
		pb->callback((Fl_Callback *)select_palette_cb, this);
		_palette_buttons[i] = pb;
	}
	_palettes_pane->end();
	_palettes_pane->resizable(NULL);
	_palettes_tab->begin();
	int wgt_off = text_width("Trans:", 3);
	int ax = _palettes_pane->x() + _palettes_pane->w() + wgt_m;
	int ay = _palettes_pane->y() + (_palettes_pane->h() - wgt_h) / 2;
	int aw = gw - 10 - _palettes_pane->w() - wgt_m;
	_transparency = new Default_Slider(ax+wgt_off, ay, aw-wgt_off, wgt_h, "Trans:");
	_palettes_tab->end();
	_palettes_tab->resizable(NULL);
	_left_tabs->resizable(_tiles_tab);
	_left_group->begin();
	wgt_w = MAX(text_width("Tile: $F:FF", 3), text_width("Tile: $A:AA", 3));
	int qw = wgt_w + wgt_m * 2 + wgt_h * 3;
	int qx = gx + gw - qw;
	_top_group = new Bounded_Group(qx, gy, qw, wgt_h);
	_tile_heading = new Label(qx, gy, wgt_w, wgt_h, "Tile: $0:00");
	qx += _tile_heading->w() + wgt_m;
	_current_tile = new Tile_Swatch(qx+2, gy+2, TILE_SIZE_2X+2, TILE_SIZE_2X+2);
	_current_attributes = new Tile_Swatch(qx+2, gy+2, TILE_SIZE_2X+2, TILE_SIZE_2X+2);
	qx += _current_tile->w() + 4 + wgt_m;
	_x_flip_tb = new Toolbar_Toggle_Button(qx, gy, wgt_h, wgt_h);
	_priority_tb = new Toolbar_Toggle_Button(qx, gy, wgt_h, wgt_h);
	qx += _x_flip_tb->w();
	_y_flip_tb = new Toolbar_Toggle_Button(qx, gy, wgt_h, wgt_h);
	_obp1_tb = new Toolbar_Toggle_Button(qx, gy, wgt_h, wgt_h);
	_top_group->end();
	_top_group->resizable(NULL);
	_left_group->resizable(_left_tabs);
	wx += _left_group->w() + win_m; ww -= _left_group->w() + win_m;
	// Middle spacer
	_main_group->begin();
	Spacer *mid_spacer = new Spacer(wx, wy, 2, wh);
	wx += mid_spacer->w() + win_m; ww -= mid_spacer->w() + win_m;
	// Right group
	_right_group = new Fl_Group(wx, wy, ww, wh);
	gx = _right_group->x(); gy = _right_group->y(); gw = _right_group->w();
	_tilemap_name = new Label(gx, gy, gw, wgt_h);
	wy += _tilemap_name->h() + wgt_m; wh -= _tilemap_name->h() + wgt_m;
	_tilemap_scroll = new Workspace(wx, wy, ww, wh);
	_tilemap_scroll->end();
	_tilemap_scroll->resizable(NULL);
	_right_group->resizable(_tilemap_scroll);
	_main_group->resizable(_right_group);
	begin();

	// Dialogs
	_tilemap_open_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	_tilemap_save_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	_tileset_load_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	_png_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	_error_dialog = new Modal_Dialog(this, "Error", Modal_Dialog::ERROR_ICON);
	_warning_dialog = new Modal_Dialog(this, "Warning", Modal_Dialog::WARNING_ICON);
	_success_dialog = new Modal_Dialog(this, "Success", Modal_Dialog::SUCCESS_ICON);
	_unsaved_dialog = new Modal_Dialog(this, "Warning", Modal_Dialog::WARNING_ICON, true);
	_about_dialog = new Modal_Dialog(this, "About " PROGRAM_NAME, Modal_Dialog::APP_ICON);
	_tilemap_options_dialog = new Tilemap_Options_Dialog("Tilemap Options");
	_new_tilemap_dialog = new New_Tilemap_Dialog("New Tilemap");
	_tilemap_width_dialog = new Tilemap_Width_Dialog("Tilemap Width");
	_resize_dialog = new Resize_Dialog("Resize Tilemap");
	_reformat_dialog = new Reformat_Dialog("Reformat Tilemap");
	_add_tileset_dialog = new Add_Tileset_Dialog("Add Tileset");
	_image_to_tiles_dialog = new Image_To_Tiles_Dialog("Image to Tiles");
	_help_window = new Help_Window(48, 48, 500, 400, PROGRAM_NAME " Help");

	// Drag-and-drop receivers
	_tilemap_dnd_receiver = new DnD_Receiver(0, 0, 0, 0);
	_tilemap_dnd_receiver->callback((Fl_Callback *)drag_and_drop_tilemap_cb, this);
	_tilemap_dnd_receiver->user_data(this);
	_tileset_dnd_receiver = new DnD_Receiver(0, 0, 0, 0);
	_tileset_dnd_receiver->callback((Fl_Callback *)drag_and_drop_tileset_cb, this);
	_tileset_dnd_receiver->user_data(this);

	// Configure window
	size_range(647, 406);
	resizable(_main_group);
	callback((Fl_Callback *)exit_cb, this);
	xclass(PROGRAM_NAME);

	// Configure window icon
#ifdef _WIN32
	icon((const void *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));
#else
	fl_open_display();
	XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display), (char **)&APP_ICON_XPM, &_icon_pixmap, &_icon_mask, NULL);
	icon((const void *)_icon_pixmap);
#endif

	// Configure workspaces
	_tilemap_scroll->dnd_receiver(_tilemap_dnd_receiver);
	_tiles_scroll->dnd_receiver(_tileset_dnd_receiver);
	_palettes_pane->dnd_receiver(_tileset_dnd_receiver);

	// Configure menu bar
	_menu_bar->box(OS_PANEL_THIN_UP_BOX);
	_menu_bar->down_box(FL_FLAT_BOX);

	// Configure menu bar items
	Fl_Menu_Item menu_items[] = {
		// label, shortcut, callback, data, flags
		OS_SUBMENU("Tile&map"),
		OS_MENU_ITEM("&New", FL_COMMAND + 'n', (Fl_Callback *)new_cb, this, 0),
		OS_MENU_ITEM("&Open...", FL_COMMAND + 'o', (Fl_Callback *)open_cb, this, 0),
		OS_MENU_ITEM("Open &Recent", 0, NULL, NULL, FL_SUBMENU),
		// NUM_RECENT items with callback open_recent_tilemap_cb
		OS_NULL_MENU_ITEM(FL_ALT + '1', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '2', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '3', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '4', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '5', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '6', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '7', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '8', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '9', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '0', (Fl_Callback *)open_recent_tilemap_cb, this, 0),
		OS_MENU_ITEM("Clear &Recent", 0, (Fl_Callback *)clear_recent_tilemaps_cb, this, 0),
		{},
		OS_MENU_ITEM("&Close", FL_COMMAND + 'w', (Fl_Callback *)close_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Save", FL_COMMAND + 's', (Fl_Callback *)save_cb, this, 0),
		OS_MENU_ITEM("Save &As...", FL_COMMAND + 'S', (Fl_Callback *)save_as_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Print...", FL_COMMAND + 'p', (Fl_Callback *)print_cb, this, 0),
		{},
		OS_SUBMENU("Tile&set"),
		OS_MENU_ITEM("&Load...", FL_COMMAND + 't', (Fl_Callback *)load_tileset_cb, this, 0),
		OS_MENU_ITEM("&Add...", FL_COMMAND + 'a', (Fl_Callback *)add_tileset_cb, this, 0),
		OS_MENU_ITEM("R&eload", FL_COMMAND + 'r', (Fl_Callback *)reload_tilesets_cb, this, 0),
		OS_MENU_ITEM("Load &Recent", 0, NULL, NULL, FL_SUBMENU),
		// NUM_RECENT items with callback load_recent_tileset_cb
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '1', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '2', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '3', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '4', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '5', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '6', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '7', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '8', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '9', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + FL_SHIFT + '0', (Fl_Callback *)load_recent_tileset_cb, this, 0),
		OS_MENU_ITEM("Clear &Recent", 0, (Fl_Callback *)clear_recent_tilesets_cb, this, 0),
		{},
		OS_MENU_ITEM("&Unload", FL_COMMAND + 'W', (Fl_Callback *)unload_tilesets_cb, this, 0),
		{},
		OS_SUBMENU("&Edit"),
		OS_MENU_ITEM("&Undo", FL_COMMAND + 'z', (Fl_Callback *)undo_cb, this, 0),
		OS_MENU_ITEM("&Redo", FL_COMMAND + 'y', (Fl_Callback *)redo_cb, this, 0),
		{},
		OS_SUBMENU("&View"),
		OS_MENU_ITEM("&Theme", 0, NULL, NULL, FL_SUBMENU | FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Classic", 0, (Fl_Callback *)classic_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::CLASSIC ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Aero", 0, (Fl_Callback *)aero_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::AERO ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Metro", 0, (Fl_Callback *)metro_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::METRO ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("A&qua", 0, (Fl_Callback *)aqua_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::AQUA ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Greybird", 0, (Fl_Callback *)greybird_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::GREYBIRD ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Me&tal", 0, (Fl_Callback *)metal_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::METAL ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Blue", 0, (Fl_Callback *)blue_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::BLUE ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Olive", 0, (Fl_Callback *)olive_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::OLIVE ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Rose Gold", 0, (Fl_Callback *)rose_gold_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::ROSE_GOLD ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Dark", 0, (Fl_Callback *)dark_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::DARK ? FL_MENU_VALUE : 0)),
		{},
		OS_MENU_ITEM("Zoom &In", FL_COMMAND + '=', (Fl_Callback *)zoom_in_cb, this, 0),
		OS_MENU_ITEM("Zoom &Out", FL_COMMAND + '-', (Fl_Callback *)zoom_out_cb, this, 0),
		OS_MENU_ITEM("&Zoom Reset", FL_COMMAND + '0', (Fl_Callback *)zoom_reset_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Grid", FL_COMMAND + 'g', (Fl_Callback *)grid_cb, this,
			FL_MENU_TOGGLE | (Config::grid() ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Rainbow Tiles", FL_COMMAND + 'i', (Fl_Callback *)rainbow_tiles_cb, this,
			FL_MENU_TOGGLE | (Config::rainbow_tiles() ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Bold Palettes", FL_COMMAND + 'b', (Fl_Callback *)bold_palettes_cb, this,
			FL_MENU_TOGGLE | (Config::bold_palettes() ? FL_MENU_VALUE : 0)),
		{},
		OS_SUBMENU("&Tools"),
		OS_MENU_ITEM("&Width...", FL_COMMAND + 'd', (Fl_Callback *)tilemap_width_cb, this, 0),
		OS_MENU_ITEM("Re&size...", FL_COMMAND + 'e', (Fl_Callback *)resize_cb, this, 0),
		OS_MENU_ITEM("Re&format...", FL_COMMAND + 'f', (Fl_Callback *)reformat_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Image to Tiles...", FL_COMMAND + 'x', (Fl_Callback *)image_to_tiles_cb, this, 0),
		{},
		OS_SUBMENU("&Help"),
		OS_MENU_ITEM("&Help", FL_F + 1, (Fl_Callback *)help_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&About", FL_COMMAND + '/', (Fl_Callback *)about_cb, this, 0),
		{},
		{}
	};
	_menu_bar->copy(menu_items);

	// Initialize menu bar items
	int first_recent_tilemap_i = _menu_bar->find_index((Fl_Callback *)open_recent_tilemap_cb);
	int first_recent_tileset_i = _menu_bar->find_index((Fl_Callback *)load_recent_tileset_cb);
	for (int i = 0; i < NUM_RECENT; i++) {
		_recent_tilemap_mis[i] = const_cast<Fl_Menu_Item *>(&_menu_bar->menu()[first_recent_tilemap_i + i]);
		_recent_tileset_mis[i] = const_cast<Fl_Menu_Item *>(&_menu_bar->menu()[first_recent_tileset_i + i]);
	}
#define TS_FIND_MENU_ITEM_CB(c) (const_cast<Fl_Menu_Item *>(_menu_bar->find_item((Fl_Callback *)(c))))
	_classic_theme_mi = TS_FIND_MENU_ITEM_CB(classic_theme_cb);
	_aero_theme_mi = TS_FIND_MENU_ITEM_CB(aero_theme_cb);
	_metro_theme_mi = TS_FIND_MENU_ITEM_CB(metro_theme_cb);
	_aqua_theme_mi = TS_FIND_MENU_ITEM_CB(aqua_theme_cb);
	_greybird_theme_mi = TS_FIND_MENU_ITEM_CB(greybird_theme_cb);
	_metal_theme_mi = TS_FIND_MENU_ITEM_CB(metal_theme_cb);
	_blue_theme_mi = TS_FIND_MENU_ITEM_CB(blue_theme_cb);
	_olive_theme_mi = TS_FIND_MENU_ITEM_CB(olive_theme_cb);
	_rose_gold_theme_mi = TS_FIND_MENU_ITEM_CB(rose_gold_theme_cb);
	_dark_theme_mi = TS_FIND_MENU_ITEM_CB(dark_theme_cb);
	_grid_mi = TS_FIND_MENU_ITEM_CB(grid_cb);
	_rainbow_tiles_mi = TS_FIND_MENU_ITEM_CB(rainbow_tiles_cb);
	_bold_palettes_mi = TS_FIND_MENU_ITEM_CB(bold_palettes_cb);
	// Conditional menu items
	_close_mi = TS_FIND_MENU_ITEM_CB(close_cb);
	_save_mi = TS_FIND_MENU_ITEM_CB(save_cb);
	_save_as_mi = TS_FIND_MENU_ITEM_CB(save_as_cb);
	_print_mi = TS_FIND_MENU_ITEM_CB(print_cb);
	_reload_tilesets_mi = TS_FIND_MENU_ITEM_CB(reload_tilesets_cb);
	_unload_tilesets_mi = TS_FIND_MENU_ITEM_CB(unload_tilesets_cb);
	_undo_mi = TS_FIND_MENU_ITEM_CB(undo_cb);
	_redo_mi = TS_FIND_MENU_ITEM_CB(redo_cb);
	_zoom_in_mi = TS_FIND_MENU_ITEM_CB(zoom_in_cb);
	_zoom_out_mi = TS_FIND_MENU_ITEM_CB(zoom_out_cb);
	_tilemap_width_mi = TS_FIND_MENU_ITEM_CB(tilemap_width_cb);
	_resize_mi = TS_FIND_MENU_ITEM_CB(resize_cb);
	_reformat_mi = TS_FIND_MENU_ITEM_CB(reformat_cb);
#undef TS_FIND_MENU_ITEM_CB

	for (int i = 0, md = 0; i < _menu_bar->size(); i++) {
		Fl_Menu_Item *mi = (Fl_Menu_Item *)&_menu_bar->menu()[i];
		if (md > 0 && mi && mi->label() && !mi->checkbox() && !mi->radio()) {
			Fl_Pixmap *icon = &BLANK_ICON;
			Fl_Multi_Label *ml = new Fl_Multi_Label();
			ml->typea = _FL_IMAGE_LABEL;
			ml->labela = (const char *)icon;
			ml->typeb = FL_NORMAL_LABEL;
			ml->labelb = mi->text;
			mi->image(icon);
			ml->label(mi);
		}
		if (mi->submenu()) { md++; }
		else if (!mi->label()) { md--; }
	}

	// Configure toolbar buttons

	_new_tb->tooltip("New Tilemap... (Ctrl+N)");
	_new_tb->callback((Fl_Callback *)new_cb, this);
	_new_tb->image(NEW_ICON);
	_new_tb->take_focus();

	_open_tb->tooltip("Open Tilemap... (Ctrl+O)");
	_open_tb->callback((Fl_Callback *)open_cb, this);
	_open_tb->image(OPEN_ICON);

	_save_tb->tooltip("Save Tilemap (Ctrl+S)");
	_save_tb->callback((Fl_Callback *)save_cb, this);
	_save_tb->image(SAVE_ICON);
	_save_tb->deimage(SAVE_DISABLED_ICON);

	_print_tb->tooltip("Print Tilemap (Ctrl+P)");
	_print_tb->callback((Fl_Callback *)print_cb, this);
	_print_tb->image(PRINT_ICON);
	_print_tb->deimage(PRINT_DISABLED_ICON);

	_load_tb->tooltip("Load Tileset... (Ctrl+T)");
	_load_tb->callback((Fl_Callback *)load_tileset_cb, this);
	_load_tb->image(LOAD_ICON);

	_add_tb->tooltip("Add Tileset... (Ctrl+A)");
	_add_tb->callback((Fl_Callback *)add_tileset_cb, this);
	_add_tb->image(ADD_ICON);

	_reload_tb->tooltip("Reload Tilesets (Ctrl+R)");
	_reload_tb->callback((Fl_Callback *)reload_tilesets_cb, this);
	_reload_tb->image(RELOAD_ICON);
	_reload_tb->deimage(RELOAD_DISABLED_ICON);

	_undo_tb->tooltip("Undo (Ctrl+Z)");
	_undo_tb->callback((Fl_Callback *)undo_cb, this);
	_undo_tb->image(UNDO_ICON);
	_undo_tb->deimage(UNDO_DISABLED_ICON);

	_redo_tb->tooltip("Redo (Ctrl+Y)");
	_redo_tb->callback((Fl_Callback *)redo_cb, this);
	_redo_tb->image(REDO_ICON);
	_redo_tb->deimage(REDO_DISABLED_ICON);

	_zoom_in_tb->tooltip("Zoom In (Ctrl+=)");
	_zoom_in_tb->callback((Fl_Callback *)zoom_in_cb, this);
	_zoom_in_tb->image(ZOOM_IN_ICON);
	_zoom_in_tb->deimage(ZOOM_IN_DISABLED_ICON);
	_zoom_in_tb->shortcut(FL_COMMAND + '+');

	_zoom_out_tb->tooltip("Zoom Out (Ctrl+-)");
	_zoom_out_tb->callback((Fl_Callback *)zoom_out_cb, this);
	_zoom_out_tb->image(ZOOM_OUT_ICON);
	_zoom_out_tb->deimage(ZOOM_OUT_DISABLED_ICON);

	_grid_tb->tooltip("Grid (Ctrl+G)");
	_grid_tb->callback((Fl_Callback *)grid_tb_cb, this);
	_grid_tb->image(GRID_ICON);
	_grid_tb->value(Config::grid());

	_rainbow_tiles_tb->tooltip("Rainbow Tiles (Ctrl+I)");
	_rainbow_tiles_tb->callback((Fl_Callback *)rainbow_tiles_tb_cb, this);
	_rainbow_tiles_tb->image(RAINBOW_ICON);
	_rainbow_tiles_tb->value(Config::rainbow_tiles());

	_bold_palettes_tb->tooltip("Bold Palettes (Ctrl+B)");
	_bold_palettes_tb->callback((Fl_Callback *)bold_palettes_tb_cb, this);
	_bold_palettes_tb->image(BOLD_ICON);
	_bold_palettes_tb->value(Config::bold_palettes());

	_width_heading->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);

	_tilemap_width->default_value(GAME_BOY_WIDTH);
	_tilemap_width->range(1, 1024);
	_tilemap_width->callback((Fl_Callback *)tilemap_width_tb_cb, this);

	_resize_tb->tooltip("Resize... (Ctrl+E)");
	_resize_tb->callback((Fl_Callback *)resize_cb, this);
	_resize_tb->image(RESIZE_ICON);
	_resize_tb->deimage(RESIZE_DISABLED_ICON);

	_reformat_tb->tooltip("Reformat... (Ctrl+F)");
	_reformat_tb->callback((Fl_Callback *)reformat_cb, this);
	_reformat_tb->image(REFORMAT_ICON);
	_reformat_tb->deimage(REFORMAT_DISABLED_ICON);

	_image_to_tiles_tb->tooltip("Image to Tiles... (Ctrl+X)");
	_image_to_tiles_tb->callback((Fl_Callback *)image_to_tiles_cb, this);
	_image_to_tiles_tb->image(INPUT_ICON);

	_current_tile->attributes(false);

	_x_flip_tb->tooltip("X Flip (Horizontal)");
	_x_flip_tb->image(X_FLIP_ICON);
	_x_flip_tb->deimage(X_FLIP_DISABLED_ICON);
	_x_flip_tb->callback((Fl_Callback *)x_flip_cb, this);

	_y_flip_tb->tooltip("Y Flip (Vertical)");
	_y_flip_tb->image(Y_FLIP_ICON);
	_y_flip_tb->deimage(Y_FLIP_DISABLED_ICON);
	_y_flip_tb->callback((Fl_Callback *)y_flip_cb, this);

	_current_attributes->attributes(true);

	_priority_tb->tooltip("Priority");
	_priority_tb->image(PRIORITY_ICON);
	_priority_tb->deimage(PRIORITY_DISABLED_ICON);
	_priority_tb->callback((Fl_Callback *)priority_cb, this);

	_obp1_tb->tooltip("OBP1");
	_obp1_tb->image(OBP1_ICON);
	_obp1_tb->deimage(OBP1_DISABLED_ICON);
	_obp1_tb->callback((Fl_Callback *)obp1_cb, this);

	_transparency->color(OS_TAB_COLOR);
	int default_alpha = Preferences::get("alpha", 4);
	_transparency->range(1, 9);
	_transparency->step(1);
	double alpha_steps = floor((_transparency->maximum() - _transparency->minimum()) / _transparency->step()) + 1;
	_transparency->slider_size(1.0 / alpha_steps);
	_transparency->default_value(_transparency->clamp(default_alpha));
	_transparency->callback((Fl_Callback *)transparency_cb, this);
	_transparency->do_callback();

	// Configure containers

	select_tile(0x000);
	select_palette(0);

	_left_tabs->callback((Fl_Callback *)change_tab_cb, this);

	_palettes_tab->size_range(_palettes_tab->w(), _palettes_tab->h(), _palettes_tab->w(), _palettes_tab->h());

	_top_group->size_range(_top_group->w(), _top_group->h(), _top_group->w(), _top_group->h());

	// Configure dialogs

	_tilemap_open_chooser->title("Open Tilemap");
	_tilemap_open_chooser->filter("Tilemap Files\t*.{tilemap,rle,bin,map,raw,kmp}\n");

	_tilemap_save_chooser->title("Save Tilemap");
	_tilemap_save_chooser->filter("Tilemap Files\t*.{tilemap,rle,bin,map,raw,kmp}\n");
	_tilemap_save_chooser->preset_file("NewTilemap.tilemap");
	_tilemap_save_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);

	_tileset_load_chooser->title("Open Tileset");
	_tileset_load_chooser->filter("Tileset Files\t*.{png,bmp,1bpp,2bpp,4bpp,1bpp.lz,2bpp.lz}\n");

	_png_chooser->title("Print Screenshot");
	_png_chooser->filter("PNG Files\t*.png\n");
	_png_chooser->preset_file("screenshot.png");
	_png_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);

	_error_dialog->width_range(280, 700);
	_warning_dialog->width_range(280, 700);
	_success_dialog->width_range(280, 700);
	_unsaved_dialog->width_range(280, 700);

	_new_tilemap_dialog->tilemap_width(GAME_BOY_WIDTH);
	_new_tilemap_dialog->tilemap_height(GAME_BOY_HEIGHT);

	std::string subject(PROGRAM_NAME " " PROGRAM_VERSION_STRING), message(
		"Copyright \xc2\xa9 " CURRENT_YEAR " " PROGRAM_AUTHOR ".\n"
		"\n"
		"Source code is available at:\n"
		"https://github.com/Rangi42/tilemap-studio\n"
		"\n"
		"Some icons by Yusuke Kamiyamane."
	);
	_about_dialog->subject(subject);
	_about_dialog->message(message);
	_about_dialog->width_range(280, 700);

	_help_window->content(
#include "help.html" // a C++11 raw string literal
		);

	update_zoom();
	update_recent_tilemaps();
	update_recent_tilesets();
	update_tilemap_metadata();
	update_tileset_metadata();
	update_active_controls();
}

Main_Window::~Main_Window() {
	delete _menu_bar; // includes menu items
	delete _status_bar; // includes status bar fields
	delete _main_group; // includes map and blocks
	delete _tilemap_dnd_receiver;
	delete _tileset_dnd_receiver;
	delete _tilemap_open_chooser;
	delete _tilemap_save_chooser;
	delete _tileset_load_chooser;
	delete _png_chooser;
	delete _error_dialog;
	delete _warning_dialog;
	delete _success_dialog;
	delete _unsaved_dialog;
	delete _about_dialog;
	delete _resize_dialog;
	delete _reformat_dialog;
	delete _image_to_tiles_dialog;
	delete _help_window;
}

void Main_Window::show() {
	Fl_Double_Window::show();
#ifdef _WIN32
	// Fix for 16x16 icon from <http://www.fltk.org/str.php?L925>
	HWND hwnd = fl_xid(this);
	HANDLE big_icon = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON,
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CXICON), 0);
	SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(big_icon));
	HANDLE small_icon = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CXSMICON), 0);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small_icon));
#else
	// Fix for X11 icon alpha mask <https://www.mail-archive.com/fltk@easysw.com/msg02863.html>
	XWMHints *hints = XGetWMHints(fl_display, fl_xid(this));
	hints->flags |= IconMaskHint;
	hints->icon_mask = _icon_mask;
	XSetWMHints(fl_display, fl_xid(this), hints);
	XFree(hints);
#endif
}

int Main_Window::handle(int event) {
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;
	default:
		return Fl_Double_Window::handle(event);
	}
}

void Main_Window::store_recent_tilemap() {
	std::string last(_tilemap_file);
	for (int i = 0; i < NUM_RECENT; i++) {
		if (_recent_tilemaps[i] == _tilemap_file) {
			_recent_tilemaps[i] = last;
			break;
		}
		std::swap(last, _recent_tilemaps[i]);
	}
	update_recent_tilemaps();
}

void Main_Window::update_recent_tilemaps() {
	int last = -1;
	for (int i = 0; i < NUM_RECENT; i++) {
		Fl_Multi_Label *ml = (Fl_Multi_Label *)_recent_tilemap_mis[i]->label();
		if (ml->labelb[0]) {
			delete ml->labelb;
			ml->labelb = "";
		}
		if (_recent_tilemaps[i].empty()) {
			_recent_tilemap_mis[i]->hide();
		}
		else {
			const char *basename = fl_filename_name(_recent_tilemaps[i].c_str());
			char *label = new char[FL_PATH_MAX]();
			strcpy(label, OS_MENU_ITEM_PREFIX);
			strcat(label, basename);
			strcat(label, OS_MENU_ITEM_SUFFIX);
			ml->labelb = label;
			_recent_tilemap_mis[i]->show();
			last = i;
		}
		_recent_tilemap_mis[i]->flags &= ~FL_MENU_DIVIDER;
	}
	_recent_tilemap_mis[last]->flags |= FL_MENU_DIVIDER;
}

void Main_Window::store_recent_tileset() {
	const std::string &tileset_file = _tileset_files.back();
	std::string last(tileset_file);
	for (int i = 0; i < NUM_RECENT; i++) {
		if (_recent_tilesets[i] == tileset_file) {
			_recent_tilesets[i] = last;
			break;
		}
		std::swap(last, _recent_tilesets[i]);
	}
	update_recent_tilesets();
}

void Main_Window::update_recent_tilesets() {
	int last = -1;
	for (int i = 0; i < NUM_RECENT; i++) {
		Fl_Multi_Label *ml = (Fl_Multi_Label *)_recent_tileset_mis[i]->label();
		if (ml->labelb && ml->labelb[0]) {
			delete ml->labelb;
			ml->labelb = "";
		}
		if (_recent_tilesets[i].empty()) {
			_recent_tileset_mis[i]->hide();
		}
		else {
			const char *basename = fl_filename_name(_recent_tilesets[i].c_str());
			char *label = new char[FL_PATH_MAX]();
			strcpy(label, OS_MENU_ITEM_PREFIX);
			strcat(label, basename);
			strcat(label, OS_MENU_ITEM_SUFFIX);
			ml->labelb = label;
			_recent_tileset_mis[i]->show();
			last = i;
		}
		_recent_tileset_mis[i]->flags &= ~FL_MENU_DIVIDER;
	}
	_recent_tileset_mis[last]->flags |= FL_MENU_DIVIDER;
}

void Main_Window::update_zoom() {
	char buffer[64] = {};
	sprintf(buffer, "Zoom: %dx", Config::zoom());
	_zoom_level->copy_label(buffer);
	if (Config::zoom() <= MIN_ZOOM) {
		_zoom_out_mi->deactivate();
		_zoom_out_tb->deactivate();
	}
	else {
		_zoom_out_mi->activate();
		_zoom_out_tb->activate();
	}
	if (Config::zoom() >= MAX_ZOOM) {
		_zoom_in_mi->deactivate();
		_zoom_in_tb->deactivate();
	}
	else {
		_zoom_in_mi->activate();
		_zoom_in_tb->activate();
	}
	tilemap_width_tb_cb(NULL, this);
}

void Main_Window::update_status(Tile_Tessera *tt) {
	if (!_tilemap.size()) {
		_tilemap_dimensions->label("");
		_hover_id->label("");
		_hover_xy->label("");
		_hover_landmark->label("");
		_status_bar->redraw();
		return;
	}
	char buffer[64] = {};
#ifdef __GNUC__
	sprintf(buffer, "Tilemap: %zu x %zu", _tilemap.width(), _tilemap.height());
#else
	sprintf(buffer, "Tilemap: %u x %u", (uint32_t)_tilemap.width(), (uint32_t)_tilemap.height());
#endif
	_tilemap_dimensions->copy_label(buffer);
	if (!tt) {
		_hover_id->label("");
		_hover_xy->label("");
		_hover_landmark->label("");
		_status_bar->redraw();
		return;
	}
	int bank = (int)(tt->id() >> 8), offset = (int)(tt->id() & 0xFF);
	sprintf(buffer, "ID: $%d:%02X", bank, offset);
	_hover_id->copy_label(buffer);
#ifdef __GNUC__
	sprintf(buffer, "X/Y (%zu, %zu)", tt->col(), tt->row());
#else
	sprintf(buffer, "X/Y (%u, %u)", (uint32_t)tt->col(), (uint32_t)tt->row());
#endif
	_hover_xy->copy_label(buffer);
	if (_tilemap.width() == GAME_BOY_WIDTH && _tilemap.height() == GAME_BOY_HEIGHT) {
		if (format_has_landmarks(Config::format())) {
			size_t lx = tt->col() * TILE_SIZE + TILE_SIZE / 2;
			size_t ly = tt->row() * TILE_SIZE + TILE_SIZE / 2;
#ifdef __GNUC__
			sprintf(buffer, "Landmark (%zu, %zu)", lx, ly);
#else
			sprintf(buffer, "Landmark (%u, %u)", (uint32_t)lx, (uint32_t)ly);
#endif
			_hover_landmark->copy_label(buffer);
		}
		else if (format_has_emaps(Config::format()) && tt->col() >= 2 && tt->col() <= 0xF + 2 &&
			tt->row() >= 1 && tt->row() <= 0xF + 1) {
			int lx = (int)tt->col() - 2, ly = (int)tt->row() - 1;
			sprintf(buffer, "EMAP $%X, $%X", lx, ly);
			_hover_landmark->copy_label(buffer);
		}
		else {
			_hover_landmark->label("");
		}
	}
	else {
		_hover_landmark->label("");
	}
}

void Main_Window::update_tilemap_metadata() {
	if (_tilemap.size()) {
		if (_tilemap_file.empty()) {
			_tilemap_name->label(NEW_TILEMAP_NAME);
		}
		else {
			const char *basename = fl_filename_name(_tilemap_file.c_str());
			if (_attrmap_file.size()) {
				char buffer[FL_PATH_MAX] = {};
				strcpy(buffer, basename);
				strcat(buffer, " / ");
				basename = fl_filename_name(_attrmap_file.c_str());
				strcat(buffer, basename);
				_tilemap_name->copy_label(buffer);
			}
			else {
				_tilemap_name->label(basename);
			}
		}
		const char *name = format_name(Config::format());
		_tilemap_format->label(name);
	}
	else {
		_tilemap_name->label(NO_FILE_SELECTED_LABEL);
		_tilemap_format->label("");
	}
}

void Main_Window::update_tileset_metadata() {
	if (!_tilesets.empty()) {
		if (_tilesets.size() == 1) {
			std::string &f = _tileset_files.front();
			const char *basename = fl_filename_name(f.c_str());
			_tileset_name->label(basename);
		}
		else {
			char buffer[FL_PATH_MAX] = {};
#ifdef __GNUC__
			sprintf(buffer, "%zu files", _tilesets.size());
#else
			sprintf(buffer, "%u files", _tilesets.size());
#endif
			_tileset_name->copy_label(buffer);
		}
	}
	else {
		_tileset_name->label(NO_FILES_SELECTED_LABEL);
	}
}

void Main_Window::update_active_controls() {
	if (_tilemap.size()) {
		_close_mi->activate();
		_save_mi->activate();
		_save_tb->activate();
		_save_as_mi->activate();
		_print_mi->activate();
		_print_tb->activate();
		if (_tilemap.can_undo()) {
			_undo_mi->activate();
			_undo_tb->activate();
		}
		else {
			_undo_mi->deactivate();
			_undo_tb->deactivate();
		}
		if (_tilemap.can_redo()) {
			_redo_mi->activate();
			_redo_tb->activate();
		}
		else {
			_redo_mi->deactivate();
			_redo_tb->deactivate();
		}
		_tilemap_width_mi->activate();
		_width_heading->activate();
		_tilemap_width->activate();
		_resize_mi->activate();
		_resize_tb->activate();
		_reformat_mi->activate();
		_reformat_tb->activate();
	}
	else {
		_close_mi->deactivate();
		_save_mi->deactivate();
		_save_tb->deactivate();
		_save_as_mi->deactivate();
		_print_mi->deactivate();
		_print_tb->deactivate();
		_undo_mi->deactivate();
		_undo_tb->deactivate();
		_redo_mi->deactivate();
		_redo_tb->deactivate();
		_tilemap_width_mi->deactivate();
		_width_heading->deactivate();
		_tilemap_width->deactivate();
		_resize_mi->deactivate();
		_resize_tb->deactivate();
		_reformat_mi->deactivate();
		_reformat_tb->deactivate();
	}

	if (!_tilesets.empty()) {
		_reload_tilesets_mi->activate();
		_reload_tb->activate();
		_unload_tilesets_mi->activate();
	}
	else {
		_reload_tilesets_mi->deactivate();
		_reload_tb->deactivate();
		_unload_tilesets_mi->deactivate();
	}

	if (format_can_flip(Config::format())) {
		_x_flip_tb->activate();
		_y_flip_tb->activate();
	}
	else {
		_x_flip_tb->clear();
		_x_flip_tb->deactivate();
		_x_flip_tb->do_callback();
		_y_flip_tb->clear();
		_y_flip_tb->deactivate();
		_y_flip_tb->do_callback();
	}

	if (format_has_metadata(Config::format())) {
		_priority_tb->activate();
		_obp1_tb->activate();
	}
	else {
		_priority_tb->clear();
		_priority_tb->deactivate();
		_priority_tb->do_callback();
		_obp1_tb->clear();
		_obp1_tb->deactivate();
		_obp1_tb->do_callback();
	}

	int n = format_tileset_size(Config::format());
	while (_tiles_scroll->children()) {
		_tiles_scroll->remove(0);
	}
	for (int i = 0; i < n; i++) {
		_tiles_scroll->add(_tile_buttons[i]);
	}
	_tiles_scroll->add(_tiles_scroll->scrollbar);
	_tiles_scroll->add(_tiles_scroll->hscrollbar);
	_tiles_scroll->init_sizes();
	int tw = TILES_PER_ROW * TILE_SIZE_2X, max_th = ((n + TILES_PER_ROW - 1) / TILES_PER_ROW) * TILE_SIZE_2X;
	_tiles_scroll->contents(tw, max_th);
	if (!Config::show_attributes() && _selected_tile->id() >= n) {
		select_tile(0x000);
		_tiles_scroll->scroll_to(0, 0);
	}

	_left_group->init_sizes();
	int min_th = (MAX_NUM_PALETTES / PALETTES_PER_ROW) * TILE_SIZE_2X;
	if (Config::show_attributes()) {
		max_th = min_th;
	}
	else {
		min_th = tw;
	}
	int tdw = 12 + Fl::scrollbar_size(), tdh = 12 + OS_TAB_HEIGHT;
	_left_tabs->size_range(tw + tdw, min_th + tdh, tw + tdw, max_th + tdh);
	int tdy = _tilemap_scroll->y() - _left_tabs->y();
	_left_tabs->resize(_left_tabs->x(), _left_tabs->y(), _left_tabs->w(), _tilemap_scroll->h() + tdy);

	if (format_has_palettes(Config::format())) {
		_palettes_tab->activate();
		int m = format_palettes_size(Config::format());
		for (int i = 0; i < MAX_NUM_PALETTES; i++) {
			_palettes_pane->remove(0);
		}
		for (int i = 0; i < m; i++) {
			_palettes_pane->add(_palette_buttons[i]);
		}
		if (Config::show_attributes() && _selected_palette->palette() >= m) {
			select_palette(0);
		}
	}
	else {
		_palettes_tab->deactivate();
		if (_left_tabs->value() == _palettes_tab) {
			_left_tabs->value(_tiles_tab);
			_left_tabs->do_callback();
		}
	}

	if (Config::show_attributes()) {
		_tile_heading->hide();
		_current_tile->hide();
		_x_flip_tb->hide();
		_y_flip_tb->hide();
		_current_attributes->show();
		_priority_tb->show();
		_obp1_tb->show();
	}
	else {
		_tile_heading->show();
		_current_tile->show();
		_x_flip_tb->show();
		_y_flip_tb->show();
		_current_attributes->hide();
		_priority_tb->hide();
		_obp1_tb->hide();
	}
}

void Main_Window::resize_tilemap() {
	size_t w = _resize_dialog->tilemap_width(), h = _resize_dialog->tilemap_height();
	size_t n = w * h;
	if (_tilemap.size() == n) { return; }

	_tilemap.resize(w, h, _resize_dialog->horizontal_anchor(), _resize_dialog->vertical_anchor());

	while (_tilemap_scroll->children() > 2) { // keep scrollbars
		_tilemap_scroll->remove(0);
	}
	for (size_t i = 0; i < n; i++) {
		Tile_Tessera *tt = _tilemap.tile(i);
		tt->callback((Fl_Callback *)change_tile_cb, this);
		_tilemap_scroll->add(tt);
	}

	_tilemap_width->default_value(w);
	tilemap_width_tb_cb(NULL, this);
	update_status(NULL);
	update_active_controls();
	redraw();
}

void Main_Window::reformat_tilemap() {
	Tilemap_Format fmt = _reformat_dialog->format();
	if (Config::format() == fmt) { return; }

	const char *filename = _tilemap_file.c_str();
	const char *basename = fl_filename_name(filename);

	if (!_reformat_dialog->force() && !_tilemap.can_format_as(fmt)) {
		std::string msg = "Cannot reformat ";
		msg = msg + basename + " \nas " + format_name(fmt) + "!";
		_error_dialog->message(msg);
		_error_dialog->show(this);
		return;
	}

	Config::format(fmt);
	_tilemap.limit_to_format(fmt);
	_tilemap.modified(true);

	_tiles_scroll->scroll_to(0, 0);

	update_tilemap_metadata();
	update_active_controls();
	redraw();

	std::string msg = "Reformatted ";
	msg = msg + basename + "!";
	_success_dialog->message(msg);
	_success_dialog->show(this);
}

void Main_Window::edit_tile(Tile_Tessera *tt) {
	Tile_State fs = tt->state();
	Tile_State ts(tile_id(), x_flip(), y_flip(), priority(), obp1(), palette());
	bool a = Config::show_attributes();
	if (fs.same(ts, a)) { return; }
	tt->assign(ts, a);
	tt->damage(1);
	_tilemap.modified(true);
}

void Main_Window::flood_fill(Tile_Tessera *tt) {
	Tile_State fs = tt->state();
	Tile_State ts(tile_id(), x_flip(), y_flip(), priority(), obp1(), palette());
	bool a = Config::show_attributes();
	if (fs.same(ts, a)) { return; }
	std::queue<size_t> queue;
	size_t w = _tilemap.width(), h = _tilemap.height(), n = _tilemap.size();
	size_t row = tt->row(), col = tt->col();
	size_t i = row * w + col;
	queue.push(i);
	while (!queue.empty()) {
		size_t j = queue.front();
		queue.pop();
		if (j >= n) { continue; }
		Tile_Tessera *ff = _tilemap.tile(j);
		if (!ff->state().same(fs, a)) { continue; }
		ff->assign(ts, a); // fill
		size_t r = ff->row(), c = ff->col();
		if (c > 0) { queue.push(j-1); } // left
		if (c < w - 1) { queue.push(j+1); } // right
		if (r > 0) { queue.push(j-w); } // up
		if (r < h - 1) { queue.push(j+w); } // down
	}
	_tilemap_scroll->redraw();
	_tilemap.modified(true);
}

void Main_Window::substitute_tile(Tile_Tessera *tt) {
	Tile_State fs = tt->state();
	Tile_State ts(tile_id(), x_flip(), y_flip(), priority(), obp1(), palette());
	bool a = Config::show_attributes();
	size_t n = _tilemap.size();
	for (size_t i = 0; i < n; i++) {
		Tile_Tessera *ff = _tilemap.tile(i);
		if (ff->state().same(fs, a)) {
			ff->assign(ts, a);
		}
	}
	_tilemap_scroll->redraw();
	_tilemap.modified(true);
}

void Main_Window::swap_tiles(Tile_Tessera *tt) {
	Tile_State fs = tt->state();
	Tile_State ts(tile_id(), x_flip(), y_flip(), priority(), obp1(), palette());
	bool a = Config::show_attributes();
	if (fs.same(ts, a)) { return; }
	size_t n = _tilemap.size();
	for (size_t i = 0; i < n; i++) {
		Tile_Tessera *ff = _tilemap.tile(i);
		if (ff->state().same(fs, a)) {
			ff->assign(ts, a);
		}
		else if (ff->state().same(ts, a)) {
			ff->assign(fs, a);
		}
	}
	_tilemap_scroll->redraw();
	_tilemap.modified(true);
}

void Main_Window::open_tilemap(const char *filename, size_t width, size_t height) {
	if (filename) {
		_tilemap_options_dialog->use_tilemap(filename);
		_tilemap_options_dialog->show(this);
		if (_tilemap_options_dialog->canceled()) { return; }
	}

	_tilemap.modified(false);
	close_cb(NULL, this);

	const char *basename;

	if (filename) {
		const char *attrmap_filename = _tilemap_options_dialog->attrmap_filename();

		_tilemap_file = filename;
		_attrmap_file = attrmap_filename ? attrmap_filename : "";
		basename = fl_filename_name(filename);
		const char *attrmap_basename = fl_filename_name(attrmap_filename);

		Config::format(_tilemap_options_dialog->format());

		Tilemap::Result r = _tilemap.read_tiles(filename, attrmap_filename);
		if (r) {
			_tilemap.clear();
			std::string msg = "Error reading ";
			msg = msg + (r >= Tilemap::ATTRMAP_BAD_FILE ? attrmap_basename : basename);
			msg = msg + "!\n\n" + Tilemap::error_message(r);
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return;
		}

		if (width) {
			_tilemap.width(width);
		}
		else {
			_tilemap.guess_width();
		}
	}
	else {
		_tilemap_file = "";
		_attrmap_file = "";
		basename = NEW_TILEMAP_NAME;

		Config::format(_new_tilemap_dialog->format());

		_tilemap.new_tiles(width, height);
	}

	size_t n = _tilemap.size();
	for (size_t i = 0; i < n; i++) {
		Tile_Tessera *tt = _tilemap.tile(i);
		tt->callback((Fl_Callback *)change_tile_cb, this);
		_tilemap_scroll->add(tt);
	}

	_tilemap_width->default_value(_tilemap.width());
	tilemap_width_tb_cb(NULL, this);

	// set filenames
	char buffer[FL_PATH_MAX] = {};
	sprintf(buffer, PROGRAM_NAME " - %s", basename);
	copy_label(buffer);
	sprintf(buffer, "%s", basename);
	fl_filename_setext(buffer, sizeof(buffer), ".png");
	_png_chooser->preset_file(buffer);

	store_recent_tilemap();
	update_tilemap_metadata();
	update_status(NULL);
	update_active_controls();
	redraw();
}

void Main_Window::open_recent_tilemap(int n) {
	if (n < 0 || n >= NUM_RECENT || _recent_tilemaps[n].empty()) {
		return;
	}

	if (unsaved()) {
		std::string msg = modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Open another tilemap anyway?";
		_unsaved_dialog->message(msg);
		_unsaved_dialog->show(this);
		if (_unsaved_dialog->canceled()) { return; }
	}

	const char *filename = _recent_tilemaps[n].c_str();
	_tilemap.modified(false);
	open_tilemap(filename);
}

bool Main_Window::save_tilemap(bool force) {
	const char *filename = _tilemap_file.c_str();
	const char *attrmap_filename = _attrmap_file.c_str();
	const char *basename = fl_filename_name(filename);

	if (_tilemap.modified() || force) {
		if (!_tilemap.write_tiles(filename, attrmap_filename)) {
			std::string msg = "Could not write to ";
			msg = msg + basename + "!";
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return false;
		}
	}
	_tilemap.modified(false);

	char buffer[FL_PATH_MAX] = {};
	sprintf(buffer, PROGRAM_NAME " - %s", basename);
	copy_label(buffer);
	update_tilemap_metadata();

	strcpy(buffer, basename);
	fl_filename_setext(buffer, FL_PATH_MAX, ".png");
	_png_chooser->preset_file(buffer);

	if (force) {
		store_recent_tilemap();
	}

	std::string msg = "Saved ";
	msg = msg + basename + "!";
	_success_dialog->message(msg);
	_success_dialog->show(this);

	return true;
}

void Main_Window::add_tileset(const char *filename, int start, int offset, int length) {
	const char *basename = fl_filename_name(filename);
	Tileset tileset(start, offset, length);
	Tileset::Result result = tileset.read_tiles(filename);
	if (result) {
		std::string msg = "Error reading ";
		msg = msg + basename + "!\n\n" + Tileset::error_message(result);
		_error_dialog->message(msg);
		_error_dialog->show(this);
		tileset.clear();
		return;
	}
	_tilesets.push_back(tileset);
	_tileset_files.push_back(filename);
	store_recent_tileset();
	update_tileset_metadata();
	update_active_controls();
	redraw();
}

void Main_Window::load_recent_tileset(int n) {
	if (n < 0 || n >= NUM_RECENT || _recent_tilesets[n].empty()) {
		return;
	}

	if (_tilesets.size() > 1) {
		std::string msg = "You have added ";
		msg = msg + std::to_string(_tilesets.size()) + " tilesets!\n\n"
			"Load another tileset anyway?";
		_unsaved_dialog->message(msg);
		_unsaved_dialog->show(this);
		if (_unsaved_dialog->canceled()) { return; }
	}

	const char *filename = _recent_tilesets[n].c_str();
	load_tileset(filename);
}

void Main_Window::select_tile(uint16_t id) {
	if (_selected_tile) {
		_selected_tile->clear();
	}
	_selected_tile = _tile_buttons[id];
	_selected_tile->setonly();
	_current_tile->id(id);

	char buffer[16] = {};
	int index = (int)id;
	int bank = index >> 8, offset = index & 0xFF;
	sprintf(buffer, "Tile: $%d:%02X", bank, offset);
	_tile_heading->copy_label(buffer);

	int ds = (index / TILES_PER_ROW) * TILE_SIZE_2X;
	if (ds >= _tiles_scroll->yposition() + _tiles_scroll->h() - TILE_SIZE_2X / 2) {
		_tiles_scroll->scroll_to(0, ds + TILE_SIZE_2X - _tiles_scroll->h() + Fl::box_dh(_tiles_scroll->box()));
	}
	else if (ds + TILE_SIZE_2X <= _tiles_scroll->yposition() + TILE_SIZE_2X / 2) {
		_tiles_scroll->scroll_to(0, ds);
	}

	_current_tile->redraw();
	_tile_heading->redraw();
	_tiles_tab->redraw();
}

void Main_Window::highlight_tile(uint16_t id) {
	Config::highlight_id(Config::highlight_id() != id ? id : (uint16_t)-1);
	_tilemap_scroll->redraw();
	_tiles_tab->redraw();
}

void Main_Window::select_palette(int palette) {
	if (_selected_palette) {
		_selected_palette->clear();
	}
	_selected_palette = _palette_buttons[palette];
	_selected_palette->setonly();
	_current_attributes->palette(palette);

	_current_attributes->redraw();
	_palettes_tab->redraw();
}

void Main_Window::drag_and_drop_tilemap_cb(DnD_Receiver *dndr, Main_Window *mw) {
	Fl_Window *top = Fl::modal();
	if (top && top != mw) { return; }
	std::string filename = dndr->text().substr(0, dndr->text().find('\n'));
	mw->open_tilemap(filename.c_str());
}

void Main_Window::drag_and_drop_tileset_cb(DnD_Receiver *dndr, Main_Window *mw) {
	Fl_Window *top = Fl::modal();
	if (top && top != mw) { return; }
	std::string filename = dndr->text().substr(0, dndr->text().find('\n'));
	mw->load_tileset(filename.c_str());
}

void Main_Window::new_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Create a new tilemap anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	mw->_new_tilemap_dialog->format(Config::format());
	mw->_new_tilemap_dialog->show(mw);
	if (mw->_new_tilemap_dialog->canceled()) { return; }

	mw->_tilemap.modified(false);
	mw->new_tilemap(mw->_new_tilemap_dialog->tilemap_width(), mw->_new_tilemap_dialog->tilemap_height());
}

void Main_Window::open_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Open another tilemap anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	int status = mw->_tilemap_open_chooser->show();
	if (status == 1) { return; }

	const char *filename = mw->_tilemap_open_chooser->filename();
	const char *basename = fl_filename_name(filename);
	if (status == -1) {
		std::string msg = "Could not open ";
		msg = msg + basename + "!\n\n" + mw->_tilemap_open_chooser->errmsg();
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	mw->open_tilemap(filename);
}

void Main_Window::open_recent_tilemap_cb(Fl_Menu_ *m, Main_Window *mw) {
	int first_recent_i = m->find_index((Fl_Callback *)open_recent_tilemap_cb);
	int i = m->find_index(m->mvalue()) - first_recent_i;
	mw->open_recent_tilemap(i);
}

void Main_Window::clear_recent_tilemaps_cb(Fl_Menu_ *, Main_Window *mw) {
	for (int i = 0; i < NUM_RECENT; i++) {
		mw->_recent_tilemaps[i].clear();
		mw->_recent_tilemap_mis[i]->hide();
	}
}

void Main_Window::load_recent_tileset_cb(Fl_Menu_ *m, Main_Window *mw) {
	int first_recent_i = m->find_index((Fl_Callback *)load_recent_tileset_cb);
	int i = m->find_index(m->mvalue()) - first_recent_i;
	mw->load_recent_tileset(i);
}

void Main_Window::clear_recent_tilesets_cb(Fl_Menu_ *, Main_Window *mw) {
	for (int i = 0; i < NUM_RECENT; i++) {
		mw->_recent_tilesets[i].clear();
		mw->_recent_tileset_mis[i]->hide();
	}
}

void Main_Window::close_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }

	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Close it anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	mw->label(PROGRAM_NAME);
	mw->_tilemap.clear();
	mw->_tilemap_scroll->clear();
	mw->_tilemap_scroll->scroll_to(0, 0);
	mw->_tilemap_scroll->contents(0, 0);
	mw->_tiles_scroll->scroll_to(0, 0);
	mw->init_sizes();
	mw->_tilemap_file.clear();
	mw->_attrmap_file.clear();
	mw->update_tilemap_metadata();
	mw->update_status(NULL);
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::save_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	if (mw->_tilemap_file.empty()) {
		save_as_cb(NULL, mw);
	}
	else {
		mw->save_tilemap(false);
	}
}

void Main_Window::save_as_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }

	int status = mw->_tilemap_save_chooser->show();
	if (status == 1) { return; }

	Tilemap_Format fmt = Config::format();

	char filename[FL_PATH_MAX] = {};
	const char *ext = format_extension(fmt);
	add_dot_ext(mw->_tilemap_save_chooser->filename(), ext, filename);
	const char *basename = fl_filename_name(filename);

	if (status == -1) {
		std::string msg = "Could not open ";
		msg = msg + basename + "!\n\n" + mw->_tilemap_save_chooser->errmsg();
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	mw->_tilemap_file = filename ? filename : "";
	mw->save_tilemap(true);
}

void Main_Window::load_tileset_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->_tilesets.size() > 1) {
		std::string msg = "You have added ";
		msg = msg + std::to_string(mw->_tilesets.size()) + " tilesets!\n\n"
			"Load another tileset anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	int status = mw->_tileset_load_chooser->show();
	if (status == 1) { return; }

	const char *filename = mw->_tileset_load_chooser->filename();
	const char *basename = fl_filename_name(filename);
	if (status == -1) {
		std::string msg = "Could not open ";
		msg = msg + basename + "!\n\n" + mw->_tileset_load_chooser->errmsg();
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	mw->load_tileset(filename);
}

void Main_Window::add_tileset_cb(Fl_Widget *, Main_Window *mw) {
	int status = mw->_tileset_load_chooser->show();
	if (status == 1) { return; }

	const char *filename = mw->_tileset_load_chooser->filename();
	const char *basename = fl_filename_name(filename);
	if (status == -1) {
		std::string msg = "Could not open ";
		msg = msg + basename + "!\n\n" + mw->_tileset_load_chooser->errmsg();
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	mw->_add_tileset_dialog->limit_tileset_options(filename);
	mw->_add_tileset_dialog->start_id(mw->tile_id());
	mw->_add_tileset_dialog->show(mw);
	if (mw->_add_tileset_dialog->canceled()) { return; }
	int start = mw->_add_tileset_dialog->start_id();
	int offset = mw->_add_tileset_dialog->offset();
	int length = mw->_add_tileset_dialog->length();
	mw->add_tileset(filename, start, offset, length);
}

void Main_Window::reload_tilesets_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->_tilesets.empty()) { return; }

	std::vector<Tileset> tilesets(mw->_tilesets);
	std::vector<std::string> tileset_files(mw->_tileset_files);
	mw->_tilesets.clear();
	mw->_tileset_files.clear();
	size_t n = tilesets.size();
	for (size_t i = 0; i < n; i++) {
		const char *filename = tileset_files[i].c_str();
		Tileset &t = tilesets[i];
		mw->add_tileset(filename, t.start_id(), t.offset(), t.length());
	}
}

void Main_Window::unload_tilesets_cb(Fl_Widget *w, Main_Window *mw) {
	if (mw->_tilesets.empty()) { return; }

	if (w && mw->_tilesets.size() > 1) {
		std::string msg = "You have added ";
		msg = msg + std::to_string(mw->_tilesets.size()) + " tilesets!\n\n"
			"Unload them anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	for (Tileset &t : mw->_tilesets) {
		t.clear();
	}
	mw->_tilesets.clear();
	mw->_tileset_files.clear();
	mw->update_tileset_metadata();
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::print_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }

	int status = mw->_png_chooser->show();
	if (status == 1) { return; }

	char filename[FL_PATH_MAX] = {};
	add_dot_ext(mw->_png_chooser->filename(), ".png", filename);
	const char *basename = fl_filename_name(filename);

	if (status == -1) {
		std::string msg = "Could not print to ";
		msg = msg + basename + "!\n\n" + mw->_png_chooser->errmsg();
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	Fl_RGB_Image *img = mw->_tilemap.print_tilemap();
	Image::Result result = Image::write_image(filename, img);
	delete img;
	if (result) {
		std::string msg = "Could not print to ";
		msg = msg + basename + "!\n\n" + Image::error_message(result);
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
	}
	else {
		std::string msg = "Printed ";
		msg = msg + basename + "!";
		mw->_success_dialog->message(msg);
		mw->_success_dialog->show(mw);
	}
}

void Main_Window::exit_cb(Fl_Widget *, Main_Window *mw) {
	// Override default behavior of Esc to close main window
	if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) { return; }

	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Exit anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	// Save global config
	Preferences::set("theme", OS::current_theme());
	Preferences::set("x", mw->x());
	Preferences::set("y", mw->y());
	Preferences::set("w", mw->w());
	Preferences::set("h", mw->h());
	Preferences::set("format", Config::format());
	Preferences::set("zoom", Config::zoom());
	Preferences::set("grid", Config::grid());
	Preferences::set("rainbow", Config::rainbow_tiles());
	Preferences::set("bold", Config::bold_palettes());
	Preferences::set("alpha", (int)mw->_transparency->value());
	for (int i = 0; i < NUM_RECENT; i++) {
		Preferences::set_string(Fl_Preferences::Name("recent-map%d", i), mw->_recent_tilemaps[i]);
	}
	for (int i = 0; i < NUM_RECENT; i++) {
		Preferences::set_string(Fl_Preferences::Name("recent-set%d", i), mw->_recent_tilesets[i]);
	}
	if (mw->_resize_dialog->initialized()) {
		Preferences::set("resize-anchor", mw->_resize_dialog->anchor());
	}

	exit(EXIT_SUCCESS);
}

void Main_Window::undo_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	mw->_tilemap.undo();
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::redo_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	mw->_tilemap.redo();
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::classic_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_classic_theme();
	mw->_classic_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::aero_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_aero_theme();
	mw->_aero_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::metro_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_metro_theme();
	mw->_metro_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::aqua_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_aqua_theme();
	mw->_aqua_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::greybird_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_greybird_theme();
	mw->_greybird_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::metal_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_metal_theme();
	mw->_metal_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::blue_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_blue_theme();
	mw->_blue_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::olive_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_olive_theme();
	mw->_olive_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::rose_gold_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_rose_gold_theme();
	mw->_rose_gold_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::dark_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_dark_theme();
	mw->_dark_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::zoom_in_cb(Fl_Widget *, Main_Window *mw) {
	if (Config::zoom() < MAX_ZOOM) {
		Config::zoom(Config::zoom() + 1);
	}
	mw->update_zoom();
	mw->redraw();
}

void Main_Window::zoom_out_cb(Fl_Widget *, Main_Window *mw) {
	if (Config::zoom() > MIN_ZOOM) {
		Config::zoom(Config::zoom() - 1);
	}
	mw->update_zoom();
	mw->redraw();
}

void Main_Window::zoom_reset_cb(Fl_Widget *, Main_Window *mw) {
	Config::zoom(DEFAULT_ZOOM);
	mw->update_zoom();
	mw->redraw();
}

void Main_Window::grid_cb(Fl_Menu_ *m, Main_Window *mw) {
	Config::grid(!!m->mvalue()->value());
	mw->_grid_tb->value(Config::grid());
	mw->redraw();
}

void Main_Window::rainbow_tiles_cb(Fl_Menu_ *m, Main_Window *mw) {
	Config::rainbow_tiles(!!m->mvalue()->value());
	mw->_rainbow_tiles_tb->value(Config::rainbow_tiles());
	mw->redraw();
}

void Main_Window::bold_palettes_cb(Fl_Menu_ *m, Main_Window *mw) {
	Config::bold_palettes(!!m->mvalue()->value());
	mw->_bold_palettes_tb->value(Config::bold_palettes());
	mw->redraw();
}

void Main_Window::grid_tb_cb(Toolbar_Button *, Main_Window *mw) {
	Config::grid(!!mw->_grid_tb->value());
	if (Config::grid()) { mw->_grid_mi->set(); }
	else { mw->_grid_mi->clear(); }
	mw->redraw();
}

void Main_Window::rainbow_tiles_tb_cb(Toolbar_Button *, Main_Window *mw) {
	Config::rainbow_tiles(!!mw->_rainbow_tiles_tb->value());
	if (Config::rainbow_tiles()) { mw->_rainbow_tiles_mi->set(); }
	else { mw->_rainbow_tiles_mi->clear(); }
	mw->redraw();
}

void Main_Window::bold_palettes_tb_cb(Toolbar_Button *, Main_Window *mw) {
	Config::bold_palettes(!!mw->_bold_palettes_tb->value());
	if (Config::bold_palettes()) { mw->_bold_palettes_mi->set(); }
	else { mw->_bold_palettes_mi->clear(); }
	mw->redraw();
}

void Main_Window::tilemap_width_cb(Fl_Menu_ *, Main_Window *mw) {
	mw->_tilemap_width_dialog->tilemap_width((size_t)mw->_tilemap_width->value());
	mw->_tilemap_width_dialog->show(mw);
	if (mw->_tilemap_width_dialog->canceled()) { return; }
	mw->_tilemap_width->default_value(mw->_tilemap_width_dialog->tilemap_width());
	tilemap_width_tb_cb(NULL, mw);
}

void Main_Window::resize_cb(Fl_Menu_ *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	mw->_resize_dialog->tilemap_size(mw->_tilemap.width(), mw->_tilemap.height());
	mw->_resize_dialog->show(mw);
	if (mw->_resize_dialog->canceled()) { return; }
	mw->resize_tilemap();
}

void Main_Window::reformat_cb(Fl_Menu_ *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	mw->_reformat_dialog->format(Config::format());
	mw->_reformat_dialog->show(mw);
	if (mw->_reformat_dialog->canceled()) { return; }
	mw->reformat_tilemap();
}

void Main_Window::image_to_tiles_cb(Fl_Widget *, Main_Window *mw) {
	mw->_image_to_tiles_dialog->format(Config::format());
	mw->_image_to_tiles_dialog->show(mw);
	if (mw->_image_to_tiles_dialog->canceled()) { return; }
	mw->image_to_tiles();
}

void Main_Window::help_cb(Fl_Widget *, Main_Window *mw) {
	mw->_help_window->show(mw);
}

void Main_Window::about_cb(Fl_Widget *, Main_Window *mw) {
	mw->_about_dialog->show(mw);
}

void Main_Window::tilemap_width_tb_cb(OS_Spinner *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	mw->_tilemap.width((size_t)mw->_tilemap_width->value());
	int sx = mw->_tilemap_scroll->x() + Fl::box_dx(mw->_tilemap_scroll->box());
	int sy = mw->_tilemap_scroll->y() + Fl::box_dy(mw->_tilemap_scroll->box());
	mw->_tilemap_scroll->init_sizes();
	int cw = (int)mw->_tilemap.width() * TILE_SIZE * Config::zoom();
	int ch = (int)mw->_tilemap.height() * TILE_SIZE * Config::zoom();
	mw->_tilemap_scroll->contents(cw, ch);
	mw->_tilemap_scroll->scroll_to(0, 0);
	mw->_tilemap.reposition_tiles(sx, sy);
	mw->_tilemap_scroll->redraw();
	mw->update_status(NULL);
}

void Main_Window::x_flip_cb(Toolbar_Toggle_Button *, Main_Window *mw) {
	mw->_current_tile->x_flip(mw->x_flip());
	mw->_current_tile->redraw();
}

void Main_Window::y_flip_cb(Toolbar_Toggle_Button *, Main_Window *mw) {
	mw->_current_tile->y_flip(mw->y_flip());
	mw->_current_tile->redraw();
}

void Main_Window::priority_cb(OS_Check_Button *, Main_Window *mw) {
	mw->_current_attributes->priority(mw->priority());
	mw->_current_attributes->redraw();
}

void Main_Window::obp1_cb(OS_Check_Button *, Main_Window *mw) {
	mw->_current_attributes->obp1(mw->obp1());
	mw->_current_attributes->redraw();
}

void Main_Window::transparency_cb(Default_Slider *, Main_Window *mw) {
	Tile_State::alpha((uchar)(mw->_transparency->value() * (0xFF / 10) + (0xFF / 10)));
	mw->redraw();
}

void Main_Window::change_tab_cb(OS_Tabs *, Main_Window *mw) {
	Config::show_attributes(mw->_left_tabs->value() == mw->_palettes_tab);
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::select_tile_cb(Tile_Button *tb, Main_Window *mw) {
	if (Fl::event_button() == FL_LEFT_MOUSE) {
		// Left-click to select
		mw->select_tile(tb->id());
	}
	else if (Fl::event_button() == FL_RIGHT_MOUSE) {
		// Right-click to highlight
		mw->highlight_tile(tb->id());
	}
}

void Main_Window::select_palette_cb(Palette_Button *pb, Main_Window *mw) {
	if (Fl::event_button() == FL_LEFT_MOUSE) {
		// Left-click to select
		mw->select_palette(pb->palette());
	}
}

void Main_Window::change_tile_cb(Tile_Tessera *tt, Main_Window *mw) {
	if (!mw->_map_editable) { return; }
	if (Fl::event_button() == FL_LEFT_MOUSE) {
		if (!mw->_selected_tile) { return; }
		if (Fl::event_is_click()) {
			mw->_tilemap.remember();
			mw->update_active_controls();
		}
		if (Fl::event_shift()) {
			// Shift+left-click to flood fill
			mw->flood_fill(tt);
		}
		else if (Fl::event_ctrl()) {
			// Ctrl+left-click to replace
			mw->substitute_tile(tt);
		}
		else if (Fl::event_alt()) {
			// Alt+click to swap
			mw->swap_tiles(tt);
		}
		else {
			// Left-click/drag to edit
			mw->edit_tile(tt);
		}
	}
	else if (Fl::event_button() == FL_RIGHT_MOUSE) {
		// Right-click to select
		if (Config::show_attributes()) {
			mw->_priority_tb->value(tt->priority());
			mw->_priority_tb->do_callback();
			mw->_obp1_tb->value(tt->obp1());
			mw->_obp1_tb->do_callback();
			mw->_priority_tb->redraw();
			mw->_obp1_tb->redraw();
			mw->select_palette(tt->palette());
		}
		else {
			mw->_x_flip_tb->value(tt->x_flip());
			mw->_x_flip_tb->do_callback();
			mw->_y_flip_tb->value(tt->y_flip());
			mw->_y_flip_tb->do_callback();
			mw->_x_flip_tb->redraw();
			mw->_y_flip_tb->redraw();
			mw->select_tile(tt->id());
		}
	}
}
