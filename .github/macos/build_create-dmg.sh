#!/bin/sh
set -e

version="1.0.9"

prefix="$PWD/lib"
mkdir -p "$prefix"
cd "$prefix"

[ -f "create-dmg-$version.tar.gz" ] || curl -Lo "create-dmg-$version.tar.gz" "https://github.com/create-dmg/create-dmg/archive/refs/tags/v$version.tar.gz"
rm -rf "create-dmg-$version"
tar xf "create-dmg-$version.tar.gz"
cd "create-dmg-$version"
make prefix="$prefix" install
cd ..
rm -rf "create-dmg-$version"
