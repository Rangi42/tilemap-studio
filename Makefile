DESTDIR =
PREFIX = /usr/local

tilemapstudio = tilemapstudio
tilemapstudiod = tilemapstudiod

CXX ?= g++
LD = $(CXX)
RM = rm -rf

srcdir = src
resdir = res
tmpdir = tmp
debugdir = tmp/debug
bindir = bin

CXXFLAGS = -std=c++11 -I$(srcdir) -I$(resdir) $(shell fltk-config --use-images --cxxflags)
LDFLAGS = $(shell fltk-config --use-images --ldflags) $(shell pkg-config --libs libpng xpm)

RELEASEFLAGS = -DNDEBUG -O3 -flto -march=native
DEBUGFLAGS = -DDEBUG -D_DEBUG -O0 -g -ggdb3 -Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter

COMMON = $(wildcard $(srcdir)/*.h) $(wildcard $(resdir)/*.xpm)
SOURCES = $(wildcard $(srcdir)/*.cpp)
OBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(tmpdir)/%.o)
DEBUGOBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(debugdir)/%.o)
TARGET = $(bindir)/$(tilemapstudio)
DEBUGTARGET = $(bindir)/$(tilemapstudiod)
DESKTOP = "$(DESTDIR)$(PREFIX)/share/applications/Tilemap Studio.desktop"

.PHONY: all $(tilemapstudio) $(tilemapstudiod) release debug clean install uninstall

.SUFFIXES: .o .cpp

all: $(tilemapstudio)

$(tilemapstudio): release
$(tilemapstudiod): debug

release: CXXFLAGS += $(RELEASEFLAGS)
release: $(TARGET)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(DEBUGTARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(LDFLAGS)

$(DEBUGTARGET): $(DEBUGOBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(LDFLAGS)

$(tmpdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(debugdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET) $(DEBUGTARGET) $(OBJECTS) $(DEBUGOBJECTS)

install: release
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(tilemapstudio)
	mkdir -p $(DESTDIR)$(PREFIX)/share/pixmaps
	cp $(resdir)/app.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/tilemapstudio48.xpm
	cp $(resdir)/app-icon.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/tilemapstudio16.xpm
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	echo "[Desktop Entry]" > $(DESKTOP)
	echo "Name=Tilemap Studio" >> $(DESKTOP)
	echo "Comment=Edit Game Boy, Color, and Advance tilemaps" >> $(DESKTOP)
	echo "Icon=$(PREFIX)/share/pixmaps/tilemapstudio48.xpm" >> $(DESKTOP)
	echo "Exec=$(PREFIX)/bin/$(tilemapstudio)" >> $(DESKTOP)
	echo "Type=Application" >> $(DESKTOP)
	echo "Terminal=false" >> $(DESKTOP)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(tilemapstudio)
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/tilemapstudio48.xpm
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/tilemapstudio16.xpm
	rm -f $(DESKTOP)
