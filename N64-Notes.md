# Nintendo 64 Notes

This project was originally written for SDL2, but was abstracted so that alternate system
implementations could be provided on top of the `System` base struct.

This branch uses the open-source libdragon SDK to run on the Nintendo 64.

## Port Status

As of right now, the implementation is incomplete and highly experimental.

## Prerequisites

### Install libdragon

See https://github.com/DragonMinded/libdragon for instructions.

### Compile and install zlib for libdragon

This game uses zlib to compress resources. It might make sense to decompress the files
so that the N64 doesn't have to spend a bunch of time decompressing at run-time, but
for the short term the simplest solution is to preserve the game files as-is.

You will need to download, cross-compile, and install zlib into your N64 SDK:

```sh
curl -O https://zlib.net/zlib-1.2.11.tar.gz
tar -xf zlib-1.2.11.tar.gz
cd zlib-1.2.11
prefix=${N64_INST}/mips64-elf CROSS_PREFIX=${N64_INST}/bin/mips64-elf- ./configure
make install
```

## Building the ROM

The build process expects the game files to be present in the `game` directory.
You will need to provide these yourself. This project uses the MS-DOS version,
and should support either "Out Of This World" or "Another World" game files.
Only the `bank` files and `memlist.bin` are actually needed.

To build the ROM for yourself, run `buildN64.bash`
