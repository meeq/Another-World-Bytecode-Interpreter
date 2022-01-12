#!/usr/bin/env bash

set -Eeuo pipefail

rebuild=0
debug=0
start=0
builddir="buildSDL2"
title="Out Of This World"

while :; do
	case "${1-}" in
	-d | --debug) debug=1 ;;
	-r | --rebuild) rebuild=1 ;;
	-s | --start) start=1 ;;
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
		-DSYS_IMPLEMENTATION=SDL2 \
		-DGAME_TITLE="$title" \
		-Wno-dev
fi

cmake --build $builddir --config $buildtype

if [ $start -eq 1 ]; then
	$builddir/raw --datapath=./game
fi
