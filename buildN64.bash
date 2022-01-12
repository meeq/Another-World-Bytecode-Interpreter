#!/usr/bin/env bash

set -Eeuo pipefail

rebuild=0
debug=0
start=0
builddir="buildN64"
title="Out Of This World"

while :; do
	case "${1-}" in
	-d | --debug) debug=1 ;;
	-r | --rebuild) rebuild=1 ;;
	-t | --title)
		title="${2-}"
		shift
		;;
	*) break ;;
	esac
	shift
done

if [ $debug -eq 1 ]; then
	buildtype=Debug
else
	buildtype=Release
fi

# Preserve CMake build files between rebuilds
if [ $rebuild -eq 0 ]; then
	rm -Rf $builddir
	cmake -B $builddir \
		-DCMAKE_BUILD_TYPE=$buildtype \
		-DCMAKE_TOOLCHAIN_FILE=./cmake/N64LibDragon.cmake \
		-DSYS_IMPLEMENTATION=N64 \
		-DGAME_TITLE="$title"
	touch ./cmake/N64LibDragon.cmake
fi

cmake --build $builddir --config $buildtype
cd $builddir

# Assume game files don't change between rebuilds
if [ $rebuild -eq 0 ]; then
	rm -f raw.dfs
	$N64_INST/bin/mkdfs raw.dfs ../game
fi

$N64_INST/bin/mips64-elf-objcopy raw.elf raw.bin -O binary
$N64_INST/bin/n64tool -l 4M \
	-h $N64_INST/mips64-elf/lib/header \
	-o raw.z64 \
	-t "$title" \
	raw.bin -s 1M raw.dfs
$N64_INST/bin/chksum64 raw.z64
