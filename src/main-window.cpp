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
#pragma warning(pop)

#include "version.h"
#include "utils.h"
#include "themes.h"
#include "widgets.h"
#include "modal-dialog.h"
#include "option-dialogs.h"
#include "preferences.h"
#include "config.h"
#include "tilemap.h"
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
	_tilemap_file(), _tileset_file(), _recent(), _tilemap(), _wx(x), _wy(y), _ww(w), _wh(h) {

	// Get global configs
	Tilemap::Format format_config = (Tilemap::Format)Preferences::get("format", Config::format());
	int start_config = Preferences::get("start", Config::start());
	int tiles2x_config = Preferences::get("tiles2x", Config::tiles2x());
	int rainbow_tiles_config = Preferences::get("rainbow", Config::rainbow_tiles());
	Config::format(format_config);
	Config::start((uint8_t)start_config);
	Config::tiles2x(!!tiles2x_config);
	Config::rainbow_tiles(!!rainbow_tiles_config);

	for (int i = 0; i < NUM_RECENT; i++) {
		_recent[i] = Preferences::get_string(Fl_Preferences::Name("recent%d", i));
	}

	// Populate window

	int wx = 0, wy = 0, ww = w, wh = h;

	// Initialize menu bar
	_menu_bar = new Fl_Menu_Bar(wx, wy, ww, 21);
	wy += _menu_bar->h();
	wh -= _menu_bar->h();

	// Initialize status bar
	_status_bar = new Toolbar(wx, wh+_menu_bar->h()-23, ww, 23);
	wh -= _status_bar->h();
	_tilemap_dimensions = new Status_Bar_Field(0, 0, text_width("Tilemap: 999 x 999", 8), 21, "");
	new Spacer(0, 0, 2, 21);
	_tilemap_format = new Status_Bar_Field(0, 0, text_width("Run-length encoded", 8), 21, "");
	new Spacer(0, 0, 2, 21);
	_hover_id = new Status_Bar_Field(0, 0, text_width("ID: $99", 8), 21, "");
	new Spacer(0, 0, 2, 21);
	_hover_xy = new Status_Bar_Field(0, 0, text_width("X/Y (999, 999)", 8), 21, "");
	_status_bar->end();
	begin();

	// Main group
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	_main_group = new Fl_Group(wx, wy, ww, wh);
	wx += win_m; ww -= win_m * 2;
	wy += win_m; wh -= win_m * 2;
	_left_group = new Fl_Group(wx, wy, ww-280, wh);
	int gx = _left_group->x(), gy = _left_group->y(), gw = _left_group->w(), gh = _left_group->h();
	_left_top_bar = new Fl_Group(gx, gy, gw, wgt_h);
	int bx = _left_top_bar->x(), by = _left_top_bar->y(), bw = _left_top_bar->w(), bh = _left_top_bar->h();
	int wgt_w = text_width("Tilemap:", 4);
	Label *tilemap_heading = new Label(bx, by, wgt_w, bh, "Tilemap:");
	bx += tilemap_heading->w(); bw -= tilemap_heading->w();
	_new_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _new_tb->w(); bw -= _new_tb->w();
	_open_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _open_tb->w(); bw -= _open_tb->w();
	_save_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _save_tb->w(); bw -= _save_tb->w();
	_print_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _print_tb->w() + win_m; bw -= _print_tb->w() + win_m;
	int wgt_off = text_width("Format:", 2);
	bx += wgt_off; bw -= wgt_off;
	_format = new Dropdown(bx, by, bw, bh, "Format:");
	_left_top_bar->end();
	_left_top_bar->resizable(_format);
	gy += _left_top_bar->h() + wgt_m; gh -= _left_top_bar->h() + wgt_m;
	_left_group->begin();
	_left_bottom_bar = new Fl_Group(gx, gy, gw, wgt_h);
	bx = _left_bottom_bar->x(), by = _left_bottom_bar->y(), bw = _left_bottom_bar->w(), bh = _left_bottom_bar->h();
	wgt_off = wgt_m + wgt_h * 4 + text_width("Width:", 2) + text_width("999", 2);
	wgt_w = bw - wgt_off;
	_tilemap_name = new Label_Button(bx, by, wgt_w, bh);
	bx += wgt_w;
	_undo_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _undo_tb->w();
	_redo_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _redo_tb->w();
	new Fl_Box(bx, by, 2, wgt_h); bx += 2;
	new Spacer(bx, by, 2, wgt_h); bx += 2;
	new Fl_Box(bx, by, 2, wgt_h); bx += 2;
	wgt_off = text_width("Width:", 2);
	wgt_w = text_width("999", 2) + wgt_h;
	_tilemap_width = new OS_Spinner(bx+wgt_off, by, wgt_w, wgt_h, "Width:");
	bx += wgt_off + wgt_w + wgt_m; bw -= wgt_off + wgt_w + wgt_m;
	_resize_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _resize_tb->w();
	_left_bottom_bar->end();
	_left_bottom_bar->resizable(_tilemap_name);
	gy += _left_bottom_bar->h() + wgt_m; gh -= _left_bottom_bar->h() + wgt_m;
	_left_group->begin();
	_tilemap_scroll = new Workspace(gx, gy, gw, gh);
	_tilemap_scroll->end();
	_tilemap_scroll->resizable(NULL);
	_left_group->resizable(_tilemap_scroll);
	wx += _left_group->w() + win_m; ww -= _left_group->w() + win_m;
	_main_group->begin();
	Spacer *mid_spacer = new Spacer(wx, wy, 2, wh);
	wx += mid_spacer->w() + win_m; ww -= mid_spacer->w() + win_m;
	_right_group = new Fl_Group(wx, wy, ww, wh);
	_right_top_bar = new Fl_Group(wx, wy, ww, wgt_h);
	bx = _right_top_bar->x(), by = _right_top_bar->y(), bw = _right_top_bar->w(), bh = _right_top_bar->h();
	wgt_w = text_width("Tileset:", 4);
	Label *tileset_heading = new Label(bx, by, wgt_w, bh, "Tileset:");
	bx += tileset_heading->w(); bw -= tileset_heading->w();
	_load_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _load_tb->w(); bw -= _load_tb->w();
	_reload_tb = new Toolbar_Button(bx, by, wgt_h, wgt_h);
	bx += _reload_tb->w(); bw -= _reload_tb->w();
	wgt_off = MAX(text_width("AA", 2), text_width("FF", 2)) + wgt_h;
	wgt_w = text_width("2x Tiles", 2) + wgt_h;
	_start_id = new OS_Hex_Spinner(bx+bw-wgt_w-win_m-wgt_off, by, wgt_off, bh, "Start:");
	bx += _start_id->w() + win_m; bw -= _start_id->w() + win_m;
	_2x_tiles = new OS_Check_Button(bx+bw-wgt_w, by, wgt_w, bh, "2x Tiles");
	_right_top_bar->end();
	_right_top_bar->resizable(NULL);
	wy += _right_top_bar->h() + wgt_m;
	_right_group->begin();
	_right_bottom_bar = new Fl_Group(wx, wy, ww, wgt_h);
	bx = _right_bottom_bar->x(), by = _right_bottom_bar->y(), bw = _right_bottom_bar->w(), bh = _right_bottom_bar->h();
	wgt_off = text_width("Tile: $FFF", 2) + wgt_m;
	wgt_w = bw - wgt_off;
	_tileset_name = new Label_Button(bx, by, wgt_w, bh);
	bx += wgt_w + wgt_m; bw -= wgt_w + wgt_m;
	_tile_heading = new Label(bx, by, bw, bh, "Tile: $00");
	_right_bottom_bar->end();
	_right_bottom_bar->resizable(_tileset_name);
	wy += _right_bottom_bar->h() + wgt_m;
	_right_group->begin();
	_tileset_pane = new Workpane(wx, wy, 258, 258);
	int ox = wx + Fl::box_dx(_tileset_pane->box()), oy = wy + Fl::box_dy(_tileset_pane->box());
	for (int i = 0; i < NUM_TILES; i++) {
		int tx = ox + (i % 16) * TILE_SIZE_PX, ty = oy + (i / 16) * TILE_SIZE_PX;
		Tile_Button *tb = new Tile_Button(tx, ty, (uint8_t)i);
		tb->callback((Fl_Callback *)select_tile_cb, this);
		_tileset[i] = tb;
	}
	_tileset_pane->end();
	_tileset_pane->resizable(NULL);
	wy += _tileset_pane->h() + win_m;
	_right_group->begin();
	wgt_w = text_width("Flip:", 4);
	_flip_heading = new Label(wx, wy, wgt_w, wgt_h, "Flip:");
	wx += _flip_heading->w(); ww -= _flip_heading->w();
	wgt_w = text_width("X", 2) + wgt_h;
	_x_flip = new OS_Check_Button(wx, wy, wgt_w, wgt_h, "X");
	wx += _x_flip->w(); ww -= _x_flip->w();
	wgt_w = text_width("Y", 2) + wgt_h;
	_y_flip = new OS_Check_Button(wx, wy, wgt_w, wgt_h, "Y");
	wx += _y_flip->w(); ww -= _y_flip->w();
	wgt_w = text_width("Image to Tiles...", 12);
	wgt_off = ww - wgt_w;
	_image_to_tiles = new OS_Button(wx+wgt_off, wy, wgt_w, wgt_h, "Image to Tiles...");
	_right_group->end();
	_right_group->resizable(NULL);
	_main_group->begin();
	_main_group->resizable(_left_group);

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
	_new_tilemap_dialog = new New_Tilemap_Dialog("New Tilemap");
	_tilemap_width_dialog = new Tilemap_Width_Dialog("Tilemap Width");
	_resize_dialog = new Resize_Dialog("Resize Tilemap");
	_tileset_start_dialog = new Tileset_Start_Dialog("Tileset Start");
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
	size_range(622, 406);
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
	_tileset_pane->dnd_receiver(_tileset_dnd_receiver);

	// Configure menu bar
	_menu_bar->box(OS_PANEL_THIN_UP_BOX);
	_menu_bar->down_box(FL_FLAT_BOX);

	// Configure menu bar items
	Fl_Menu_Item menu_items[] = {
		// label, shortcut, callback, data, flags
		OS_SUBMENU("&File"),
		OS_MENU_ITEM("&New", FL_COMMAND + 'n', (Fl_Callback *)new_cb, this, 0),
		OS_MENU_ITEM("&Open...", FL_COMMAND + 'o', (Fl_Callback *)open_cb, this, 0),
		OS_MENU_ITEM("Open &Recent", 0, NULL, NULL, FL_SUBMENU | FL_MENU_DIVIDER),
		// NUM_RECENT items with callback open_recent_cb
		OS_NULL_MENU_ITEM(FL_ALT + '1', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '2', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '3', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '4', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '5', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '6', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '7', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '8', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '9', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '0', (Fl_Callback *)open_recent_cb, this, 0),
		OS_MENU_ITEM("Clear &Recent", 0, (Fl_Callback *)clear_recent_cb, this, 0),
		{},
		OS_MENU_ITEM("&Close", FL_COMMAND + 'w', (Fl_Callback *)close_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Save", FL_COMMAND + 's', (Fl_Callback *)save_cb, this, 0),
		OS_MENU_ITEM("Save &As...", FL_COMMAND + 'S', (Fl_Callback *)save_as_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("Load &Tileset...", FL_COMMAND + 't', (Fl_Callback *)load_tileset_cb, this, 0),
		OS_MENU_ITEM("Re&load Tileset", FL_COMMAND + 'r', (Fl_Callback *)reload_tileset_cb, this, 0),
		OS_MENU_ITEM("&Unload Tileset", FL_COMMAND + 'W', (Fl_Callback *)unload_tileset_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Print...", FL_COMMAND + 'p', (Fl_Callback *)print_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("E&xit", FL_ALT + FL_F + 4, (Fl_Callback *)exit_cb, this, 0),
		{},
		OS_SUBMENU("&Edit"),
		OS_MENU_ITEM("&Undo", FL_COMMAND + 'z', (Fl_Callback *)undo_cb, this, 0),
		OS_MENU_ITEM("&Redo", FL_COMMAND + 'y', (Fl_Callback *)redo_cb, this, 0),
		{},
		OS_SUBMENU("&View"),
		OS_MENU_ITEM("&Theme", 0, NULL, NULL, FL_SUBMENU),
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
		OS_MENU_ITEM("&Rainbow Tiles", FL_COMMAND + 'b', (Fl_Callback *)rainbow_tiles_cb, this,
			FL_MENU_TOGGLE | (Config::rainbow_tiles() ? FL_MENU_VALUE : 0)),
		{},
		OS_SUBMENU("&Tools"),
		OS_MENU_ITEM("&Width...", FL_COMMAND + 'd', (Fl_Callback *)tilemap_width_cb, this, 0),
		OS_MENU_ITEM("Re&size...", FL_COMMAND + 'e', (Fl_Callback *)resize_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Image to Tiles...", FL_COMMAND + 'i', (Fl_Callback *)image_to_tiles_cb, this, 0),
		{},
		OS_SUBMENU("&Options"),
		OS_MENU_ITEM("&Format", 0, NULL, NULL, FL_SUBMENU),
		OS_MENU_ITEM("&Plain", 0, (Fl_Callback *)plain_format_cb, this,
			FL_MENU_RADIO | (Config::format() == Tilemap::Format::PLAIN ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Run-length encoded (RLE)", 0, (Fl_Callback *)rle_format_cb, this,
			FL_MENU_RADIO | (Config::format() == Tilemap::Format::RLE ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("GSC &Town Map ($FF end)", 0, (Fl_Callback *)gsc_town_map_format_cb, this,
			FL_MENU_RADIO | (Config::format() == Tilemap::Format::FF_END ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Pok\xc3\xa9gear &card (RLE + $FF end)", 0, (Fl_Callback *)pokegear_card_format_cb, this,
			FL_MENU_RADIO | (Config::format() == Tilemap::Format::RLE_FF_END ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("RBY Town Map (RLE &nybbles + $00 end)", 0, (Fl_Callback *)rby_town_map_format_cb, this,
			FL_MENU_RADIO | (Config::format() == Tilemap::Format::RLE_NYBBLES ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("PC Town Map (&X\x2fY flip)", 0, (Fl_Callback *)pc_town_map_format_cb, this,
			FL_MENU_RADIO | (Config::format() == Tilemap::Format::XY_FLIP ? FL_MENU_VALUE : 0)),
		{},
		OS_MENU_ITEM("Tileset &Start...", 0, (Fl_Callback *)tileset_start_cb, this, 0),
		OS_MENU_ITEM("2x &Tiles", 0, (Fl_Callback *)tiles2x_cb, this,
			FL_MENU_TOGGLE | (Config::tiles2x() ? FL_MENU_VALUE : 0)),
		{},
		OS_SUBMENU("&Help"),
		OS_MENU_ITEM("&Help", FL_F + 1, (Fl_Callback *)help_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&About", FL_COMMAND + '/', (Fl_Callback *)about_cb, this, 0),
		{},
		{}
	};
	_menu_bar->copy(menu_items);

	// Initialize menu bar items
	int first_recent_i = _menu_bar->find_index((Fl_Callback *)open_recent_cb);
	for (int i = 0; i < NUM_RECENT; i++) {
		_recent_mis[i] = const_cast<Fl_Menu_Item *>(&_menu_bar->menu()[first_recent_i + i]);
	}
#define PM_FIND_MENU_ITEM_CB(c) (const_cast<Fl_Menu_Item *>(_menu_bar->find_item((Fl_Callback *)(c))))
	_classic_theme_mi = PM_FIND_MENU_ITEM_CB(classic_theme_cb);
	_aero_theme_mi = PM_FIND_MENU_ITEM_CB(aero_theme_cb);
	_metro_theme_mi = PM_FIND_MENU_ITEM_CB(metro_theme_cb);
	_aqua_theme_mi = PM_FIND_MENU_ITEM_CB(aqua_theme_cb);
	_greybird_theme_mi = PM_FIND_MENU_ITEM_CB(greybird_theme_cb);
	_metal_theme_mi = PM_FIND_MENU_ITEM_CB(metal_theme_cb);
	_blue_theme_mi = PM_FIND_MENU_ITEM_CB(blue_theme_cb);
	_olive_theme_mi = PM_FIND_MENU_ITEM_CB(olive_theme_cb);
	_rose_gold_theme_mi = PM_FIND_MENU_ITEM_CB(rose_gold_theme_cb);
	_dark_theme_mi = PM_FIND_MENU_ITEM_CB(dark_theme_cb);
	_plain_format_mi = PM_FIND_MENU_ITEM_CB(plain_format_cb);
	_rle_format_mi = PM_FIND_MENU_ITEM_CB(rle_format_cb);
	_gsc_town_map_format_mi = PM_FIND_MENU_ITEM_CB(gsc_town_map_format_cb);
	_pokegear_card_format_mi = PM_FIND_MENU_ITEM_CB(pokegear_card_format_cb);
	_rby_town_map_format_mi = PM_FIND_MENU_ITEM_CB(rby_town_map_format_cb);
	_pc_town_map_format_mi = PM_FIND_MENU_ITEM_CB(pc_town_map_format_cb);
	_2x_tiles_mi = PM_FIND_MENU_ITEM_CB(tiles2x_cb);
	_rainbow_tiles_mi = PM_FIND_MENU_ITEM_CB(rainbow_tiles_cb);
	// Conditional menu items
	_reload_tileset_mi = PM_FIND_MENU_ITEM_CB(reload_tileset_cb);
	_unload_tileset_mi = PM_FIND_MENU_ITEM_CB(unload_tileset_cb);
	_close_mi = PM_FIND_MENU_ITEM_CB(close_cb);
	_save_mi = PM_FIND_MENU_ITEM_CB(save_cb);
	_save_as_mi = PM_FIND_MENU_ITEM_CB(save_as_cb);
	_print_mi = PM_FIND_MENU_ITEM_CB(print_cb);
	_undo_mi = PM_FIND_MENU_ITEM_CB(undo_cb);
	_redo_mi = PM_FIND_MENU_ITEM_CB(redo_cb);
	_tilemap_width_mi = PM_FIND_MENU_ITEM_CB(tilemap_width_cb);
	_resize_mi = PM_FIND_MENU_ITEM_CB(resize_cb);
#undef PM_FIND_MENU_ITEM_CB

	// Configure toolbar buttons

	_new_tb->tooltip("New... (Ctrl+N)");
	_new_tb->callback((Fl_Callback *)new_cb, this);
	_new_tb->image(NEW_ICON);
	_new_tb->take_focus();

	_open_tb->tooltip("Open... (Ctrl+O)");
	_open_tb->callback((Fl_Callback *)open_cb, this);
	_open_tb->image(OPEN_ICON);

	_save_tb->tooltip("Save (Ctrl+S)");
	_save_tb->callback((Fl_Callback *)save_cb, this);
	_save_tb->image(SAVE_ICON);
	_save_tb->deimage(SAVE_DISABLED_ICON);

	_print_tb->tooltip("Print (Ctrl+P)");
	_print_tb->callback((Fl_Callback *)print_cb, this);
	_print_tb->image(PRINT_ICON);
	_print_tb->deimage(PRINT_DISABLED_ICON);

	_format->add("Plain");                                // PLAIN
	_format->add("Run-length encoded (RLE)");             // RLE
	_format->add("GSC Town Map ($FF end)");               // FF_END
	_format->add("Pok\xc3\xa9gear card (RLE + $FF end)"); // RLE_FF_END
	_format->add("RBY Town Map (RLE nybbles + $00 end)"); // RLE_NYBBLES
	_format->add("PC Town Map (X\\/Y flip)");             // XY_FLIP
	_format->value(Config::format());
	_format->callback((Fl_Callback *)format_tb_cb, this);

	_tilemap_name->callback((Fl_Callback *)open_cb, this);

	_undo_tb->tooltip("Undo (Ctrl+Z)");
	_undo_tb->callback((Fl_Callback *)undo_cb, this);
	_undo_tb->image(UNDO_ICON);
	_undo_tb->deimage(UNDO_DISABLED_ICON);

	_redo_tb->tooltip("Redo (Ctrl+Y)");
	_redo_tb->callback((Fl_Callback *)redo_cb, this);
	_redo_tb->image(REDO_ICON);
	_redo_tb->deimage(REDO_DISABLED_ICON);

	_tilemap_width->value(20);
	_tilemap_width->range(1, 255);
	_tilemap_width->callback((Fl_Callback *)tilemap_width_tb_cb, this);

	_resize_tb->tooltip("Resize... (Ctrl+E)");
	_resize_tb->callback((Fl_Callback *)resize_cb, this);
	_resize_tb->image(RESIZE_ICON);
	_resize_tb->deimage(RESIZE_DISABLED_ICON);

	_load_tb->tooltip("Load Tileset... (Ctrl+T)");
	_load_tb->callback((Fl_Callback *)load_tileset_cb, this);
	_load_tb->image(LOAD_ICON);

	_reload_tb->tooltip("Reload Tileset... (Ctrl+R)");
	_reload_tb->callback((Fl_Callback *)reload_tileset_cb, this);
	_reload_tb->image(RELOAD_ICON);
	_reload_tb->deimage(RELOAD_DISABLED_ICON);

	_start_id->value(Config::start());
	_start_id->callback((Fl_Callback *)tileset_start_tb_cb, this);

	_2x_tiles->value(Config::tiles2x());
	_2x_tiles->callback((Fl_Callback *)tiles2x_tb_cb, this);

	_tileset_name->callback((Fl_Callback *)load_tileset_cb, this);

	select_tile_cb(_tileset[0x00], this);

	_image_to_tiles->callback((Fl_Callback *)image_to_tiles_cb, this);

	_flip_heading->box(FL_FLAT_BOX);

	// Configure dialogs

	_tilemap_open_chooser->title("Open Tilemap");
	_tilemap_open_chooser->filter("Tilemap Files\t*.{tilemap,rle,bin,map}\n");

	_tilemap_save_chooser->title("Save Tilemap");
	_tilemap_save_chooser->filter("Tilemap Files\t*.{tilemap,rle,bin,map}\n");
	_tilemap_save_chooser->preset_file("NewTilemap.tilemap");
	_tilemap_save_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);

	_tileset_load_chooser->title("Open Tileset");
	_tileset_load_chooser->filter("Tileset Files\t*.{png,bmp,1bpp,2bpp,1bpp.lz,2bpp.lz}\n");

	_png_chooser->title("Print Screenshot");
	_png_chooser->filter("PNG Files\t*.png\n");
	_png_chooser->preset_file("screenshot.png");
	_png_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);

	_error_dialog->width_range(280, 700);
	_warning_dialog->width_range(280, 700);
	_success_dialog->width_range(280, 700);
	_unsaved_dialog->width_range(280, 700);

	_new_tilemap_dialog->tilemap_width(DEFAULT_WIDTH);
	_new_tilemap_dialog->tilemap_height(DEFAULT_HEIGHT);

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

	update_recent_tilemaps();
	update_metadata();
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
	delete _tileset_start_dialog;
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
		if (_recent[i] == _tilemap_file) {
			_recent[i] = last;
			break;
		}
		std::swap(last, _recent[i]);
	}
	update_recent_tilemaps();
}

 void Main_Window::update_recent_tilemaps() {
	int last = -1;
	for (int i = 0; i < NUM_RECENT; i++) {
		if (_recent[i].empty()) {
			_recent_mis[i]->label("");
			_recent_mis[i]->hide();
		}
		else {
			const char *basename = fl_filename_name(_recent[i].c_str());
			_recent_mis[i]->label(basename);
			_recent_mis[i]->show();
			last = i;
		}
		_recent_mis[i]->flags &= ~FL_MENU_DIVIDER;
	}
	_recent_mis[last]->flags |= FL_MENU_DIVIDER;
}

void Main_Window::update_status(Tile_Tessera *tt) {
	if (!_tilemap.size()) {
		_tilemap_dimensions->label("");
		_hover_id->label("");
		_hover_xy->label("");
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
		_status_bar->redraw();
		return;
	}
	sprintf(buffer, "ID: $%02X", tt->id());
	_hover_id->copy_label(buffer);
	sprintf(buffer, "X/Y (%u, %u)", tt->col(), tt->row());
	_hover_xy->copy_label(buffer);
}

void Main_Window::update_metadata() {
	if (_tilemap.size()) {
		if (_tilemap_file.empty()) {
			_tilemap_name->label(NEW_TILEMAP_NAME);
		}
		else {
			const char *basename = fl_filename_name(_tilemap_file.c_str());
			_tilemap_name->label(basename);
		}
		const char *name = _tilemap.format_name(Config::format());
		_tilemap_format->label(name);
	}
	else {
		_tilemap_name->label("No file selected");
		_tilemap_format->label("");
	}
	if (false/*TODO: _tileset has image*/) {
		const char *basename = fl_filename_name(_tileset_file.c_str());
		_tileset_name->label(basename);
	}
	else {
		_tileset_name->label("No file selected");
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
		_resize_mi->activate();
		_resize_tb->activate();
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
		_resize_mi->deactivate();
		_resize_tb->deactivate();
	}
	if (false/*TODO: _tileset has image*/) {
		_reload_tileset_mi->activate();
		_reload_tb->activate();
		_unload_tileset_mi->activate();
	}
	else {
		_reload_tileset_mi->deactivate();
		_reload_tb->deactivate();
		_unload_tileset_mi->deactivate();
	}
	if (Config::format() == Tilemap::Format::XY_FLIP) {
		_flip_heading->activate();
		_x_flip->activate();
		_y_flip->activate();
	}
	else {
		_flip_heading->deactivate();
		_x_flip->clear();
		_x_flip->deactivate();
		_y_flip->clear();
		_y_flip->deactivate();
	}
	int n = Tilemap::format_tileset_size((Tilemap::Format)Config::format());
	for (int i = 0; i < n; i++) {
		_tileset[i]->activate();
	}
	for (int i = n; i < NUM_TILES; i++) {
		_tileset[i]->deactivate();
	}
	if (_selected->id() >= n) {
		select_tile_cb(_tileset[0x00], this);
	}
}

void Main_Window::resize_tilemap() {
	size_t w = (size_t)_resize_dialog->tilemap_width(), h = (size_t)_resize_dialog->tilemap_height();
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

	_tilemap_width->value(w);
	tilemap_width_tb_cb(NULL, this);
	update_status(NULL);
	update_active_controls();
	redraw();
}

void Main_Window::flood_fill(Tile_Tessera *tt) {
	Tile_State fs = tt->state();
	Tile_State ts(_selected->id(), x_flip(), y_flip());
	if (fs == ts) { return; }
	std::queue<size_t> queue;
	size_t w = _tilemap.width(), h = _tilemap.height(), n = _tilemap.size();
	uint8_t row = tt->row(), col = tt->col();
	size_t i = row * w + col;
	queue.push(i);
	while (!queue.empty()) {
		size_t j = queue.front();
		queue.pop();
		if (j >= n) { continue; }
		Tile_Tessera *ff = _tilemap.tile(j);
		if (ff->state() != fs) { continue; }
		ff->state(ts); // fill
		uint8_t r = ff->row(), c = ff->col();
		if (c > 0) { queue.push(j-1); } // left
		if (c < w - 1) { queue.push(j+1); } // right
		if (r > 0) { queue.push(j-w); } // up
		if (r < h - 1) { queue.push(j+w); } // down
	}
}

void Main_Window::substitute_tile(Tile_Tessera *tt) {
	Tile_State fs = tt->state();
	Tile_State ts(_selected->id(), x_flip(), y_flip());
	size_t n = _tilemap.size();
	for (size_t i = 0; i < n; i++) {
		Tile_Tessera *ff = _tilemap.tile(i);
		if (ff->state() == fs) {
			ff->state(ts);
		}
	}
}

void Main_Window::open_tilemap(const char *filename, size_t width, size_t height) {
	_unsaved = false;
	close_cb(NULL, this);

	const char *basename;

	if (filename) {
		_tilemap_file = filename;
		basename = fl_filename_name(filename);

		Tilemap::Result r = _tilemap.read_tiles(filename);
		if (r) {
			_tilemap.clear();
			std::string msg = "Error reading ";
			msg = msg + basename + "!\n\n" + Tilemap::error_message(r);
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return;
		}
	}
	else {
		_tilemap_file = "";
		basename = NEW_TILEMAP_NAME;
		_unsaved = true;

		_tilemap.new_tiles(width, height);
	}

	size_t n = _tilemap.size();
	for (size_t i = 0; i < n; i++) {
		Tile_Tessera *tt = _tilemap.tile(i);
		tt->callback((Fl_Callback *)change_tile_cb, this);
		_tilemap_scroll->add(tt);
	}

	_tilemap_width->value(_tilemap.width());
	tilemap_width_tb_cb(NULL, this);

	// set filenames
	char buffer[FL_PATH_MAX] = {};
	sprintf(buffer, PROGRAM_NAME " - %s", basename);
	copy_label(buffer);
	sprintf(buffer, "%s", basename);
	fl_filename_setext(buffer, sizeof(buffer), ".png");
	_png_chooser->preset_file(buffer);

	store_recent_tilemap();
	update_metadata();
	update_status(NULL);
	update_active_controls();
	redraw();
}

void Main_Window::open_recent_tilemap(int n) {
	if (n < 0 || n >= NUM_RECENT || _recent[n].empty()) {
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

	const char *filename = _recent[n].c_str();
	open_tilemap(filename);
}

bool Main_Window::save_tilemap(bool force) {
	const char *filename = _tilemap_file.c_str();
	const char *basename = fl_filename_name(filename);

	if (!_tilemap.can_write_tiles()) {
		std::string msg = "Cannot format the tilemap as ";
		msg = msg + Tilemap::format_name(Config::format()) + "!";
		_error_dialog->message(msg);
		_error_dialog->show(this);
		return false;
	}

	if (_tilemap.modified() || force) {
		if (!_tilemap.write_tiles(filename)) {
			std::string msg = "Could not write to ";
			msg = msg + basename + "!";
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return false;
		}
	}

	_unsaved = false;
	_tilemap.modified(false);

	char buffer[FL_PATH_MAX] = {};
	sprintf(buffer, PROGRAM_NAME " - %s", basename);
	copy_label(buffer);
	update_metadata();

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

void Main_Window::load_tileset(const char *filename) {
	const char *basename = fl_filename_name(filename);
	// TODO: load_tileset
	fputs("load_tileset", stderr);
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

	mw->_new_tilemap_dialog->show(mw);
	if (mw->_new_tilemap_dialog->canceled()) { return; }

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

void Main_Window::open_recent_cb(Fl_Menu_ *m, Main_Window *mw) {
	int first_recent_i = m->find_index((Fl_Callback *)open_recent_cb);
	int i = m->find_index(m->mvalue()) - first_recent_i;
	mw->open_recent_tilemap(i);
}

void Main_Window::clear_recent_cb(Fl_Menu_ *, Main_Window *mw) {
	for (int i = 0; i < NUM_RECENT; i++) {
		mw->_recent[i].clear();
		mw->_recent_mis[i]->hide();
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

	mw->_unsaved = false;
	mw->label(PROGRAM_NAME);
	mw->_tilemap.clear();
	mw->_tilemap_scroll->clear();
	mw->_tilemap_scroll->scroll_to(0, 0);
	mw->_tilemap_scroll->contents(0, 0);
	mw->init_sizes();
	mw->_tilemap_file.clear();
	mw->update_metadata();
	mw->update_status(NULL);
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::save_cb(Fl_Widget *w, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	if (mw->_tilemap_file.empty()) {
		save_as_cb(w, mw);
	}
	else {
		mw->save_tilemap(false);
	}
}

void Main_Window::save_as_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }

	int status = mw->_tilemap_save_chooser->show();
	if (status == 1) { return; }

	Tilemap::Format fmt = Config::format();

	char filename[FL_PATH_MAX] = {};
	const char *ext = Tilemap::format_extension(fmt);
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

	// TODO: load_tileset_cb
	fputs("load_tileset_cb", stderr);
}

void Main_Window::reload_tileset_cb(Fl_Widget *, Main_Window *) {
	if (false/*TODO: _tileset has image*/) { return; }
	// TODO: reload_tileset_cb
	fputs("reload_tileset_cb", stderr);
}

void Main_Window::unload_tileset_cb(Fl_Widget *, Main_Window *) {
	if (false/*TODO: _tileset has image*/) { return; }
	// TODO: unload_tileset_cb
	fputs("unload_tileset_cb", stderr);
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

	// TODO: print_cb
	fputs("print_cb", stderr);
}

void Main_Window::exit_cb(Fl_Widget *, Main_Window *mw) {
	// Override default behavior of Esc to close main window
	if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) { return; }

	// Save global config
	Preferences::set("theme", OS::current_theme());
	Preferences::set("x", mw->x());
	Preferences::set("y", mw->y());
	Preferences::set("w", mw->w());
	Preferences::set("h", mw->h());
	Preferences::set("format", Config::format());
	Preferences::set("start", (int)Config::start());
	Preferences::set("tiles2x", Config::tiles2x());
	for (int i = 0; i < NUM_RECENT; i++) {
		Preferences::set_string(Fl_Preferences::Name("recent%d", i), mw->_recent[i]);
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

void Main_Window::rainbow_tiles_cb(Fl_Menu_ *m, Main_Window *mw) {
	Config::rainbow_tiles(!!m->mvalue()->value());
	mw->redraw();
}

void Main_Window::tilemap_width_cb(Fl_Menu_ *, Main_Window *mw) {
	mw->_tilemap_width_dialog->tilemap_width((int)mw->_tilemap_width->value());
	mw->_tilemap_width_dialog->show(mw);
	if (mw->_tilemap_width_dialog->canceled()) { return; }
	mw->_tilemap_width->value(mw->_tilemap_width_dialog->tilemap_width());
	tilemap_width_tb_cb(mw->_tilemap_width, mw);
}

void Main_Window::resize_cb(Fl_Menu_ *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	mw->_resize_dialog->tilemap_size((uint8_t)mw->_tilemap.width(), (uint8_t)mw->_tilemap.height());
	mw->_resize_dialog->show(mw);
	if (mw->_resize_dialog->canceled()) { return; }
	mw->resize_tilemap();
}

void Main_Window::image_to_tiles_cb(Fl_Widget *, Main_Window *mw) {
	mw->_image_to_tiles_dialog->format(Config::format());
	mw->_image_to_tiles_dialog->start_id(Config::start());
	mw->_image_to_tiles_dialog->tiles2x(Config::tiles2x());
	mw->_image_to_tiles_dialog->show(mw);
	if (mw->_tileset_start_dialog->canceled()) { return; }
	// TODO: image_to_tiles_cb
	fputs("image_to_tiles_cb", stderr);
}

void Main_Window::plain_format_cb(Fl_Menu_ *, Main_Window *mw) {
	Config::format(Tilemap::Format::PLAIN);
	mw->_format->value(Config::format());
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::rle_format_cb(Fl_Menu_ *, Main_Window *mw) {
	Config::format(Tilemap::Format::RLE);
	mw->_format->value(Config::format());
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::gsc_town_map_format_cb(Fl_Menu_ *, Main_Window *mw) {
	Config::format(Tilemap::Format::FF_END);
	mw->_format->value(Config::format());
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::pokegear_card_format_cb(Fl_Menu_ *, Main_Window *mw) {
	Config::format(Tilemap::Format::RLE_FF_END);
	mw->_format->value(Config::format());
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::rby_town_map_format_cb(Fl_Menu_ *, Main_Window *mw) {
	Config::format(Tilemap::Format::RLE_NYBBLES);
	mw->_format->value(Config::format());
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::pc_town_map_format_cb(Fl_Menu_ *, Main_Window *mw) {
	Config::format(Tilemap::Format::XY_FLIP);
	mw->_format->value(Config::format());
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::tileset_start_cb(Fl_Menu_ *, Main_Window *mw) {
	mw->_tileset_start_dialog->start_id(Config::start());
	mw->_tileset_start_dialog->show(mw);
	if (mw->_tileset_start_dialog->canceled()) { return; }
	mw->_start_id->value(mw->_tileset_start_dialog->start_id());
	mw->_start_id->redraw();
}

void Main_Window::help_cb(Fl_Widget *, Main_Window *mw) {
	mw->_help_window->show(mw);
}

void Main_Window::about_cb(Fl_Widget *, Main_Window *mw) {
	mw->_about_dialog->show(mw);
}

void Main_Window::tiles2x_cb(Fl_Menu_ *m, Main_Window *mw) {
	Config::tiles2x(!!m->mvalue()->value());
	mw->_2x_tiles->value(Config::tiles2x());
	mw->_2x_tiles->redraw();
}

void Main_Window::format_tb_cb(Dropdown *, Main_Window *mw) {
	Config::format((Tilemap::Format)mw->_format->value());
	Fl_Menu_Item *menu_items[NUM_FORMATS] = {
		mw->_plain_format_mi, mw->_rle_format_mi, mw->_gsc_town_map_format_mi,
		mw->_pokegear_card_format_mi, mw->_rby_town_map_format_mi, mw->_pc_town_map_format_mi
	};
	menu_items[Config::format()]->setonly();
	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::tilemap_width_tb_cb(OS_Spinner *, Main_Window *mw) {
	if (!mw->_tilemap.size()) { return; }
	mw->_tilemap.width((size_t)mw->_tilemap_width->value());
	int sx = mw->_tilemap_scroll->x() + Fl::box_dx(mw->_tilemap_scroll->box());
	int sy = mw->_tilemap_scroll->y() + Fl::box_dy(mw->_tilemap_scroll->box());
	mw->_tilemap.reposition_tiles(sx, sy);
	mw->_tilemap_scroll->contents(mw->_tilemap.width() * TILE_SIZE_PX, mw->_tilemap.height() * TILE_SIZE_PX);
	mw->_tilemap_scroll->scroll_to(0, 0);
	mw->_tilemap_scroll->init_sizes();
	mw->_tilemap_scroll->redraw();
}

void Main_Window::tileset_start_tb_cb(OS_Hex_Spinner *, Main_Window *mw) {
	Config::start((uint8_t)mw->_start_id->value());
}

void Main_Window::tiles2x_tb_cb(OS_Check_Button *, Main_Window *mw) {
	Config::tiles2x(!!mw->_2x_tiles->value());
	if (Config::tiles2x()) {
		mw->_2x_tiles_mi->set();
	}
	else {
		mw->_2x_tiles_mi->clear();
	}
}

void Main_Window::change_tile_cb(Tile_Tessera *tt, Main_Window *mw) {
	if (!mw->_map_editable) { return; }
	if (Fl::event_button() == FL_LEFT_MOUSE) {
		if (!mw->_selected) { return; }
		if (Fl::event_is_click()) {
			mw->_tilemap.remember();
			mw->update_active_controls();
		}
		if (Fl::event_shift()) {
			// Shift+left-click to flood fill
			mw->flood_fill(tt);
			mw->_tilemap_scroll->redraw();
			mw->_tilemap.modified(true);
		}
		else if (Fl::event_ctrl()) {
			// Ctrl+left-click to replace
			mw->substitute_tile(tt);
			mw->_tilemap_scroll->redraw();
			mw->_tilemap.modified(true);
		}
		else {
			// Left-click/drag to edit
			tt->id(mw->_selected->id());
			tt->x_flip(mw->x_flip());
			tt->y_flip(mw->y_flip());
			tt->damage(1);
			mw->_tilemap.modified(true);
		}
	}
	else if (Fl::event_button() == FL_RIGHT_MOUSE) {
		// Right-click to select
		mw->_x_flip->value(tt->x_flip());
		mw->_y_flip->value(tt->y_flip());
		mw->_x_flip->redraw();
		mw->_y_flip->redraw();
		uint8_t id = tt->id();
		select_tile_cb(mw->_tileset[id], mw);
	}
}

void Main_Window::select_tile_cb(Tile_Button *tb, Main_Window *mw) {
	tb->setonly();
	mw->_selected = tb;
	char buffer[32];
	sprintf(buffer, "Tile: $%02X", tb->id());
	mw->_tile_heading->copy_label(buffer);
	mw->_tile_heading->redraw();
}
