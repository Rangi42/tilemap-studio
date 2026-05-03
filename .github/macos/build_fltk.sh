#!/bin/sh
set -e

version="1.4.4"

NPROC="${NPROC:-$(getconf _NPROCESSORS_ONLN)}"
export MAKEFLAGS="-j$NPROC"

prefix="$PWD/lib"
mkdir -p "$prefix"
cd "$prefix"

[ -f "fltk-$version-source.tar.bz2" ] || \
	curl -LO "https://github.com/fltk/fltk/releases/download/release-$version/fltk-$version-source.tar.bz2"
rm -rf "fltk-$version"
tar xf "fltk-$version-source.tar.bz2"
cd "fltk-$version"
cmake \
	-D CMAKE_INSTALL_PREFIX="$(realpath "$PWD/../..")" \
	-D CMAKE_BUILD_TYPE=Release \
	-D FLTK_USE_SYSTEM_LIBPNG=0 \
	-D FLTK_USE_SYSTEM_ZLIB=0 \
	-D CMAKE_OSX_DEPLOYMENT_TARGET="$(sw_vers -productVersion | cut -d '.' -f 1).0"
make DIRS='$(IMAGEDIRS) src $(CAIRODIR)' install
cd ..
rm -rf "fltk-$version"
