#include "config.h"

Tilemap_Format Config::_format = Tilemap_Format::PLAIN;
int Config::_zoom = 2;
bool Config::_grid = false;
bool Config::_rainbow_tiles = false;
bool Config::_bold_palettes = true;
bool Config::_print_grid = false;
bool Config::_print_rainbow_tiles = false;
bool Config::_print_palettes = false;
bool Config::_print_bold_palettes = false;
uint16_t Config::_highlight_id = (uint16_t)-1;
bool Config::_show_attributes = false;
bool Config::_auto_load_tileset = true;
