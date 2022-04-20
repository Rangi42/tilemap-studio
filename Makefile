OS_MAC :=
ifeq ($(shell uname -s),Darwin)
OS_MAC := 1
endif

DESTDIR =
PREFIX = /usr/local

APPNAME = Tilemap Studio
tilemapstudio = tilemapstudio
tilemapstudiod = tilemapstudiod

ifdef OS_MAC
CXX ?= clang++
else
CXX ?= g++
endif
LD = $(CXX)
RM = rm -rf

srcdir = src
resdir = res
tmpdir = tmp
debugdir = tmp/debug
bindir = bin

CXXFLAGS := -std=c++17 -I$(srcdir) -I$(resdir) $(shell fltk-config --use-images --cxxflags) $(CXXFLAGS)
LDFLAGS := $(shell fltk-config --use-images --ldflags) $(LDFLAGS)
ifndef OS_MAC
LDFLAGS += $(shell pkg-config --libs libpng xpm)
endif

RELEASEFLAGS = -DNDEBUG -O3 -flto
DEBUGFLAGS = -DDEBUG -D_DEBUG -O0 -g -ggdb3 -Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter

COMMON = $(wildcard $(srcdir)/*.h) $(wildcard $(resdir)/*.xpm) $(resdir)/help.html
SOURCES = $(wildcard $(srcdir)/*.cpp)
OBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(tmpdir)/%.o)
DEBUGOBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(debugdir)/%.o)

ifdef OS_MAC
SOURCES_MAC = $(wildcard $(srcdir)/*.mm)
OBJECTS += $(SOURCES_MAC:$(srcdir)/%.mm=$(tmpdir)/%.o)
DEBUGOBJECTS += $(SOURCES_MAC:$(srcdir)/%.mm=$(debugdir)/%.o)
endif

TARGET = $(bindir)/$(tilemapstudio)
DEBUGTARGET = $(bindir)/$(tilemapstudiod)

.PHONY: all $(tilemapstudio) $(tilemapstudiod) release debug clean appdir appdmg install uninstall

.SUFFIXES: .o .cpp

all: $(tilemapstudio)

$(tilemapstudio): release
$(tilemapstudiod): debug

release: CXXFLAGS := $(RELEASEFLAGS) $(CXXFLAGS)
release: $(TARGET)

debug: CXXFLAGS := $(DEBUGFLAGS) $(CXXFLAGS)
debug: $(DEBUGTARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(DEBUGTARGET): $(DEBUGOBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(tmpdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(debugdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifdef OS_MAC
$(tmpdir)/%.o: $(srcdir)/%.mm $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(debugdir)/%.o: $(srcdir)/%.mm $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<
endif

clean:
	$(RM) $(TARGET) $(DEBUGTARGET) $(OBJECTS) $(DEBUGOBJECTS)

ifdef OS_MAC
APPDIR = "$(bindir)/$(APPNAME).app"
APPDMG = "$(bindir)/$(APPNAME).dmg"
CONTENTS = $(APPDIR)/Contents

appdir: release
	rm -rf $(APPDIR)
	install -d $(CONTENTS)/macOS $(CONTENTS)/Resources
	install -m755 $(TARGET) $(CONTENTS)/macOS/tilemapstudio
	install -m644 $(resdir)/app.icns $(CONTENTS)/Resources/AppIcon.icns
	install -m644 $(resdir)/Info.plist $(CONTENTS)/Info.plist
	printf 'APPL????' > $(CONTENTS)/PkgInfo

appdmg: appdir
	rm -f $(APPDMG)
	rm -rf $(APPDMG).dir/
	mkdir -p $(APPDMG).dir
	cp -a $(APPDIR) $(APPDMG).dir/
	create-dmg \
	  --volname "$(APPNAME)" \
	  --volicon $(resdir)/app.icns \
	  --window-pos 200 120 \
	  --window-size 800 400 \
	  --icon-size 100 \
	  --icon "$(APPNAME).app" 200 190 \
	  --hide-extension "$(APPNAME).app" \
	  --app-drop-link 600 185 \
	  $(APPDMG) $(APPDMG).dir/
	rm -rf $(APPDMG).dir/

install: appdir
	rm -rf "/Applications/$(APPNAME).app"
	cp -av $(APPDIR) "/Applications/$(APPNAME).app"
# Remove admin-owned files if ran as "sudo"
	rm -rf $(APPDIR)

uninstall:
	rm -rf "/Applications/$(APPNAME).app"
else
DESKTOP = "$(DESTDIR)$(PREFIX)/share/applications/$(APPNAME).desktop"

install: release
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(tilemapstudio)
	mkdir -p $(DESTDIR)$(PREFIX)/share/pixmaps
	cp $(resdir)/app.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/tilemapstudio48.xpm
	cp $(resdir)/app-icon.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/tilemapstudio16.xpm
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	echo "[Desktop Entry]" > $(DESKTOP)
	echo "Name=$(APPNAME)" >> $(DESKTOP)
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
endif
