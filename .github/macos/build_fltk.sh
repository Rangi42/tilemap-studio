#!/bin/sh
set -e

version="1.3.8"

NPROC="${NPROC:-$(getconf _NPROCESSORS_ONLN)}"
export MAKEFLAGS="-j$NPROC"

prefix="$PWD/lib"
mkdir -p "$prefix"
cd "$prefix"

[ -f "fltk-$version-source.tar.bz2" ] || curl -LO "https://www.fltk.org/pub/fltk/$version/fltk-$version-source.tar.bz2"
rm -rf "fltk-$version"
tar xf "fltk-$version-source.tar.bz2"
cd "fltk-$version"
./configure \
	--prefix="$prefix" \
	--with-abiversion=10308 \
	--enable-localjpeg \
	--enable-localpng \
	--disable-localzlib
make DIRS='$(IMAGEDIRS) src $(CAIRODIR)' install
cd ..
rm -rf "fltk-$version"
