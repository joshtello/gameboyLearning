# Game Boy Emulator

A Game Boy emulator written in C++ using SDL2 for rendering.

## Building

### Windows (MinGW)

```bash
g++ gameboy.cpp -I./x86_64-w64-mingw32/include/SDL2 -L./x86_64-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -o gameboy
```

## Running

```bash
./gameboy <ROM file>
```

## Features

- CPU emulation (WIP)
- Memory management
- PPU skeleton
- SDL2 display

## Roadmap

- [ ] Complete CPU instruction set (~500 opcodes)
- [ ] Memory Bank Controller (MBC) support
- [ ] PPU rendering (background & sprites)
- [ ] Timers
- [ ] Interrupts
- [ ] Joypad input

## Resources

- [Pan Docs](https://gbdev.io/pandocs/)
- [Opcode Table](https://gbdev.io/gb-opcodes/optables/)
- [Test ROMs](https://github.com/retrio/gb-test-roms)

