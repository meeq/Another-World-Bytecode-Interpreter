# Another World for Nintendo 64

Another World is a platforming, shooting, puzzle-solving game from the early 1990's released on many home computer and video game console platforms.

The project is a port of the [`Another-World-Bytecode-Interpreter`](https://github.com/fabiensanglard/Another-World-Bytecode-Interpreter) game engine to N64 using the open source [LibDragon SDK](https://github.com/DragonMinded/libdragon). The parent project targets SDL2, but was intentionally abstracted so that alternate system implementations could be provided based on the `System` struct. In this fork, SDL2 support has been retained, and N64 has been added as an additional CMake `SYS_IMPLEMENTATION` backend option.

This port is still unfinished and experimental with known bugs. Contributions are welcome.

## Supported Versions

English PC DOS version is supported ("Out of this World"). You will need the original datafiles:

- `BANK*`
- `MEMLIST.BIN`

By default, the build scripts expect the datafiles to be in the `./game` directory.

## Compiling

### N64

#### Prerequisites

##### Install libdragon

See https://github.com/DragonMinded/libdragon for instructions.

##### Compile and install zlib for libdragon

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

#### Build the ROM

Run `bash buildN64.bash` to build a release-optimized N64 ROM file. Inspect the build script for additional options.

This ROM file has been tested to work on real Nintendo 64 hardware using the EverDrive-64 by krikzz and 64drive by retroactive.

This ROM file should also be compatible with low-level, accuracy-focused Nintendo 64 emulators such as Ares, CEN64 and MAME.

#### Known issues

- No title screen or menu
- Saving is not implemented
- Audio doesn't sound right

### SDL2

Run `bash buildSDL2.bash` to build a native executable. Inspect the build script for additional options.

#### Controls

- Arrow Keys      allow you to move Lester
- Enter/Space     allow you run/shoot with your gun
- C               allow to enter a code to jump at a specific level
- P               pause the game
- Alt X           exit the game
- Ctrl S          save game state
- Ctrl L          load game state
- Ctrl + and -    change game state slot
- Ctrl F          toggle fast mode
- TAB             change window scale factor

## Credits

- Eric Chahi, original creator of Another World
- Gregory Montoir, reverse-engineered the original engine
- Fabien Sanglard, additional reverse-engineering and code cleanup
