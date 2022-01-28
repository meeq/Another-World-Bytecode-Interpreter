# Another World for Nintendo 64

Another World is a platforming, shooting, puzzle-solving game from the early 1990's released on many home computer and video game console platforms.

The project is a port of the [`Another-World-Bytecode-Interpreter`](https://github.com/fabiensanglard/Another-World-Bytecode-Interpreter) game engine to N64 using the open source [LibDragon SDK](https://github.com/DragonMinded/libdragon). The SDL2 system implementation has been retained, and N64 has been added as an additional CMake `SYS_IMPLEMENTATION` backend option.

This port is still unfinished and experimental with known bugs. Contributions are welcome.

## Supported Versions

English PC DOS version is supported ("Out of this World"). You will need the original datafiles:

- `BANK*`
- `MEMLIST.BIN`

By default, the build scripts expect the datafiles to be in the `./game` directory.

## Compiling

### N64

Run `bash buildN64.bash` to build a release-optimized N64 ROM file. Inspect the build script for additional options.

This ROM file has been tested to work on real Nintendo 64 hardware using the EverDrive-64 by krikzz and 64drive by retroactive.

This ROM file should also be compatible with low-level, accuracy-focused Nintendo 64 emulators such as Ares, CEN64 and MAME.

#### Known issues

- No title screen or menu
- Saving is not implemented
- Audio doesn't sound right

### SDL2

Run `bash buildSDL2.bash` to build a native executable. Inspect the build script for additional options.

### Starting the game

To start the game, you can either:

- put the game's datafiles in the same directory as the executable
- use the `--datapath` command line option to specify the datafiles directory

### Controls

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
