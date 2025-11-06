// Game Boy Emulator - Complete Skeleton
// Compile: g++ gameboy.cpp -I./SDL2/x86_64-w64-mingw32/include/SDL2 -L./SDL2/x86_64-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -o gameboy
// Run: ./gameboy rom.gb

#include <SDL.h>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdint>

// Game Boy screen is 160x144 pixels
const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;
const int SCALE = 4;

// Memory Map (simplified)
// 0x0000-0x3FFF: ROM Bank 0
// 0x4000-0x7FFF: ROM Bank 1+ (switchable)
// 0x8000-0x9FFF: VRAM
// 0xA000-0xBFFF: External RAM
// 0xC000-0xDFFF: Work RAM
// 0xE000-0xFDFF: Echo RAM
// 0xFE00-0xFE9F: OAM (sprites)
// 0xFF00-0xFF7F: I/O Registers
// 0xFF80-0xFFFE: High RAM
// 0xFFFF: Interrupt Enable

class Memory {
private:
    std::vector<uint8_t> rom;           // Cartridge ROM
    std::array<uint8_t, 0x2000> vram;   // Video RAM
    std::array<uint8_t, 0x2000> wram;   // Work RAM
    std::array<uint8_t, 0xA0> oam;      // Sprite attribute table
    std::array<uint8_t, 0x80> hram;     // High RAM
    std::array<uint8_t, 0x80> io;       // I/O registers
    
    uint8_t ie_register;                // Interrupt Enable

public:
    Memory() {
        vram.fill(0);
        wram.fill(0);
        oam.fill(0);
        hram.fill(0);
        io.fill(0);
        ie_register = 0;
    }
    
    bool loadROM(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open ROM: " << filename << std::endl;
            return false;
        }
        
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        rom.resize(size);
        file.read(reinterpret_cast<char*>(rom.data()), size);
        file.close();
        
        std::cout << "Loaded ROM: " << filename << " (" << size << " bytes)" << std::endl;
        return true;
    }
    
    uint8_t read(uint16_t addr) {
        // TODO: Implement memory mapping
        // This is where you handle different memory regions
        
        if (addr < 0x8000) {
            // ROM - TODO: Handle banking
            if (addr < rom.size()) return rom[addr];
            return 0xFF;
        }
        else if (addr >= 0x8000 && addr < 0xA000) {
            // VRAM
            return vram[addr - 0x8000];
        }
        else if (addr >= 0xC000 && addr < 0xE000) {
            // Work RAM
            return wram[addr - 0xC000];
        }
        else if (addr >= 0xFE00 && addr < 0xFEA0) {
            // OAM
            return oam[addr - 0xFE00];
        }
        else if (addr >= 0xFF00 && addr < 0xFF80) {
            // I/O Registers - TODO: Implement proper I/O handling
            return io[addr - 0xFF00];
        }
        else if (addr >= 0xFF80 && addr < 0xFFFF) {
            // High RAM
            return hram[addr - 0xFF80];
        }
        else if (addr == 0xFFFF) {
            // Interrupt Enable
            return ie_register;
        }
        
        return 0xFF;
    }
    
    void write(uint16_t addr, uint8_t value) {
        // TODO: Implement memory mapping for writes
        
        if (addr < 0x8000) {
            // ROM - TODO: Handle MBC (Memory Bank Controller)
            return;
        }
        else if (addr >= 0x8000 && addr < 0xA000) {
            // VRAM
            vram[addr - 0x8000] = value;
        }
        else if (addr >= 0xC000 && addr < 0xE000) {
            // Work RAM
            wram[addr - 0xC000] = value;
        }
        else if (addr >= 0xFE00 && addr < 0xFEA0) {
            // OAM
            oam[addr - 0xFE00] = value;
        }
        else if (addr >= 0xFF00 && addr < 0xFF80) {
            // I/O Registers - TODO: Implement proper I/O handling
            io[addr - 0xFF00] = value;
        }
        else if (addr >= 0xFF80 && addr < 0xFFFF) {
            // High RAM
            hram[addr - 0xFF80] = value;
        }
        else if (addr == 0xFFFF) {
            // Interrupt Enable
            ie_register = value;
        }
    }
};

class CPU {
private:
    // Registers
    struct {
        uint8_t a, f;  // Accumulator & Flags
        uint8_t b, c;
        uint8_t d, e;
        uint8_t h, l;
        uint16_t sp;   // Stack Pointer
        uint16_t pc;   // Program Counter
    } regs;
    
    Memory* memory;
    
    // Flag helpers
    void setFlag(uint8_t flag, bool value) {
        if (value) regs.f |= flag;
        else regs.f &= ~flag;
    }
    
    bool getFlag(uint8_t flag) {
        return (regs.f & flag) != 0;
    }

public:
    // Flag bits
    static const uint8_t FLAG_Z = 0x80;  // Zero
    static const uint8_t FLAG_N = 0x40;  // Subtract
    static const uint8_t FLAG_H = 0x20;  // Half Carry
    static const uint8_t FLAG_C = 0x10;  // Carry
    
    CPU(Memory* mem) : memory(mem) {
        reset();
    }
    
    void reset() {
        // Initial register values (after boot ROM)
        regs.a = 0x01;
        regs.f = 0xB0;
        regs.b = 0x00;
        regs.c = 0x13;
        regs.d = 0x00;
        regs.e = 0xD8;
        regs.h = 0x01;
        regs.l = 0x4D;
        regs.sp = 0xFFFE;
        regs.pc = 0x0100;  // Start after boot ROM
    }
    
    int step() {
        // Fetch opcode
        uint8_t opcode = memory->read(regs.pc++);
        
        // TODO: Decode and execute
        // Game Boy has ~500 opcodes!
        // Start with the most common ones
        
        switch (opcode) {
            case 0x00:  // NOP
                return 4;
                
            case 0x3E:  // LD A, n - Load immediate into A
                regs.a = memory->read(regs.pc++);
                return 8;
                
            case 0x06:  // LD B, n
                regs.b = memory->read(regs.pc++);
                return 8;
                
            case 0x0E:  // LD C, n
                regs.c = memory->read(regs.pc++);
                return 8;
                
            case 0xC3:  // JP nn - Unconditional jump
                {
                    uint16_t addr = memory->read(regs.pc++);
                    addr |= memory->read(regs.pc++) << 8;
                    regs.pc = addr;
                    return 16;
                }
            case 0x04:  // INC B
                {
                    uint8_t oldValue = regs.b;
                    regs.b++;

                    // Update flags
                    setFlag(FLAG_Z, regs.b == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (oldValue & 0x0F) + 1 > 0x0F);

                    return 4;
                }
            case 0x05: // DEC B
                {
                    uint8_t oldValue = regs.b;
                    regs.b--;

                    // Update flags
                    setFlag(FLAG_Z, regs.b == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) == 0x00);

                    return 4;
                }
            case 0x0C:
                { //INC C
                    uint8_t oldValue = regs.c;
                    regs.c++;

                    setFlag(FLAG_Z, regs.c == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (oldValue & 0x0F) + 1 > 0x0F);

                    return 4;

                }
            case 0x0D:
                { //DEC C
                    uint8_t oldValue = regs.c;
                    regs.c--;

                    setFlag(FLAG_Z, regs.c == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) == 0x00);

                    return 4;

                }
            case 0x14:
                { //INC D
                    uint8_t oldValue = regs.d;
                    regs.d++;
                    
                    setFlag(FLAG_Z, regs.d == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (oldValue & 0x0F) + 1 > 0x0F);  
                    return 4;
                }
            case 0x15:
                { //DEC D
                    uint8_t oldValue = regs.d;
                    regs.d--;

                    setFlag(FLAG_Z, regs.d == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) == 0x00);

                    return 4;

                }
            case 0x1C:
                { //INC E
                    uint8_t oldValue = regs.e;
                    regs.e++;
                    
                    setFlag(FLAG_Z, regs.e == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (oldValue & 0x0F) + 1 > 0x0F);  
                    return 4;
                }
            case 0x1D:
                { //DEC E
                    uint8_t oldValue = regs.e;
                    regs.e--;

                    setFlag(FLAG_Z, regs.e == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) == 0x00);

                    return 4;

                }
            case 0x24:
                { //INC H
                    uint8_t oldValue = regs.h;
                    regs.h++;
                    
                    setFlag(FLAG_Z, regs.h == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (oldValue & 0x0F) + 1 > 0x0F);  
                    return 4;
                }
            case 0x25:
                { //DEC H
                    uint8_t oldValue = regs.h;
                    regs.h--;

                    setFlag(FLAG_Z, regs.h == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) == 0x00);

                    return 4;

                }
            case 0x2C:
                { //INC L
                    uint8_t oldValue = regs.l;
                    regs.l++;
                    
                    setFlag(FLAG_Z, regs.l == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (oldValue & 0x0F) + 1 > 0x0F);  
                    return 4;
                }
            case 0x2D:
                { //DEC L
                    uint8_t oldValue = regs.l;
                    regs.l--;

                    setFlag(FLAG_Z, regs.l == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) == 0x00);

                    return 4;

                }
            case 0x34:
                {   //INC (HL)
                    std::cout << "INC (HL) not implemented yet" << std::endl;
                    return 12;
                }
            // TODO: Implement remaining ~495 opcodes!
            // Reference: https://gbdev.io/pandocs/CPU_Instruction_Set.html
            
            default:
                std::cout << "Unknown opcode: 0x" << std::hex << (int)opcode 
                         << " at PC: 0x" << (regs.pc - 1) << std::endl;
                return 4;
        }
    }
    
    // Helper to get 16-bit register pairs
    uint16_t getBC() { return (regs.b << 8) | regs.c; }
    uint16_t getDE() { return (regs.d << 8) | regs.e; }
    uint16_t getHL() { return (regs.h << 8) | regs.l; }
    
    void setBC(uint16_t val) { regs.b = val >> 8; regs.c = val & 0xFF; }
    void setDE(uint16_t val) { regs.d = val >> 8; regs.e = val & 0xFF; }
    void setHL(uint16_t val) { regs.h = val >> 8; regs.l = val & 0xFF; }
};

class PPU {
private:
    Memory* memory;
    std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT> framebuffer;
    
public:
    PPU(Memory* mem) : memory(mem) {
        framebuffer.fill(0xFFFFFFFF);  // White
    }
    
    void step(int cycles) {
        // TODO: Implement PPU timing and rendering
        // PPU operates in different modes:
        // Mode 2: OAM Scan (80 cycles)
        // Mode 3: Drawing (172 cycles)
        // Mode 0: H-Blank (204 cycles)
        // Mode 1: V-Blank (4560 cycles, 10 scanlines)
        
        // For now, just clear to white
    }
    
    const std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT>& getFramebuffer() {
        return framebuffer;
    }
};

class GameBoy {
private:
    Memory memory;
    CPU cpu;
    PPU ppu;
    
public:
    GameBoy() : cpu(&memory), ppu(&memory) {}
    
    bool loadROM(const std::string& filename) {
        return memory.loadROM(filename);
    }
    
    void step() {
        int cycles = cpu.step();
        ppu.step(cycles);
        // TODO: Update timers, handle interrupts
    }
    
    const std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT>& getScreen() {
        return ppu.getFramebuffer();
    }
};

// SDL Display
class Display {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    
public:
    Display() {
        SDL_Init(SDL_INIT_VIDEO);
        
        window = SDL_CreateWindow(
            "Game Boy Emulator",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE,
            SDL_WINDOW_SHOWN
        );
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        
        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH, SCREEN_HEIGHT
        );
    }
    
    ~Display() {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    void render(const std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT>& pixels) {
        SDL_UpdateTexture(texture, nullptr, pixels.data(), SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }
    
    GameBoy gameboy;
    Display display;
    
    if (!gameboy.loadROM(argv[1])) {
        return 1;
    }
    
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            // TODO: Handle joypad input
        }
        
        // Run some CPU cycles
        for (int i = 0; i < 1000; i++) {
            gameboy.step();
        }
        
        display.render(gameboy.getScreen());
        SDL_Delay(16);
    }
    
    return 0;
}

/*
=== YOUR ROADMAP ===

Phase 1: CPU (WEEKS 1-3)
- Implement all ~500 opcodes in CPU::step()
- Reference: https://gbdev.io/pandocs/CPU_Instruction_Set.html
- Test with Blargg's cpu_instrs test ROM
- This is the HARDEST part but teaches you the most

Phase 2: Memory (WEEK 4)
- Implement MBC1 (Memory Bank Controller)
- Handle ROM banking properly
- Most games use MBC1

Phase 3: PPU Basics (WEEK 5-6)
- Implement background rendering
- Implement sprite rendering
- Handle PPU timing and modes
- Goal: Get Tetris displaying

Phase 4: More Features (WEEK 7+)
- Timers
- Interrupts
- Joypad
- Goal: Get Tetris playable

=== RESOURCES ===

Essential:
- Pan Docs: https://gbdev.io/pandocs/
- Opcode table: https://gbdev.io/gb-opcodes/optables/
- Test ROMs: https://github.com/retrio/gb-test-roms

Optional but helpful:
- The Ultimate Game Boy Talk: https://www.youtube.com/watch?v=HyzD8pNlpwI
- Game Boy CPU Manual: http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf

=== FIRST STEPS ===

1. Start implementing CPU opcodes one by one
2. Follow the same pattern as Chip-8 (read spec → pseudocode → implement)
3. Test frequently with simple ROMs
4. When stuck, ask specific questions!

Good luck! This will be HARD but incredibly rewarding.
*/