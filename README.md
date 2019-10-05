# Tilemap Studio

A tilemap editor for Game Boy, Color, and Advance projects. Specifically meant to support the [pret](https://github.com/pret) disassemblies like [pokered](https://github.com/pret/pokered), [pokecrystal](https://github.com/pret/pokecrystal), and [pokeemerald](https://github.com/pret/pokeemerald); as well as hacks including [Polished Crystal](https://github.com/Rangi42/polishedcrystal), [Red++](https://github.com/TheFakeMateo/RedPlusPlus), [Orange](https://github.com/PiaCarrot/pokeorange), [Prism](https://www.reddit.com/r/PokemonPrism), and many more.

Inspired by tools like [Tilemap Creator](https://github.com/erandis-vol/Tilemap-Creator), [NTME](https://www.pokecommunity.com/showthread.php?t=149454), [GSC Town Map Editor](https://hax.iimarckus.org/topic/97/) and [Karteneditor](https://i.imgur.com/70jDfdM.png), but implemented with C++ and [FLTK](http://www.fltk.org/), and with support for multi-platform tilemaps as well as Pokémon-specific formats.

Latest release: [**1.1.1**](https://github.com/Rangi42/tilemap-studio/releases/tag/v1.1.1)

Follow the steps in [INSTALL.md](INSTALL.md) to install the release copy of Tilemap Studio, or the longer instructions to build it yourself from source.

The [example/](example/) directory contains different kinds of tilemaps and tileset graphics. **pokered/town_map.rle** is an RBY Town Map; **pokecrystal/\*.bin** are GSC Town Maps; **polishedcrystal/\*.bin** are PC Town Maps; **pokecrystal/\*.tilemap.rle** are Pokégear cards; **polishedcrystal/sgb_border.map** is tiles + attributes; and **prism/*.bin** are plain tiles.

More information is at the [Skeetendo Forums](https://hax.iimarckus.org/topic/7691/). If you have questions or comments, please go there.

![Screenshot](screenshot.png)
