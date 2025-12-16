// Game Boy Emulator - Complete Skeleton
// Compile: g++ gameboy.cpp -I./SDL2/x86_64-w64-mingw32/include/SDL2 -L./SDL2/x86_64-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -o gameboy
// Run: ./gameboy rom.gb

#include <SDL.h>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdint>
#include <cmath>


// Forward declarations
class Memory;
class APU;
class CPU;
class PPU;
class Timer;

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
    uint8_t if_register;                // Interrupt Flag
    uint8_t joypad_buttons;    // Buttons: START, SELECT, B, A
    uint8_t joypad_directions; // Directions: DOWN, UP, LEFT, RIGHT
    APU* apu;                          // Audio Processing Unit pointer

public:
    Memory() {
        vram.fill(0);
        wram.fill(0);
        oam.fill(0);
        hram.fill(0);
        io.fill(0);
        ie_register = 0;
        if_register = 0;
        joypad_buttons = 0x0F;    // All released (1 = not pressed)
        joypad_directions = 0x0F; // All released
        apu = nullptr;
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

    void incrementDIV() {
        io[0x04] = (io[0x04] + 1) & 0xFF;
    }
    
    void setAPU(APU* apu_ptr) { apu = apu_ptr; }
    
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
                // I/O Registers
            if (addr == 0xFF00) {
                uint8_t p1 = io[0x00];  // Now this will be 0x10 (not 0xD0)
                uint8_t result = p1 | 0xC0;  // Set bits 6-7: 0x10 | 0xC0 = 0xD0
              
    if (!(p1 & 0x20)) {  // Check bit 5 - button select
        result = (result & 0xF0) | (joypad_buttons & 0x0F);
    } 
    
    if (!(p1 & 0x10)) {  // ✅ Now this works! 0x10 & 0x10 = 0, so !0 = true
        result = (result & 0xF0) | (joypad_directions & 0x0F);
    }
    return result;

}
if (addr == 0xFF0F) {
            return if_register;
        }
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
        if (addr == 0xFF01) {
            static std::ofstream logfile("serial_log.txt");
            static std::string buffer;
            
            char c = (char)value;
            //std::cout << c << std::flush;
            
            // Log with hex values
            logfile << "Char: '" << c << "' (0x" << std::hex << (int)(uint8_t)value << ")" << std::dec << std::endl;
            
            buffer += c;
            
            // Check for common Blargg test completion patterns
            if (buffer.find("Passed") != std::string::npos && 
                buffer.find("\n\n") != std::string::npos) {
                std::cout << "\n\n=== TEST PASSED ===" << std::endl;
                logfile << "\n=== TEST PASSED ===" << std::endl;
                logfile << "Full buffer: " << buffer << std::endl;
                logfile.close();
                exit(0);
            }
            
            if (buffer.find("Failed") != std::string::npos) {
                std::cout << "\n\n=== TEST FAILED ===" << std::endl;
                logfile << "\n=== TEST FAILED ===" << std::endl;
                logfile << "Full buffer: " << buffer << std::endl;
                logfile.close();
                exit(1);
            }
            
            // Keep buffer manageable
            if (buffer.length() > 1000) {
                buffer = buffer.substr(buffer.length() - 1000);
            }
        }
        if (addr == 0xFF02) {
            // Serial Control register
            // Bit 7: Transfer Start Flag (0=No transfer, 1=Transfer in progress)
            if (value & 0x80) {
                // Transfer starts - we handle it immediately and clear bit 7
                // In a real Game Boy, this takes ~8 clock cycles, but for emulation
                // we can clear it immediately to allow the ROM to continue
                io[0x02] = value & 0x7F;  // Clear bit 7 (transfer complete)
            } else {
                // Normal write for other values (like 0x01, 0x00, etc.)
                io[0x02] = value;
            }
            return;
        }
        if (addr == 0xFF04){
            // Writing to DIV resets it
            io[0x04] = 0;
            return;
        }
        
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
            static bool first_oam_write = true;
            if (first_oam_write && value != 0) {
                std::cout << "\n!!! FIRST OAM WRITE !!!" << std::endl;
                std::cout << "Address: 0x" << std::hex << addr << std::dec << std::endl;
                std::cout << "Value: 0x" << std::hex << (int)value << std::dec << std::endl;
                first_oam_write = false;
            }
            oam[addr - 0xFE00] = value;
        }
        else if (addr >= 0xFF00 && addr < 0xFF80) {
            // I/O Registers
            if (addr == 0xFF00) {
                // P1/JOYP - Only bits 4-5 are writable (button/direction select)
                io[0x00] = (value & 0x30);
            return;
            }
            if (addr == 0xFF0F) {
                    if_register = value;
                    return;
                }
            if (addr >= 0xFF10 && addr <= 0xFF3F) {
                // Audio registers
                io[addr - 0xFF00] = value;
                return;
            }


            if (addr == 0xFF46) {
                    // DMA Transfer: Copy 160 bytes from XX00-XX9F to OAM (FE00-FE9F)
                    uint16_t source = value << 8;  // Value * 0x100
                    
                    //std::cout << "DMA Transfer from 0x" << std::hex << source 
                    //       << " to OAM" << std::dec << std::endl;
                    
                    for (int i = 0; i < 0xA0; i++) {
                        oam[i] = read(source + i);
                    }
                    io[0x46] = value;  // Store the DMA register value
                    return;
                }
            
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


    // Button presses (bit 0 = pressed, 1 = not pressed)
    void pressButton(int button) {
        joypad_buttons &= ~(1 << button);

        uint8_t if_flag = read(0xFF0F);
        write(0xFF0F, if_flag | 0x10);  // Set bit 4 (joypad interrupt)
    }

    void releaseButton(int button) {
        joypad_buttons |= (1 << button);
    }

    void pressDirection(int direction) {
        joypad_directions &= ~(1 << direction);
    }

    void releaseDirection(int direction) {
        joypad_directions |= (1 << direction);
        uint8_t if_flag = read(0xFF0F);
        write(0xFF0F, if_flag | 0x10);
    }
    
    // Button/Direction constants
    enum {
        BTN_A = 0,
        BTN_B = 1,
        BTN_SELECT = 2,
        BTN_START = 3,
        DIR_RIGHT = 0,
        DIR_LEFT = 1,
        DIR_UP = 2,
        DIR_DOWN = 3
    };




};

class Timer {
private:
    Memory* memory;
    int divider_counter;
    int timer_counter;
    
public:
    Timer(Memory* mem) : memory(mem), divider_counter(0), timer_counter(0) {}
    
    void step(int cycles) {
    // Update DIV register (increments at 16384 Hz)
    divider_counter += cycles;
    if (divider_counter >= 256) {
        divider_counter -= 256;
        memory->incrementDIV();  // ✅ Bypass the reset handler
    }
    
    // Check if timer is enabled
    uint8_t tac = memory->read(0xFF07);
    if (tac & 0x04) {
        timer_counter += cycles;
        
        int frequency = 0;
        switch (tac & 0x03) {
            case 0: frequency = 1024; break;
            case 1: frequency = 16;   break;
            case 2: frequency = 64;   break;
            case 3: frequency = 256;  break;
        }
        
        if (timer_counter >= frequency) {
            timer_counter -= frequency;
            
            uint8_t tima = memory->read(0xFF05);
            if (tima == 0xFF) {
                uint8_t tma = memory->read(0xFF06);
                memory->write(0xFF05, tma);
                
                uint8_t if_flag = memory->read(0xFF0F);
                memory->write(0xFF0F, if_flag | 0x04);
            } else {
                memory->write(0xFF05, tima + 1);
            }
        }
    }
}
};

class APU {
private:
    Memory* memory;
    
    // Channel 1: Square wave with sweep
    struct {
        bool enabled;
        int frequency;
        int duty;
        int volume;
        float phase;
    } ch1;
    
    // Channel 2: Square wave
    struct {
        bool enabled;
        int frequency;
        int duty;
        int volume;
        float phase;
    } ch2;
    
    // Audio state
    float sample_timer;
    static constexpr float SAMPLE_RATE = 44100.0f;
    static constexpr float GB_CLOCK = 4194304.0f;  // Game Boy CPU clock
    
public:
    APU(Memory* mem) : memory(mem) {
        ch1 = {};
        ch2 = {};
        sample_timer = 0.0f;
    }
    
    void step(int cycles) {

        uint8_t nr14 = memory->read(0xFF14);
        if (nr14 & 0x80) {
            updateChannel1();
        }
        
        // Check if channel 2 was just triggered
        uint8_t nr24 = memory->read(0xFF19);
        if (nr24 & 0x80) {
            updateChannel2();
        }
        for(int i = 0; i < cycles; i++) {
            
            ch1.phase += ch1.frequency / GB_CLOCK;
            ch2.phase += ch2.frequency / GB_CLOCK;
            
            // Keep phase in range [0, 1)
            if (ch1.phase >= 1.0f) ch1.phase -= 1.0f;
            if (ch2.phase >= 1.0f) ch2.phase -= 1.0f;
                }

        }
    
    float generateSample() {
        float sample = 0.0f;

        if (ch1.enabled) {
            sample += generateSquare(ch1.phase, ch1.duty) * (ch1.volume / 15.0f);
        }

        if (ch2.enabled) {
            sample += generateSquare(ch2.phase, ch2.duty) * (ch2.volume / 15.0f);
        }


        return sample * 0.5f;  // Simple normalization
    }

    void updateChannel1() {
        ch1.duty = (memory->read(0xFF11) >> 6) & 0x03;
        ch1.volume = (memory->read(0xFF12) >> 4) & 0x0F;
        
        uint8_t nr14 = memory->read(0xFF14);
        uint16_t freq_data = ((nr14 & 0x07) << 8) | memory->read(0xFF13);
        ch1.frequency = 131072 / (2048 - freq_data);
        
        // Check trigger bit (bit 7)
        if (nr14 & 0x80) {
            ch1.enabled = true;
            ch1.phase = 0.0f;
            
            // ✅ Clear the trigger bit (hardware does this automatically)
            memory->write(0xFF14, nr14 & 0x7F);
        }

    }
    
    void updateChannel2() {
        // NR21 (0xFF16): Duty and length [DD-- ----]
        ch2.duty = (memory->read(0xFF16) >> 6) & 0x03;
        
        // NR22 (0xFF17): Volume [VVVV EDDD]
        ch2.volume = (memory->read(0xFF17) >> 4) & 0x0F;
        
        // NR23 (0xFF18): Frequency LOW byte
        // NR24 (0xFF19): Trigger and frequency HIGH [T--- -HHH]
        uint8_t nr24 = memory->read(0xFF19);
        uint16_t freq_data = ((nr24 & 0x07) << 8) | memory->read(0xFF18);
        ch2.frequency = 131072 / (2048 - freq_data);
        
        // Check trigger bit (bit 7)
        if (nr24 & 0x80) {
            ch2.enabled = true;
            ch2.phase = 0.0f;
            
            // Clear the trigger bit
            memory->write(0xFF19, nr24 & 0x7F);
        }
    }
    
private:
    float generateSquare(float phase, int duty) {
        float duty_cycle = 0.5f;
        switch(duty) {
            case 0: duty_cycle = 0.125f; break;
            case 1: duty_cycle = 0.25f; break;
            case 2: duty_cycle = 0.5f; break;
            case 3: duty_cycle = 0.75f; break;
        }
        return (fmod(phase, 1.0f) < duty_cycle) ? 1.0f : -1.0f;
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
    bool ime; // Interrupt Master Enable
    bool halted;
    bool ei_pending;

    // Flag helpers
    void setFlag(uint8_t flag, bool value) {
        if (value) regs.f |= flag;
        else regs.f &= ~flag;
        regs.f &= 0xF0;  // ✅ Always mask lower 4 bits
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
        ime = false;
        halted = false;
        ei_pending = false;
    }
    
    int step() {
    
        
        // ✅ Handle EI delayed enable FIRST
        if (ei_pending) {
            ime = true;
            ei_pending = false;
        }
        
        // Handle HALT
        if (halted) {
            uint8_t ie = memory->read(0xFFFF);
            uint8_t if_flag = memory->read(0xFF0F);
            if (ie & if_flag) {
                halted = false;
            } else {
                return 4;
            }
        }
        
        // ✅ Handle interrupts (only if IME is enabled)
        if (ime) {
            uint8_t ie = memory->read(0xFFFF);
            uint8_t if_flag = memory->read(0xFF0F);
            uint8_t triggered = ie & if_flag;
            
            if (triggered) {
                ime = false;  // Disable interrupts
                
                // Service highest priority interrupt
                for (int i = 0; i < 5; i++) {
                    if (triggered & (1 << i)) {
                        
                        // Clear the interrupt flag
                        memory->write(0xFF0F, if_flag & ~(1 << i));
                        
                        // Push PC onto stack
                        memory->write(--regs.sp, (regs.pc >> 8) & 0xFF);
                        memory->write(--regs.sp, regs.pc & 0xFF);
                        
                        // Jump to interrupt vector
                        regs.pc = 0x0040 + (i * 8);
                        return 20;
                    }
                }
            }
        } else {
        // ✅ ADD THIS - See why interrupts aren't enabled
        static int debug_counter = 0;
        if (++debug_counter % 50000 == 0) {
            uint8_t ie = memory->read(0xFFFF);
            uint8_t if_flag = memory->read(0xFF0F);
        }
    }
    
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
                    setFlag(FLAG_Z, regs.b == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (regs.b & 0x0F) == 0x00);  // ✅ More explicit
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
                    setFlag(FLAG_H, (regs.c & 0x0F) == 0x00);  
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
                    setFlag(FLAG_H, (regs.d & 0x0F) == 0x00);
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
                    setFlag(FLAG_H, (regs.e & 0x0F) == 0x00); 
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
                    setFlag(FLAG_H, (regs.h & 0x0F) == 0x00);
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
                    setFlag(FLAG_H, (regs.l & 0x0F) == 0x00);
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
            case 0x34:  // INC (HL)
                {
                    uint16_t addr = getHL();
                    uint8_t value = memory->read(addr);
                    value++;
                    memory->write(addr, value);
                    
                    setFlag(FLAG_Z, value == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (value & 0x0F) == 0x00);  
                    return 12;
                }
            case 0x3C:
            {
                //INC A
                    uint8_t oldValue = regs.a;
                    regs.a++;
                    
                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, (regs.a & 0x0F) == 0x00);
                    return 4;          
            }
            case 0x3D:
            {
                //Dec A
                    uint8_t oldValue = regs.a;
                    regs.a--;
                    
                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) == 0x00);
                    return 4;          
            }
            //LD A, r8
            case 0x78: regs.a = regs.b; return 4;
            case 0x79: regs.a = regs.c; return 4;
            case 0x7A: regs.a = regs.d; return 4;
            case 0x7B: regs.a = regs.e; return 4;
            case 0x7C: regs.a = regs.h; return 4;
            case 0x7D: regs.a = regs.l; return 4;
            case 0x7E: regs.a = memory->read(getHL()); return 8;
            case 0x7F: regs.a = regs.a; return 4;
            //LD B, r8
            case 0x40: regs.b = regs.b; return 4;
            case 0x41: regs.b = regs.c; return 4;
            case 0x42: regs.b = regs.d; return 4;
            case 0x43: regs.b = regs.e; return 4;
            case 0x44: regs.b = regs.h; return 4;
            case 0x45: regs.b = regs.l; return 4;
            case 0x46: regs.b = memory->read(getHL()); return 8;
            case 0x47: regs.b = regs.a; return 4;
            //LD C,r8
            case 0x48: regs.c = regs.b; return 4;
            case 0x49: regs.c = regs.c; return 4;
            case 0x4A: regs.c = regs.d; return 4;
            case 0x4B: regs.c = regs.e; return 4;
            case 0x4C: regs.c = regs.h; return 4;
            case 0x4D: regs.c = regs.l; return 4;
            case 0x4E: regs.c = memory->read(getHL()); return 8;
            case 0x4F: regs.c = regs.a; return 4;
            //LD D, r8
            case 0x50: regs.d = regs.b; return 4;
            case 0x51: regs.d = regs.c; return 4;
            case 0x52: regs.d = regs.d; return 4;
            case 0x53: regs.d = regs.e; return 4;
            case 0x54: regs.d = regs.h; return 4;
            case 0x55: regs.d = regs.l; return 4;
            case 0x56: regs.d = memory->read(getHL()); return 8;
            case 0x57: regs.d = regs.a; return 4;
            //LD E, r8
            case 0x58: regs.e = regs.b; return 4;
            case 0x59: regs.e = regs.c; return 4;
            case 0x5A: regs.e = regs.d; return 4;
            case 0x5B: regs.e = regs.e; return 4;
            case 0x5C: regs.e = regs.h; return 4;
            case 0x5D: regs.e = regs.l; return 4;
            case 0x5E: regs.e = memory->read(getHL()); return 8;
            case 0x5F: regs.e = regs.a; return 4;
            //LD H, r8
            case 0x60: regs.h = regs.b; return 4;
            case 0x61: regs.h = regs.c; return 4;
            case 0x62: regs.h = regs.d; return 4;
            case 0x63: regs.h = regs.e; return 4;
            case 0x64: regs.h = regs.h; return 4;
            case 0x65: regs.h = regs.l; return 4;
            case 0x66: regs.h = memory->read(getHL()); return 8;
            case 0x67: regs.h = regs.a; return 4;
            //LD L, r8
            case 0x68: regs.l = regs.b; return 4;
            case 0x69: regs.l = regs.c; return 4;
            case 0x6A: regs.l = regs.d; return 4;
            case 0x6B: regs.l = regs.e; return 4;
            case 0x6C: regs.l = regs.h; return 4;
            case 0x6D: regs.l = regs.l; return 4;
            case 0x6E: regs.l = memory->read(getHL()); return 8;
            case 0x6F: regs.l = regs.a; return 4;
            //LD (HL), r8
            case 0x70: memory->write(getHL(), regs.b); return 8;
            case 0x71: memory->write(getHL(), regs.c); return 8;
            case 0x72: memory->write(getHL(), regs.d); return 8;
            case 0x73: memory->write(getHL(), regs.e); return 8;
            case 0x74: memory->write(getHL(), regs.h); return 8;
            case 0x75: memory->write(getHL(), regs.l); return 8;
            // 0x76 is HALT (not LD (HL), (HL))
            case 0x77: memory->write(getHL(), regs.a); return 8;

            case 0x80: //ADD A, B
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.b;
                    regs.a += regs.b;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.b & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4;   

                }
            case 0x81: //ADD A, C
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.c;
                    regs.a += regs.c;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.c & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4;   
                }
            case 0x82: // ADD A, D
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.d;
                    regs.a += regs.d;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.d & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x83: // ADD A, E
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.e;
                    regs.a += regs.e;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.e & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x84: // ADD A, H
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.h;
                    regs.a += regs.h;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.h & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x85: // ADD A, L
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.l;
                    regs.a += regs.l;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.l & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x86: // ADD A, HL
                {
                    uint8_t oldValue = regs.a;
                    uint8_t hL = memory->read(getHL());
                    uint16_t newValue = regs.a + hL;
                    regs.a += hL;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (hL & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 8; 
                }
            case 0x87: //ADD A, A
                { 
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.a;
                    regs.a += regs.a;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (oldValue & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x88: //ADC A, B
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.b + carry;
                    regs.a += regs.b + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.b & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x89: //ADC A, C
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.c + carry;
                    regs.a += regs.c + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.c & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x8A: //ADC A, D
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.d + carry;
                    regs.a += regs.d + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.d & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x8B: //ADC A, E
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.e + carry;
                    regs.a += regs.e + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.e & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x8C: //ADC A, H
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.h + carry;
                    regs.a += regs.h + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.h & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x8D: //ADC A, L
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.l + carry;
                    regs.a += regs.l + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (regs.l & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0x8E: //ADC A, HL
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t hL = memory->read(getHL());
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + hL + carry;
                    regs.a += hL + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (hL & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 8; 
                }
            case 0x8F: //ADC A, A
                {
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a + regs.a + carry;
                    regs.a += regs.a + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldValue & 0x0F) + (oldValue & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, newValue > 0xFF);
                    return 4; 
                }
            case 0xC9: // RET
                {
                    uint8_t low = memory->read(regs.sp++);
                    uint8_t high = memory->read(regs.sp++);
                    regs.pc = (high << 8) | low;
                    return 16;
                }
            case 0xCD: // CALL nn
                {
                    uint16_t addr = memory->read(regs.pc++);
                    addr |= memory->read(regs.pc++) << 8;
                    
                    // Push current PC onto stack
                    memory->write(--regs.sp, regs.pc >> 8);    // High byte
                    memory->write(--regs.sp, regs.pc & 0xFF);  // Low byte
                    
                    // Jump to address
                    regs.pc = addr;
                    
                    return 24;
                }
            case 0x18: //JR e
                {
                    int8_t offset = (int8_t)memory->read(regs.pc++);
                    regs.pc += offset;
                    return 12;
                }
            case 0x20: //JR NZ, e
                {
                    int8_t offset = (int8_t)memory->read(regs.pc++);
                    if (!getFlag(FLAG_Z)) {
                        regs.pc += offset;
                        return 12;
                    } else {
                        return 8;
                    }
                }
            case 0x28: //JR Z, e
                {
                    int8_t offset = (int8_t)memory->read(regs.pc++);
                    if (getFlag(FLAG_Z)) {
                        regs.pc += offset;
                        return 12;
                    } else {
                        return 8;
                    }
                }
            case 0x30: //JR NC, e
                {
                    int8_t offset = (int8_t)memory->read(regs.pc++);
                    if (!getFlag(FLAG_C)) {
                        regs.pc += offset;
                        return 12;
                    } else {
                        return 8;
                    }
                }
            case 0x38: //JR C, e
                {
                    int8_t offset = (int8_t)memory->read(regs.pc++);
                    if (getFlag(FLAG_C)) {
                        regs.pc += offset;
                        return 12;
                    } else {
                        return 8;
                    }
                }
            case 0xC2: // JP NZ, nn
                {
                    uint16_t addr = memory->read(regs.pc++);        // low byte
                    addr |= (memory->read(regs.pc++) << 8);
                    if (!getFlag(FLAG_Z)) {
                        regs.pc = addr;
                        return 16;
                    } else {
                        return 12;
                    }
                }
            case 0xCA: // JP Z, nn
                {
                    uint16_t addr = memory->read(regs.pc++);        // low byte
                    addr |= (memory->read(regs.pc++) << 8);
                    if (getFlag(FLAG_Z)) {
                        regs.pc = addr;
                        return 16;
                    } else {
                        return 12;
                    }
                }
            case 0xD2: // JP NC, nn
                {
                    uint16_t addr = memory->read(regs.pc++);        // low byte
                    addr |= (memory->read(regs.pc++) << 8);
                    if (!getFlag(FLAG_C)) {
                        regs.pc = addr;
                        return 16;
                    } else {
                        return 12;
                    }
                }
            case 0xDA: // JP C, nn
                {
                    uint16_t addr = memory->read(regs.pc++);        // low byte
                    addr |= (memory->read(regs.pc++) << 8);
                    if (getFlag(FLAG_C)) {
                        regs.pc = addr;
                        return 16;
                    } else {
                        return 12;
                    }
                }
            case 0x03: // INC BC
                {
                    uint16_t bc = getBC();
                    bc++;
                    setBC(bc);
                    return 8;
                }
            case 0x13: // INC DE
                {
                    uint16_t de = getDE();
                    de++;
                    setDE(de);
                    return 8;
                }
            case 0x23: // INC HL
                {
                    uint16_t hl = getHL();
                    hl++;
                    setHL(hl);
                    return 8;
                }
            case 0x33: // INC SP
                {
                    regs.sp++;
                    return 8;
                }
            case 0x0B: // DEC BC
                {
                    uint16_t bc = getBC();
                    bc--;
                    setBC(bc);
                    return 8;
                }
            case 0x1B: // DEC DE
                {
                    uint16_t de = getDE();
                    de--;
                    setDE(de);
                    return 8;
                }
            case 0x2B: // DEC HL
                {
                    uint16_t hl = getHL();
                    hl--;
                    setHL(hl);
                    return 8;
                }
            case 0x3B: // DEC SP
                {
                    regs.sp--;
                    return 8;
                }
            case 0x90: // SUB A, B
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.b;
                    regs.a -= regs.b;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.b & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.b);
                    return 4; 
                }
            case 0x91: // SUB A, C
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.c;
                    regs.a -= regs.c;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.c & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.c);
                    return 4; 
                }
            case 0x92: // SUB A, D
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.d;
                    regs.a -= regs.d;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.d & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.d);
                    return 4; 
                }
            case 0x93: // SUB A, E
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.e;
                    regs.a -= regs.e;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.e & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.e);
                    return 4; 
                }
            case 0x94: // SUB A, H
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.h;
                    regs.a -= regs.h;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.h & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.h);
                    return 4; 
                }
            case 0x95: // SUB A, L
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.l;
                    regs.a -= regs.l;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.l & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.l);
                    return 4; 
                }
            case 0x96: // SUB A, (HL)
                {
                    uint8_t hL = memory->read(getHL());
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - hL;
                    regs.a -= hL;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (hL & 0x0F));
                    setFlag(FLAG_C, oldValue < hL);
                    return 8; 
                }
            case 0x97: // SUB A, A
                {
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.a;
                    regs.a -= regs.a;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (oldValue & 0x0F));
                    setFlag(FLAG_C, oldValue < oldValue);
                    return 4; 
                }
            case 0x98: 
                { // SBC A, B
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.b - carry;
                    regs.a = regs.a - regs.b - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((regs.b & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (regs.b + carry));
                    return 4; 
                }
            case 0x99: 
                { // SBC A, C
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.c - carry;
                    regs.a = regs.a - regs.c - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((regs.c & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (regs.c + carry));
                    return 4; 
                }
            case 0x9A: 
                { // SBC A, D
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.d - carry;
                    regs.a = regs.a - regs.d - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((regs.d & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (regs.d + carry));
                    return 4; 
                }
            case 0x9B: 
                { // SBC A, E
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.e - carry;
                    regs.a = regs.a - regs.e - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((regs.e & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (regs.e + carry));
                    return 4; 
                }
            case 0x9C: 
                { // SBC A, H
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.h - carry;
                    regs.a = regs.a - regs.h - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((regs.h & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (regs.h + carry));
                    return 4; 
                }
            case 0x9D: 
                { // SBC A, L
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.l - carry;
                    regs.a = regs.a - regs.l - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((regs.l & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (regs.l + carry));
                    return 4; 
                }
            case 0x9E: 
                { // SBC A, (HL)
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t hL = memory->read(getHL());
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - hL - carry;
                    regs.a = regs.a - hL - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((hL & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (hL + carry));
                    return 8; 
                }
            case 0x9F: 
                { // SBC A, A
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldValue = regs.a;
                    uint16_t newValue = regs.a - regs.a - carry;
                    regs.a = regs.a - regs.a - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < ((oldValue & 0x0F) + carry));
                    setFlag(FLAG_C, oldValue < (oldValue + carry));
                    return 4; 
                }
            case 0xA0: // AND B
                {
                    regs.a = regs.a & regs.b;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA1: // AND C
                {
                    regs.a = regs.a & regs.c;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA2: // AND D
                {
                    regs.a = regs.a & regs.d;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA3: // AND E
                {
                    regs.a = regs.a & regs.e;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA4: // AND H
                {
                    regs.a = regs.a & regs.h;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA5: // AND L
                {
                    regs.a = regs.a & regs.l;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA6: // AND (HL)
                {
                    uint8_t hL = memory->read(getHL());
                    regs.a = regs.a & hL;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 8; 
                }
            case 0xA7: // AND A
                {
                    regs.a = regs.a & regs.a;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA8: // XOR B
                {
                    regs.a = regs.a ^ regs.b;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xA9: // XOR C
                {
                    regs.a = regs.a ^ regs.c;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xAA: // XOR D
                {
                    regs.a = regs.a ^ regs.d;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xAB: // XOR E
                {
                    regs.a = regs.a ^ regs.e;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xAC: // XOR H
                {
                    regs.a = regs.a ^ regs.h;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xAD: // XOR L
                {
                    regs.a = regs.a ^ regs.l;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xAE: // XOR (HL)
                {
                    uint8_t hL = memory->read(getHL());
                    regs.a = regs.a ^ hL;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 8; 
                }
            case 0xAF: // XOR A
                {
                    regs.a = regs.a ^ regs.a;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB0: // OR B
                {
                    regs.a = regs.a | regs.b;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB1: // OR C
                {
                    regs.a = regs.a | regs.c;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB2: // OR D
                {
                    regs.a = regs.a | regs.d;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB3: // OR E
                {
                    regs.a = regs.a | regs.e;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB4: // OR H
                {
                    regs.a = regs.a | regs.h;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB5: // OR L
                {
                    regs.a = regs.a | regs.l;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB6: // OR (HL)
                {
                    uint8_t hL = memory->read(getHL());
                    regs.a = regs.a | hL;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 8; 
                }
            case 0xB7: // OR A
                {
                    regs.a = regs.a | regs.a;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 4; 
                }
            case 0xB8: // CP B
                {
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - regs.b;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.b & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.b);
                    return 4;
                }
            case 0xB9: // CP C
                {
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - regs.c;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.c & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.c);
                    return 4;
                }
            case 0xBA: // CP D
                {
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - regs.d;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.d & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.d);
                    return 4;
                }
            case 0xBB: // CP E
                {
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - regs.e;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.e & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.e);
                    return 4;
                }
            case 0xBC: // CP H
                {
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - regs.h;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.h & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.h);
                    return 4;
                }
            case 0xBD: // CP L
                {
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - regs.l;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (regs.l & 0x0F));
                    setFlag(FLAG_C, oldValue < regs.l);
                    return 4;
                }
            case 0xBE: // CP (HL)
                {
                    uint8_t hL = memory->read(getHL());
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - hL;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (hL & 0x0F));
                    setFlag(FLAG_C, oldValue < hL);
                    return 8;
                }
            case 0xBF: // CP A
                {
                    uint8_t oldValue = regs.a;
                    uint8_t result = regs.a - regs.a;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldValue & 0x0F) < (oldValue & 0x0F));
                    setFlag(FLAG_C, oldValue < oldValue);
                    return 4;
                }
            case 0x02: // LD (BC), A
                {
                    memory->write(getBC(), regs.a);
                    return 8;
                }
            case 0x12: // LD (DE), A
                {
                    memory->write(getDE(), regs.a);
                    return 8;
                }
            case 0x22: // LD (HL+), A
                {
                    memory->write(getHL(), regs.a);
                    uint16_t hL = getHL(); 
                    hL++;
                    setHL(hL);
                    return 8;
                }
            case 0x32: // LD (HL-), A
                {
                    memory->write(getHL(), regs.a);
                    uint16_t hL = getHL(); 
                    hL--;
                    setHL(hL);
                    return 8;
                }
            case 0x0A: // LD A, (BC)
                {
                    regs.a = memory->read(getBC());
                    return 8;
                }
            case 0x1A: // LD A, (DE)
                {
                    regs.a = memory->read(getDE());
                    return 8;
                }
            case 0x2A: // LD A, (HL+)
                {
                    regs.a = memory->read(getHL());
                    uint16_t hL = getHL(); 
                    hL++;
                    setHL(hL);
                    return 8;
                }  
            case 0x3A: // LD A, (HL-)
                {
                    regs.a = memory->read(getHL());
                    uint16_t hL = getHL(); 
                    hL--;
                    setHL(hL);
                    return 8;
                }
            case 0xC1: // POP BC
                {
                    uint8_t low = memory->read(regs.sp++);
                    uint8_t high = memory->read(regs.sp++);
                    regs.c = low;
                    regs.b = high;
                    return 12;
                }
            case 0xD1: // POP DE
                {
                    uint8_t low = memory->read(regs.sp++);
                    uint8_t high = memory->read(regs.sp++);
                    regs.e = low;
                    regs.d = high;
                    return 12;
                }
            case 0xE1: // POP HL
                {
                    uint8_t low = memory->read(regs.sp++);
                    uint8_t high = memory->read(regs.sp++);
                    regs.l = low;
                    regs.h = high;
                    return 12;
                }
            case 0xF1: // POP AF
                {
                    regs.f = memory->read(regs.sp++) & 0xF0;  // ✅ Mask lower 4 bits!
                    regs.a = memory->read(regs.sp++);
                    return 12;
                }
            case 0xC5: // PUSH BC
                {
                    memory->write(--regs.sp, regs.b);  // High byte
                    memory->write(--regs.sp, regs.c);  // Low byte
                    return 16;
                }
            case 0xD5: // PUSH DE
                {
                    memory->write(--regs.sp, regs.d);  // High byte
                    memory->write(--regs.sp, regs.e);  // Low byte
                    return 16;
                }
            case 0xE5: // PUSH HL
                {
                    memory->write(--regs.sp, regs.h);  // High byte
                    memory->write(--regs.sp, regs.l);  // Low byte
                    return 16;
                }
            case 0xF5: // PUSH AF
                {
                    memory->write(--regs.sp, regs.a);
                    memory->write(--regs.sp, regs.f & 0xF0);  // ✅ Mask when pushing too
                    return 16;
                }
            case 0x07: //RLCA
                {
                    uint8_t oldA = regs.a;
                    regs.a = (regs.a << 1) | (oldA >> 7);
                    setFlag(FLAG_Z, false);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, (oldA & 0x80) != 0);
                    return 4;
                }
            case 0x0F: //RRCA
                {
                    uint8_t oldA = regs.a;
                    regs.a = (regs.a >> 1) | (oldA << 7);
                    setFlag(FLAG_Z, false);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, (oldA & 0x01) != 0);
                    return 4;
                }
            case 0x17: //RLA
                {
                    uint8_t oldA = regs.a;
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    regs.a = (regs.a << 1) | carry;
                    setFlag(FLAG_Z, false);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, (oldA & 0x80) != 0);
                    return 4;
                }
            case 0x1F: //RRA
                {
                    uint8_t oldA = regs.a;
                    uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                    regs.a = (regs.a >> 1) | carry;
                    setFlag(FLAG_Z, false);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, (oldA & 0x01) != 0);
                    return 4;
                }
            case 0x01: // LD BC, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    regs.c = low;
                    regs.b = high;
                    return 12;
                }
            case 0x11: // LD DE, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    regs.e = low;
                    regs.d = high;
                    return 12;
                }
            case 0x21: // LD HL, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    regs.l = low;
                    regs.h = high;
                    return 12;
                }
            case 0x31: // LD SP, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    regs.sp = (high << 8) | low;
                    return 12;
                }
            case 0xC6: // ADD A, n
                {
                    uint8_t value = memory->read(regs.pc++);
                    uint8_t oldA = regs.a;
                    uint16_t result = regs.a + value;
                    regs.a += value;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldA & 0x0F) + (value & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, result > 0xFF);
                    return 8;
                }
            case 0xCE: // ADC A, n
                {
                    uint8_t value = memory->read(regs.pc++);
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldA = regs.a;
                    uint16_t result = regs.a + value + carry;
                    regs.a += value + carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((oldA & 0x0F) + (value & 0x0F) + carry) > 0x0F);
                    setFlag(FLAG_C, result > 0xFF);
                    return 8;
                }
            case 0xD6: // SUB n
                {
                    uint8_t value = memory->read(regs.pc++);
                    uint8_t oldA = regs.a;
                    uint16_t result = regs.a - value;
                    regs.a -= value;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldA & 0x0F) < (value & 0x0F));
                    setFlag(FLAG_C, oldA < value);
                    return 8;
                }
            case 0xDE: // SBC A, n
                {
                    uint8_t value = memory->read(regs.pc++);
                    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                    uint8_t oldA = regs.a;
                    uint16_t result = regs.a - value - carry;
                    regs.a = regs.a - value - carry;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldA & 0x0F) < ((value & 0x0F) + carry));
                    setFlag(FLAG_C, oldA < (value + carry));
                    return 8;
                }
            case 0xE6: // AND n
                {
                    uint8_t value = memory->read(regs.pc++);
                    regs.a = regs.a & value;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, true);
                    setFlag(FLAG_C, false);
                    return 8;
                }
            case 0xEE: // XOR n
                {
                    uint8_t value = memory->read(regs.pc++);
                    regs.a = regs.a ^ value;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 8;
                }
            case 0xF6: // OR n
                {
                    uint8_t value = memory->read(regs.pc++);
                    regs.a = regs.a | value;

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, false);
                    return 8;
                }
            case 0xFE: // CP n
                {
                    uint8_t value = memory->read(regs.pc++);
                    uint8_t oldA = regs.a;
                    uint8_t result = regs.a - value;

                    setFlag(FLAG_Z, result == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (oldA & 0x0F) < (value & 0x0F));
                    setFlag(FLAG_C, oldA < value);
                    return 8;
                }
            case 0xC7: // RST 00H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x00;
                    return 16;
                }
            case 0xCF: // RST 08H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x08;
                    return 16;
                }
            case 0xD7: // RST 10H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x10;
                    return 16;
                }
            case 0xDF: // RST 18H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x18;
                    return 16;
                }
            case 0xE7: // RST 20H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x20;
                    return 16;
                }
            case 0xEF: // RST 28H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x28;
                    return 16;
                }
            case 0xF7: // RST 30H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x30;
                    return 16;
                }
            case 0xFF: // RST 38H
                {
                    memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                    memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                    regs.pc = 0x38;
                    return 16;
                }
            case 0xF0:
                { // LDH A, (n)
                    uint8_t offset = memory->read(regs.pc++);
                    regs.a = memory->read(0xFF00 + offset);
                    return 12;
                }
            case 0xE0:
                { // LDH (n), A
                    uint8_t offset = memory->read(regs.pc++);
                    memory->write(0xFF00 + offset, regs.a);
                    return 12;
                }
            case 0xC0:
                { // RET NZ
                    if (!getFlag(FLAG_Z)) {
                        uint8_t low = memory->read(regs.sp++);
                        uint8_t high = memory->read(regs.sp++);
                        regs.pc = (high << 8) | low;
                        return 20;
                    }
                    return 8;
                }
            case 0xC8:
                { // RET Z
                    if (getFlag(FLAG_Z)) {
                        uint8_t low = memory->read(regs.sp++);
                        uint8_t high = memory->read(regs.sp++);
                        regs.pc = (high << 8) | low;
                        return 20;
                    }
                    return 8;
                }
            case 0xD0:
                { // RET NC
                    if (!getFlag(FLAG_C)) {
                        uint8_t low = memory->read(regs.sp++);
                        uint8_t high = memory->read(regs.sp++);
                        regs.pc = (high << 8) | low;
                        return 20;
                    }
                    return 8;
                }
            case 0xD8:
                { // RET C
                    if (getFlag(FLAG_C)) {
                        uint8_t low = memory->read(regs.sp++);
                        uint8_t high = memory->read(regs.sp++);
                        regs.pc = (high << 8) | low;
                        return 20;
                    }
                    return 8;
                }
            case 0xEA:
                { // LD (nn), A
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    uint16_t addr = (high << 8) | low;
                    memory->write(addr, regs.a);
                    return 16;
                }
            case 0xFA:
                { // LD A, (nn)
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    uint16_t addr = (high << 8) | low;
                    regs.a = memory->read(addr);
                    return 16;
                }
            case 0x26: // LD H, n
                {
                    uint8_t value = memory->read(regs.pc++);
                    regs.h = value;
                    return 8;
                }
            case 0xC4: // CALL NZ, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    uint16_t addr = (high << 8) | low;
                    if (!getFlag(FLAG_Z)) {
                        memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                        memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                        regs.pc = addr;
                        return 24;
                    }
                    return 12;
                }
            case 0xF3:
                { // DI
                    ime = false;
                    return 4;
                }
            case 0xFB:
                { // EI
                    ei_pending = true;
                    return 4;
                }
            case 0x1E: // LD E, n
                {
                    uint8_t value = memory->read(regs.pc++);
                    regs.e = value;
                    return 8;
                }
            case 0xE2:
                { // LD (C), A
                    memory->write(0xFF00 + regs.c, regs.a);
                    return 8;
                }
            case 0xF2:
                { // LD A, (C)
                    regs.a = memory->read(0xFF00 + regs.c);
                    return 8;
                }
            case 0x19: // ADD HL, DE
                {
                    uint16_t hl = getHL();
                    uint16_t de = getDE();
                    uint32_t result = hl + de;
                    setHL(result & 0xFFFF);

                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((hl & 0x0FFF) + (de & 0x0FFF)) > 0x0FFF);
                    setFlag(FLAG_C, result > 0xFFFF);
                    return 8;
                }
            case 0x35: // DEC (HL)
                {
                    uint16_t hL = getHL();
                    uint8_t value = memory->read(hL);
                    value--;  // Decrement first
                    memory->write(hL, value);

                    setFlag(FLAG_Z, value == 0);
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, (value & 0x0F) == 0x0F);  // ✅ After decrement, check if wrapped to 0x0F
                    return 12;
                }
            case 0xCB: // CB Prefix
                {
                    uint8_t cb_opcode = memory->read(regs.pc++);  // Read next byte
                    return executeCB(cb_opcode);  // Handle CB instruction
                }
            case 0x29: // ADD HL, HL
                {
                    uint16_t hl = getHL();
                    uint32_t result = hl + hl;
                    setHL(result & 0xFFFF);

                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((hl & 0x0FFF) + (hl & 0x0FFF)) > 0x0FFF);
                    setFlag(FLAG_C, result > 0xFFFF);
                    return 8;
                }
            case 0xE9:
                { // JP (HL)
                    regs.pc = getHL();
                    return 4;
                }
            case 0xF8: // LD HL, SP+n
                {
                    int8_t offset = static_cast<int8_t>(memory->read(regs.pc++));
                    uint16_t sp = regs.sp;
                    uint16_t result = sp + offset;
                    setHL(result & 0xFFFF);

                    setFlag(FLAG_Z, false);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((sp & 0x0F) + (offset & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, ((sp & 0xFF) + (offset & 0xFF)) > 0xFF);
                    return 12;
                }
            case 0xF9: // LD SP, HL
                {
                    regs.sp = getHL();
                    return 8;
                }
            case 0x08: // LD (nn), SP
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    uint16_t addr = (high << 8) | low;
                    memory->write(addr, regs.sp & 0xFF);         // Low byte
                    memory->write(addr + 1, (regs.sp >> 8) & 0xFF); // High byte
                    return 20;
                }
            case 0x39: // ADD HL, SP
                {
                    uint16_t hl = getHL();
                    uint16_t sp = regs.sp;
                    uint32_t result = hl + sp;
                    setHL(result & 0xFFFF);

                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((hl & 0x0FFF) + (sp & 0x0FFF)) > 0x0FFF);
                    setFlag(FLAG_C, result > 0xFFFF);
                    return 8;
                }
            case 0x2E: // LD L, n
                {
                    uint8_t value = memory->read(regs.pc++);
                    regs.l = value;
                    return 8;
                }
            case 0x10: // STOP
                {
                    regs.pc++;
                    return 4;
                }
            case 0xE8:
                { // ADD SP, n
                    int8_t offset = static_cast<int8_t>(memory->read(regs.pc++));
                    uint16_t sp = regs.sp;
                    uint16_t result = sp + offset;
                    regs.sp = result & 0xFFFF;

                    setFlag(FLAG_Z, false);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((sp & 0x0F) + (offset & 0x0F)) > 0x0F);
                    setFlag(FLAG_C, ((sp & 0xFF) + (offset & 0xFF)) > 0xFF);
                    return 16;
                }
            case 0x36: // LD (HL), n
                {
                    uint8_t value = memory->read(regs.pc++);
                    uint16_t hL = getHL();
                    memory->write(hL, value);
                    return 12;
                }
            case 0x16: // LD D, n
                {
                    uint8_t value = memory->read(regs.pc++);
                    regs.d = value;
                    return 8;
                }
            case 0x09: // ADD HL, BC
                {
                    uint16_t hl = getHL();
                    uint16_t bc = getBC();
                    uint32_t result = hl + bc;
                    setHL(result & 0xFFFF);

                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, ((hl & 0x0FFF) + (bc & 0x0FFF)) > 0x0FFF);
                    setFlag(FLAG_C, result > 0xFFFF);
                    return 8;
                }
            case 0x27: // DAA
                {
                    uint8_t correction = 0;
                    bool setC = getFlag(FLAG_C);

                    if (getFlag(FLAG_H) || (!getFlag(FLAG_N) && (regs.a & 0x0F) > 0x09)) {
                        correction |= 0x06;
                    }
                    if (setC || (!getFlag(FLAG_N) && regs.a > 0x99)) {
                        correction |= 0x60;
                        setC = true;
                    }

                    if (getFlag(FLAG_N)) {
                        regs.a -= correction;
                    } else {
                        regs.a += correction;
                    }

                    setFlag(FLAG_Z, regs.a == 0);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, setC);
                    return 4;
                }
            case 0x2F: // CPL
                {
                    regs.a = ~regs.a;
                    setFlag(FLAG_N, true);
                    setFlag(FLAG_H, true);
                    return 4;
                }
            case 0x37: // SCF
                {
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, true);
                    return 4;
                }
            case 0x3F: // CCF
                {
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, !getFlag(FLAG_C));
                    return 4;
                }
            case 0x76: // HALT
                {
                    halted = true;
                    return 4;
                }
            case 0xCC: // CALL Z, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    uint16_t addr = (high << 8) | low;
                    if (getFlag(FLAG_Z)) {
                        memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                        memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                        regs.pc = addr;
                        return 24;
                    }
                    return 12;
                }
            case 0xD4: // CALL NC, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    uint16_t addr = (high << 8) | low;
                    if (!getFlag(FLAG_C)) {
                        memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                        memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                        regs.pc = addr;
                        return 24;
                    }
                    return 12;
                }
            case 0xDC: // CALL C, nn
                {
                    uint8_t low = memory->read(regs.pc++);
                    uint8_t high = memory->read(regs.pc++);
                    uint16_t addr = (high << 8) | low;
                    if (getFlag(FLAG_C)) {
                        memory->write(--regs.sp, (regs.pc >> 8) & 0xFF); // Push high byte of PC
                        memory->write(--regs.sp, regs.pc & 0xFF);        // Push low byte of PC
                        regs.pc = addr;
                        return 24;
                    }
                    return 12;
                }
            case 0xD9:
                { // RETI
                    uint8_t low = memory->read(regs.sp++);
                    uint8_t high = memory->read(regs.sp++);
                    regs.pc = (high << 8) | low;
                    ime = true; // Enable interrupts after return
                    return 16;
                }
            // TODO: Implement remaining ~495 opcodes!
            // Reference: https://gbdev.io/pandocs/CPU_Instruction_Set.html
            
           default:
                std::cout << "Unknown opcode: 0x" << std::hex << (int)opcode 
                        << " at PC: 0x" << (regs.pc - 1) << std::endl;
                std::cout << "Registers - A:" << (int)regs.a << " F:" << (int)regs.f 
                        << " B:" << (int)regs.b << " C:" << (int)regs.c << std::endl;
                exit(1);  // Stop immediately
                return 4;
        }
    }

    int executeCB(uint8_t opcode) {
    switch(opcode) {
        case 0x00: // RLC B
            {
                uint8_t oldB = regs.b;
                regs.b = (regs.b << 1) | (oldB >> 7);
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldB & 0x80);
                return 8;
            }
        case 0x01: // RLC C
            {
                uint8_t oldC = regs.c;
                regs.c = (regs.c << 1) | (oldC >> 7);
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldC & 0x80);
                return 8;
            }
        case 0x02: // RLC D
            {
                uint8_t oldD = regs.d;
                regs.d = (regs.d << 1) | (oldD >> 7);
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldD & 0x80);
                return 8;
            }
        case 0x03: // RLC E
            {
                uint8_t oldE = regs.e;
                regs.e = (regs.e << 1) | (oldE >> 7);
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldE & 0x80);
                return 8;
            }
        case 0x04: // RLC H
            {
                uint8_t oldH = regs.h;
                regs.h = (regs.h << 1) | (oldH >> 7);
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldH & 0x80);
                return 8;
            }
        case 0x05: // RLC L
            {
                uint8_t oldL = regs.l;
                regs.l = (regs.l << 1) | (oldL >> 7);
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldL & 0x80);
                return 8;
            }
        case 0x06: // RLC (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t newValue = (oldValue << 1) | (oldValue >> 7);
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldValue & 0x80);
                return 16;
            }
        case 0x07: // RLC A
            {
                uint8_t oldA = regs.a;
                regs.a = (regs.a << 1) | (oldA >> 7);
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldA & 0x80);
                return 8;
            }   
        case 0x08:    
            { // RRC B
                uint8_t oldB = regs.b;
                regs.b = (regs.b >> 1) | (oldB << 7);
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldB & 0x01);
                return 8;
            }
        case 0x09:    // RRC C
            {
                uint8_t oldC = regs.c;
                regs.c = (regs.c >> 1) | (oldC << 7);
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldC & 0x01);
                return 8;
            }
        case 0x0A:    // RRC D
            {
                uint8_t oldD = regs.d;
                regs.d = (regs.d >> 1) | (oldD << 7);
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldD & 0x01);
                return 8;
            }
        case 0x0B:    // RRC E
            {
                uint8_t oldE = regs.e;
                regs.e = (regs.e >> 1) | (oldE << 7);
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldE & 0x01);
                return 8;
            }
        case 0x0C:    // RRC H
            {
                uint8_t oldH = regs.h;
                regs.h = (regs.h >> 1) | (oldH << 7);
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldH & 0x01);
                return 8;
            }
        case 0x0D:    // RRC L
            {
                uint8_t oldL = regs.l;
                regs.l = (regs.l >> 1) | (oldL << 7);
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldL & 0x01);
                return 8;
            }
        case 0x0E:    // RRC (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t newValue = (oldValue >> 1) | (oldValue << 7);
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldValue & 0x01);
                return 16;
            }
        case 0x0F:    // RRC A
            {
                uint8_t oldA = regs.a;
                regs.a = (regs.a >> 1) | (oldA << 7);
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldA & 0x01);
                return 8;
            }
        case 0x10:   // RL B
            {
                uint8_t oldB = regs.b;
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                regs.b = (regs.b << 1) | carry;
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldB & 0x80);
                return 8;
            }
        case 0x11:   // RL C
            {
                uint8_t oldC = regs.c;
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                regs.c = (regs.c << 1) | carry;
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldC & 0x80);
                return 8;
            }
        case 0x12:   // RL D
            {
                uint8_t oldD = regs.d;
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                regs.d = (regs.d << 1) | carry;
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldD & 0x80);
                return 8;
            }
        case 0x13:   // RL E
            {
                uint8_t oldE = regs.e;
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                regs.e = (regs.e << 1) | carry;
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldE & 0x80);
                return 8;
            }
        case 0x14:   // RL H
            {
                uint8_t oldH = regs.h;
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                regs.h = (regs.h << 1) | carry;
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldH & 0x80);
                return 8;
            }
        case 0x15:   // RL L
            {
                uint8_t oldL = regs.l;
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                regs.l = (regs.l << 1) | carry;
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldL & 0x80);
                return 8;
            }
        case 0x16:   // RL (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                uint8_t newValue = (oldValue << 1) | carry;
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldValue & 0x80);
                return 16;
            }
        case 0x17:   // RL A
            {
                uint8_t oldA = regs.a;
                uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
                regs.a = (regs.a << 1) | carry;
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldA & 0x80);
                return 8;
            }
        case 0x18:  // RR B
            {
                uint8_t oldB = regs.b;
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                regs.b = (regs.b >> 1) | carry;
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldB & 0x01);
                return 8;
            }
        case 0x19:  // RR C
            {
                uint8_t oldC = regs.c;
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                regs.c = (regs.c >> 1) | carry;
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldC & 0x01);
                return 8;
            }
        case 0x1A:  // RR D
            {
                uint8_t oldD = regs.d;
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                regs.d = (regs.d >> 1) | carry;
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldD & 0x01);
                return 8;
            }
        case 0x1B:  // RR E
            {
                uint8_t oldE = regs.e;
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                regs.e = (regs.e >> 1) | carry;
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldE & 0x01);
                return 8;
            }
        case 0x1C:  // RR H
            {
                uint8_t oldH = regs.h;
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                regs.h = (regs.h >> 1) | carry;
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldH & 0x01);
                return 8;
            }
        case 0x1D:  // RR L
            {
                uint8_t oldL = regs.l;
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                regs.l = (regs.l >> 1) | carry;
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldL & 0x01);
                return 8;
            }
        case 0x1E:  // RR (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                uint8_t newValue = (oldValue >> 1) | carry;
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldValue & 0x01);
                return 16;
            }
        case 0x1F:  // RR A
            {
                uint8_t oldA = regs.a;
                uint8_t carry = getFlag(FLAG_C) ? 0x80 : 0;
                regs.a = (regs.a >> 1) | carry;
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldA & 0x01);
                return 8;
            }
        case 0x20: // SLA B
            {
                uint8_t oldB = regs.b;
                regs.b = (regs.b << 1);
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldB & 0x80);
                return 8;
            }
        case 0x21: // SLA C
            {
                uint8_t oldC = regs.c;
                regs.c = (regs.c << 1);
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldC & 0x80);
                return 8;
            }
        case 0x22: // SLA D
            {
                uint8_t oldD = regs.d;
                regs.d = (regs.d << 1);
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldD & 0x80);
                return 8;
            }
        case 0x23: // SLA E
            {
                uint8_t oldE = regs.e;
                regs.e = (regs.e << 1);
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldE & 0x80);
                return 8;
            }
        case 0x24: // SLA H
            {
                uint8_t oldH = regs.h;
                regs.h = (regs.h << 1);
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldH & 0x80);
                return 8;
            }
        case 0x25: // SLA L
            {
                uint8_t oldL = regs.l;
                regs.l = (regs.l << 1);
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldL & 0x80);
                return 8;
            }
        case 0x26: // SLA (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t newValue = (oldValue << 1);
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldValue & 0x80);
                return 16;
            }
        case 0x27: // SLA A
            {
                uint8_t oldA = regs.a;
                regs.a = (regs.a << 1);
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldA & 0x80);
                return 8;
            }
        case 0x28: // SRA B
            {
                uint8_t oldB = regs.b;
                regs.b = (regs.b >> 1) | (oldB & 0x80);
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldB & 0x01);
                return 8;
            }
        case 0x29: // SRA C
            {
                uint8_t oldC = regs.c;
                regs.c = (regs.c >> 1) | (oldC & 0x80);
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldC & 0x01);
                return 8;
            }
        case 0x2A: // SRA D
            {
                uint8_t oldD = regs.d;
                regs.d = (regs.d >> 1) | (oldD & 0x80);
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldD & 0x01);
                return 8;
            }
        case 0x2B: // SRA E
            {
                uint8_t oldE = regs.e;
                regs.e = (regs.e >> 1) | (oldE & 0x80);
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldE & 0x01);
                return 8;
            }
        case 0x2C: // SRA H
            {
                uint8_t oldH = regs.h;
                regs.h = (regs.h >> 1) | (oldH & 0x80);
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldH & 0x01);
                return 8;
            }
        case 0x2D: // SRA L
            {
                uint8_t oldL = regs.l;
                regs.l = (regs.l >> 1) | (oldL & 0x80);
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldL & 0x01);
                return 8;
            }
        case 0x2E: // SRA (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t newValue = (oldValue >> 1) | (oldValue & 0x80);
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldValue & 0x01);
                return 16;
            }
        case 0x2F: // SRA A
            {
                uint8_t oldA = regs.a;
                regs.a = (regs.a >> 1) | (oldA & 0x80);
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldA & 0x01);
                return 8;
            }
        case 0x30: // SWAP B
            {
                uint8_t oldB = regs.b;
                regs.b = ((oldB & 0x0F) << 4) | ((oldB & 0xF0) >> 4);
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 8;
            }
        case 0x31: // SWAP C
            {
                uint8_t oldC = regs.c;
                regs.c = ((oldC & 0x0F) << 4) | ((oldC & 0xF0) >> 4);
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 8;
            }
        case 0x32: // SWAP D
            {
                uint8_t oldD = regs.d;
                regs.d = ((oldD & 0x0F) << 4) | ((oldD & 0xF0) >> 4);
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 8;
            }
        case 0x33: // SWAP E
            {
                uint8_t oldE = regs.e;
                regs.e = ((oldE & 0x0F) << 4) | ((oldE & 0xF0) >> 4);
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 8;
            }
        case 0x34: // SWAP H
            {
                uint8_t oldH = regs.h;
                regs.h = ((oldH & 0x0F) << 4) | ((oldH & 0xF0) >> 4);
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 8;
            }
        case 0x35: // SWAP L
            {
                uint8_t oldL = regs.l;
                regs.l = ((oldL & 0x0F) << 4) | ((oldL & 0xF0) >> 4);
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 8;
            }
        case 0x36: // SWAP (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t newValue = ((oldValue & 0x0F) << 4) | ((oldValue & 0xF0) >> 4);
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 16;
            }
        case 0x37: // SWAP A
            {
                uint8_t oldA = regs.a;
                regs.a = ((oldA & 0x0F) << 4) | ((oldA & 0xF0) >> 4);
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, false);
                return 8;
            }
        case 0x38: // SRL B
            {
                uint8_t oldB = regs.b;
                regs.b = (regs.b >> 1);
                setFlag(FLAG_Z, regs.b == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldB & 0x01);
                return 8;
            }
        case 0x39: // SRL C
            {
                uint8_t oldC = regs.c;
                regs.c = (regs.c >> 1);
                setFlag(FLAG_Z, regs.c == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldC & 0x01);
                return 8;
            }
        case 0x3A: // SRL D
            {
                uint8_t oldD = regs.d;
                regs.d = (regs.d >> 1);
                setFlag(FLAG_Z, regs.d == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldD & 0x01);
                return 8;
            }
        case 0x3B: // SRL E
            {
                uint8_t oldE = regs.e;
                regs.e = (regs.e >> 1);
                setFlag(FLAG_Z, regs.e == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldE & 0x01);
                return 8;
            }
        case 0x3C: // SRL H
            {
                uint8_t oldH = regs.h;
                regs.h = (regs.h >> 1);
                setFlag(FLAG_Z, regs.h == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldH & 0x01);
                return 8;
            }
        case 0x3D: // SRL L
            {
                uint8_t oldL = regs.l;
                regs.l = (regs.l >> 1);
                setFlag(FLAG_Z, regs.l == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldL & 0x01);
                return 8;
            }
        case 0x3E: // SRL (HL)
            {
                uint16_t hL = getHL();
                uint8_t oldValue = memory->read(hL);
                uint8_t newValue = (oldValue >> 1);
                memory->write(hL, newValue);
                setFlag(FLAG_Z, newValue == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldValue & 0x01);
                return 16;
            }
        case 0x3F: // SRL A
            {
                uint8_t oldA = regs.a;
                regs.a = (regs.a >> 1);
                setFlag(FLAG_Z, regs.a == 0);
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, false);
                setFlag(FLAG_C, oldA & 0x01);
                return 8;
            }
        case 0x40: // BIT 0,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x41: // BIT 0,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x42: // BIT 0,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x43: // BIT 0,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x44: // BIT 0,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x45: // BIT 0,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x46: // BIT 0,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x47: // BIT 0,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x01));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x48: // BIT 1,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x49: // BIT 1,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x4A: // BIT 1,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x4B: // BIT 1,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x4C: // BIT 1,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x4D: // BIT 1,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x4E: // BIT 1,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x4F: // BIT 1,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x02));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x50: // BIT 2,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x51: // BIT 2,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x52: // BIT 2,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x53: // BIT 2,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x54: // BIT 2,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x55: // BIT 2,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x56: // BIT 2,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x57: // BIT 2,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x04));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x58: // BIT 3,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x59: // BIT 3,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x5A: // BIT 3,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x5B: // BIT 3,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x5C: // BIT 3,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x5D: // BIT 3,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x5E: // BIT 3,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x5F: // BIT 3,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x08));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x60: // BIT 4,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x61: // BIT 4,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x62: // BIT 4,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x63: // BIT 4,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x64: // BIT 4,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x65: // BIT 4,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x66: // BIT 4,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x67: // BIT 4,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x10));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x68: // BIT 5,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x69: // BIT 5,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x6A: // BIT 5,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x6B: // BIT 5,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x6C: // BIT 5,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x6D: // BIT 5,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x6E: // BIT 5,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x6F: // BIT 5,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x20));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x70: // BIT 6,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x71: // BIT 6,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x72: // BIT 6,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x73: // BIT 6,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x74: // BIT 6,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x75: // BIT 6,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x76: // BIT 6,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x77: // BIT 6,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x40));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x78: // BIT 7,B
            {
                setFlag(FLAG_Z, !(regs.b & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x79: // BIT 7,C
            {
                setFlag(FLAG_Z, !(regs.c & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x7A: // BIT 7,D
            {
                setFlag(FLAG_Z, !(regs.d & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x7B: // BIT 7,E
            {
                setFlag(FLAG_Z, !(regs.e & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x7C: // BIT 7,H
            {
                setFlag(FLAG_Z, !(regs.h & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x7D: // BIT 7,L
            {
                setFlag(FLAG_Z, !(regs.l & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x7E: // BIT 7,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                setFlag(FLAG_Z, !(value & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 16;
            }
        case 0x7F: // BIT 7,A
            {
                setFlag(FLAG_Z, !(regs.a & 0x80));
                setFlag(FLAG_N, false);
                setFlag(FLAG_H, true);
                return 8;
            }
        case 0x80: // RES 0,B
            {
                regs.b &= ~0x01;
                return 8;
            }
        case 0x81: // RES 0,C
            {
                regs.c &= ~0x01;
                return 8;
            }
        case 0x82: // RES 0,D
            {
                regs.d &= ~0x01;
                return 8;
            }
        case 0x83: // RES 0,E
            {
                regs.e &= ~0x01;
                return 8;
            }
        case 0x84: // RES 0,H
            {
                regs.h &= ~0x01;
                return 8;
            }
        case 0x85: // RES 0,L
            {
                regs.l &= ~0x01;
                return 8;
            }
        case 0x86: // RES 0,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x01;
                memory->write(hL, value);
                return 16;
            }
        case 0x87: // RES 0,A
            {
                regs.a &= ~0x01;
                return 8;
            }
        case 0x88: // RES 1,B
            {
                regs.b &= ~0x02;
                return 8;
            }
        case 0x89: // RES 1,C
            {
                regs.c &= ~0x02;
                return 8;
            }
        case 0x8A: // RES 1,D
            {
                regs.d &= ~0x02;
                return 8;
            }
        case 0x8B: // RES 1,E
            {
                regs.e &= ~0x02;
                return 8;
            }
        case 0x8C: // RES 1,H
            {
                regs.h &= ~0x02;
                return 8;
            }
        case 0x8D: // RES 1,L
            {
                regs.l &= ~0x02;
                return 8;
            }
        case 0x8E: // RES 1,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x02;
                memory->write(hL, value);
                return 16;
            }
        case 0x8F: // RES 1,A
            {
                regs.a &= ~0x02;
                return 8;
            }
        case 0x90: // RES 2,B
            {
                regs.b &= ~0x04;
                return 8;
            }
        case 0x91: // RES 2,C
            {
                regs.c &= ~0x04;
                return 8;
            }
        case 0x92: // RES 2,D
            {
                regs.d &= ~0x04;
                return 8;
            }
        case 0x93: // RES 2,E
            {
                regs.e &= ~0x04;
                return 8;
            }
        case 0x94: // RES 2,H
            {
                regs.h &= ~0x04;
                return 8;
            }
        case 0x95: // RES 2,L
            {
                regs.l &= ~0x04;
                return 8;
            }
        case 0x96: // RES 2,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x04;
                memory->write(hL, value);
                return 16;
            }
        case 0x97: // RES 2,A
            {
                regs.a &= ~0x04;
                return 8;
            }
        case 0x98: // RES 3,B
            {
                regs.b &= ~0x08;
                return 8;
            }
        case 0x99: // RES 3,C
            {
                regs.c &= ~0x08;
                return 8;
            }
        case 0x9A: // RES 3,D
            {
                regs.d &= ~0x08;
                return 8;
            }
        case 0x9B: // RES 3,E
            {
                regs.e &= ~0x08;
                return 8;
            }
        case 0x9C: // RES 3,H
            {
                regs.h &= ~0x08;
                return 8;
            }
        case 0x9D: // RES 3,L
            {
                regs.l &= ~0x08;
                return 8;
            }
        case 0x9E: // RES 3,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x08;
                memory->write(hL, value);
                return 16;
            }
        case 0x9F: // RES 3,A
            {
                regs.a &= ~0x08;
                return 8;
            }
        case 0xA0: // RES 4,B
            {
                regs.b &= ~0x10;
                return 8;
            }
        case 0xA1: // RES 4,C
            {
                regs.c &= ~0x10;
                return 8;
            }
        case 0xA2: // RES 4,D
            {
                regs.d &= ~0x10;
                return 8;
            }
        case 0xA3: // RES 4,E
            {
                regs.e &= ~0x10;
                return 8;
            }
        case 0xA4: // RES 4,H
            {
                regs.h &= ~0x10;
                return 8;
            }
        case 0xA5: // RES 4,L
            {
                regs.l &= ~0x10;
                return 8;
            }
        case 0xA6: // RES 4,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x10;
                memory->write(hL, value);
                return 16;
            }
        case 0xA7: // RES 4,A
            {
                regs.a &= ~0x10;
                return 8;
            }
        case 0xA8: // RES 5,B
            {
                regs.b &= ~0x20;
                return 8;
            }
        case 0xA9: // RES 5,C
            {
                regs.c &= ~0x20;
                return 8;
            }
        case 0xAA: // RES 5,D
            {
                regs.d &= ~0x20;
                return 8;
            }
        case 0xAB: // RES 5,E
            {
                regs.e &= ~0x20;
                return 8;
            }
        case 0xAC: // RES 5,H
            {
                regs.h &= ~0x20;
                return 8;
            }
        case 0xAD: // RES 5,L
            {
                regs.l &= ~0x20;
                return 8;
            }
        case 0xAE: // RES 5,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x20;
                memory->write(hL, value);
                return 16;
            }
        case 0xAF: // RES 5,A
            {
                regs.a &= ~0x20;
                return 8;
            }
        case 0xB0: // RES 6,B
            {
                regs.b &= ~0x40;
                return 8;
            }
        case 0xB1: // RES 6,C
            {
                regs.c &= ~0x40;
                return 8;
            }
        case 0xB2: // RES 6,D
            {
                regs.d &= ~0x40;
                return 8;
            }
        case 0xB3: // RES 6,E
            {
                regs.e &= ~0x40;
                return 8;
            }
        case 0xB4: // RES 6,H
            {
                regs.h &= ~0x40;
                return 8;
            }
        case 0xB5: // RES 6,L
            {
                regs.l &= ~0x40;
                return 8;
            }
        case 0xB6: // RES 6,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x40;
                memory->write(hL, value);
                return 16;
            }
        case 0xB7: // RES 6,A
            {
                regs.a &= ~0x40;
                return 8;
            }
        case 0xB8: // RES 7,B
            {
                regs.b &= ~0x80;
                return 8;
            }
        case 0xB9: // RES 7,C
            {
                regs.c &= ~0x80;
                return 8;
            }
        case 0xBA: // RES 7,D
            {
                regs.d &= ~0x80;
                return 8;
            }
        case 0xBB: // RES 7,E
            {
                regs.e &= ~0x80;
                return 8;
            }
        case 0xBC: // RES 7,H
            {
                regs.h &= ~0x80;
                return 8;
            }
        case 0xBD: // RES 7,L
            {
                regs.l &= ~0x80;
                return 8;
            }
        case 0xBE: // RES 7,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value &= ~0x80;
                memory->write(hL, value);
                return 16;
            }
        case 0xBF: // RES 7,A
            {
                regs.a &= ~0x80;
                return 8;
            }
        case 0xC0: // SET 0,B
            {
                regs.b |= 0x01;
                return 8;
            }
        case 0xC1: // SET 0,C
            {
                regs.c |= 0x01;
                return 8;
            }
        case 0xC2: // SET 0,D
            {
                regs.d |= 0x01;
                return 8;
            }
        case 0xC3: // SET 0,E
            {
                regs.e |= 0x01;
                return 8;
            }
        case 0xC4: // SET 0,H
            {
                regs.h |= 0x01;
                return 8;
            }
        case 0xC5: // SET 0,L
            {
                regs.l |= 0x01;
                return 8;
            }
        case 0xC6: // SET 0,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x01;
                memory->write(hL, value);
                return 16;
            }
        case 0xC7: // SET 0,A
            {
                regs.a |= 0x01;
                return 8;
            }
        case 0xC8: // SET 1,B
            {
                regs.b |= 0x02;
                return 8;
            }
        case 0xC9: // SET 1,C
            {
                regs.c |= 0x02;
                return 8;
            }
        case 0xCA: // SET 1,D
            {
                regs.d |= 0x02;
                return 8;
            }
        case 0xCB: // SET 1,E
            {
                regs.e |= 0x02;
                return 8;
            }
        case 0xCC: // SET 1,H
            {
                regs.h |= 0x02;
                return 8;
            }
        case 0xCD: // SET 1,L
            {
                regs.l |= 0x02;
                return 8;
            }
        case 0xCE: // SET 1,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x02;
                memory->write(hL, value);
                return 16;
            }
        case 0xCF: // SET 1,A
            {
                regs.a |= 0x02;
                return 8;
            }
        case 0xD0: // SET 2,B
            {
                regs.b |= 0x04;
                return 8;
            }
        case 0xD1: // SET 2,C
            {
                regs.c |= 0x04;
                return 8;
            }
        case 0xD2: // SET 2,D
            {
                regs.d |= 0x04;
                return 8;
            }
        case 0xD3: // SET 2,E
            {
                regs.e |= 0x04;
                return 8;
            }
        case 0xD4: // SET 2,H
            {
                regs.h |= 0x04;
                return 8;
            }
        case 0xD5: // SET 2,L
            {
                regs.l |= 0x04;
                return 8;
            }
        case 0xD6: // SET 2,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x04;
                memory->write(hL, value);
                return 16;
            }
        case 0xD7: // SET 2,A
            {
                regs.a |= 0x04;
                return 8;
            }
        case 0xD8: // SET 3,B
            {
                regs.b |= 0x08;
                return 8;
            }
        case 0xD9: // SET 3,C
            {
                regs.c |= 0x08;
                return 8;
            }
        case 0xDA: // SET 3,D
            {
                regs.d |= 0x08;
                return 8;
            }
        case 0xDB: // SET 3,E
            {
                regs.e |= 0x08;
                return 8;
            }
        case 0xDC: // SET 3,H
            {
                regs.h |= 0x08;
                return 8;
            }
        case 0xDD: // SET 3,L
            {
                regs.l |= 0x08;
                return 8;
            }
        case 0xDE: // SET 3,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x08;
                memory->write(hL, value);
                return 16;
            }
        case 0xDF: // SET 3,A
            {
                regs.a |= 0x08;
                return 8;
            }
        case 0xE0: // SET 4,B
            {
                regs.b |= 0x10;
                return 8;
            }
        case 0xE1: // SET 4,C
            {
                regs.c |= 0x10;
                return 8;
            }
        case 0xE2: // SET 4,D
            {
                regs.d |= 0x10;
                return 8;
            }
        case 0xE3: // SET 4,E
            {
                regs.e |= 0x10;
                return 8;
            }
        case 0xE4: // SET 4,H
            {
                regs.h |= 0x10;
                return 8;
            }
        case 0xE5: // SET 4,L
            {
                regs.l |= 0x10;
                return 8;
            }
        case 0xE6: // SET 4,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x10;
                memory->write(hL, value);
                return 16;
            }
        case 0xE7: // SET 4,A
            {
                regs.a |= 0x10;
                return 8;
            }
        case 0xE8: // SET 5,B
            {
                regs.b |= 0x20;
                return 8;
            }
        case 0xE9: // SET 5,C
            {
                regs.c |= 0x20;
                return 8;
            }
        case 0xEA: // SET 5,D
            {
                regs.d |= 0x20;
                return 8;
            }
        case 0xEB: // SET 5,E
            {
                regs.e |= 0x20;
                return 8;
            }
        case 0xEC: // SET 5,H
            {
                regs.h |= 0x20;
                return 8;
            }
        case 0xED: // SET 5,L
            {
                regs.l |= 0x20;
                return 8;
            }
        case 0xEE: // SET 5,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x20;
                memory->write(hL, value);
                return 16;
            }
        case 0xEF: // SET 5,A
            {
                regs.a |= 0x20;
                return 8;
            }
        case 0xF0: // SET 6,B
            {
                regs.b |= 0x40;
                return 8;
            }
        case 0xF1: // SET 6,C
            {
                regs.c |= 0x40;
                return 8;
            }
        case 0xF2: // SET 6,D
            {
                regs.d |= 0x40;
                return 8;
            }
        case 0xF3: // SET 6,E
            {
                regs.e |= 0x40;
                return 8;
            }
        case 0xF4: // SET 6,H
            {
                regs.h |= 0x40;
                return 8;
            }
        case 0xF5: // SET 6,L
            {
                regs.l |= 0x40;
                return 8;
            }
        case 0xF6: // SET 6,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x40;
                memory->write(hL, value);
                return 16;
            }
        case 0xF7: // SET 6,A
            {
                regs.a |= 0x40;
                return 8;
            }
        case 0xF8: // SET 7,B
            {
                regs.b |= 0x80;
                return 8;
            }
        case 0xF9: // SET 7,C
            {
                regs.c |= 0x80;
                return 8;
            }
        case 0xFA: // SET 7,D
            {
                regs.d |= 0x80;
                return 8;
            }
        case 0xFB: // SET 7,E
            {
                regs.e |= 0x80;
                return 8;
            }
        case 0xFC: // SET 7,H
            {
                regs.h |= 0x80;
                return 8;
            }
        case 0xFD: // SET 7,L
            {
                regs.l |= 0x80;
                return 8;
            }
        case 0xFE: // SET 7,(HL)
            {
                uint16_t hL = getHL();
                uint8_t value = memory->read(hL);
                value |= 0x80;
                memory->write(hL, value);
                return 16;
            }
        case 0xFF: // SET 7,A
            {
                regs.a |= 0x80;
                return 8;
            }
        // ... 256 cases total
        default:
            std::cout << "Unknown CB opcode: 0x" << std::hex << (int)opcode 
                     << " at PC: 0x" << (regs.pc - 1) << std::endl;
            return 4;}
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
    int mode; // PPU mode
    int mode_cycles; // Cycles spent in current mode
    int scanline; // Current scanline (0-153)
    struct Sprite {
        uint8_t y;
        uint8_t x;
        uint8_t tile;
        uint8_t flags;
    };
    
public:
    PPU(Memory* mem) : memory(mem) {
        framebuffer.fill(0xFFFFFFFF);  // White
        mode = 2;
        mode_cycles = 0;
        scanline = 0;
    }
    
    void step(int cycles) {
        
    mode_cycles += cycles;
    
    // Mode 2: OAM Scan (80 cycles)
    if (mode == 2) {
        if (mode_cycles >= 80) {
            mode_cycles -= 80;
            mode = 3;  // Move to drawing mode
            
            uint8_t stat = memory->read(0xFF41);
            stat = (stat & 0xFC) | mode;
            memory->write(0xFF41, stat);
        }
    }
    // Mode 3: Drawing (172 cycles)
    else if (mode == 3) {
        if (mode_cycles >= 172) {  
            mode_cycles -= 172;
            mode = 0; 

            renderScanline();
            
            uint8_t stat = memory->read(0xFF41);
            stat = (stat & 0xFC) | mode;
            memory->write(0xFF41, stat);
        }
    }
    // Mode 0: H-Blank (204 cycles)
    else if (mode == 0) {
        if (mode_cycles >= 204) {  // What goes here?
            mode_cycles -= 204;
            
            // End of scanline - what happens next?
            scanline++;  
            
            if (scanline < 144) {
                // Still in visible area
                mode = 2;  // Back to OAM scan
            } else {
                // Entering V-Blank
                mode = 1; // Switch to V-Blank mode
                  uint8_t if_flag = memory->read(0xFF0F);
            memory->write(0xFF0F, if_flag | 0x01);
            }
            // Update LY register
            memory->write(0xFF44, scanline);
            // Update STAT register
            uint8_t stat = memory->read(0xFF41);
            stat = (stat & 0xFC) | mode;
            memory->write(0xFF41, stat);
            
        }
    }
    // Mode 1: V-Blank (456 cycles per line, 10 lines)
    else if (mode == 1) {
        if (mode_cycles >= 456) {  // How many cycles per V-Blank line?
            mode_cycles -= 456;
            scanline++;
            
            if (scanline > 153) {  // When does V-Blank end? (hint: line 153)
                // Start new frame
                scanline = 0;
                mode = 2;  // Back to OAM scan
            }

            static int frame_count = 0;
            if (++frame_count % 60 == 0) {  // Every 60 frames (1 second)
            }
        }
        
        
            
            // Update LY register
            memory->write(0xFF44, scanline);
            
            // Update STAT register
            uint8_t stat = memory->read(0xFF41);
            stat = (stat & 0xFC) | mode;
            memory->write(0xFF41, stat);
        }
    }

void renderScanline() {
    // Read LCD control register
    uint8_t lcdc = memory->read(0xFF40);
    
    // Check if LCD is completely off (bit 7)
    if (!(lcdc & 0x80)) {
        // LCD is OFF - don't render anything, keep showing last frame
        return;
    }
    
    // Check if background is enabled (bit 0)
    if (!(lcdc & 0x01)) {
        // Background disabled - fill scanline with white
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int fb_index = scanline * SCREEN_WIDTH + x;
            framebuffer[fb_index] = 0xFFFFFFFF;
        }
        return;
    }
    
    // Read scroll registers
    uint8_t scy = memory->read(0xFF42);  // Scroll Y
    uint8_t scx = memory->read(0xFF43);  // Scroll X
    
    // Which tile map to use? (bit 3 of LCDC)
    uint16_t tile_map_base = (lcdc & 0x08) ? 0x9C00 : 0x9800;
    
    // Which tile data to use? (bit 4 of LCDC)
    bool use_signed = !(lcdc & 0x10);
    uint16_t tile_data_base = use_signed ? 0x9000 : 0x8000;
    
    // Calculate which row of the background we're drawing
    uint8_t bg_y = (scanline + scy) & 0xFF;  // Wrap around
    uint8_t tile_row = bg_y / 8;   // Which row of tiles
    uint8_t pixel_row = bg_y % 8;  // Which row within the tile
    
    // Draw each pixel in this scanline
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // Calculate background position
        uint8_t bg_x = (x + scx) & 0xFF;  // Wrap around
        uint8_t tile_col = bg_x / 8;
        uint8_t pixel_col = bg_x % 8;
        
        // Get tile number from tile map
        uint16_t tile_map_addr = tile_map_base + (tile_row * 32) + tile_col;
        uint8_t tile_num = memory->read(tile_map_addr);
        
        // Calculate tile data address
        uint16_t tile_addr;
        if (use_signed) {
            int8_t signed_tile = (int8_t)tile_num;
            tile_addr = tile_data_base + ((signed_tile + 128) * 16);
        } else {
            tile_addr = tile_data_base + (tile_num * 16);
        }
        
        // Read the two bytes for this pixel row
        uint8_t byte1 = memory->read(tile_addr + (pixel_row * 2));
        uint8_t byte2 = memory->read(tile_addr + (pixel_row * 2) + 1);
        
        // Extract the pixel color
        int bit = 7 - pixel_col;
        uint8_t color_num = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
        
        // Convert to RGB
        uint32_t color;
        switch (color_num) {
            case 0: color = 0xFFFFFFFF; break;
            case 1: color = 0xFFAAAAAA; break;
            case 2: color = 0xFF555555; break;
            case 3: color = 0xFF000000; break;
        }
        
        // Draw to framebuffer
        int fb_index = scanline * SCREEN_WIDTH + x;
        framebuffer[fb_index] = color;
    }
    renderSprites();
}

void renderSprites() {
    uint8_t lcdc = memory->read(0xFF40);
    
    // Check if sprites are enabled (bit 1)
    if (!(lcdc & 0x02)) {
        return;
    }
    
    // Sprite height: 8x8 or 8x16 (bit 2)
    int sprite_height = (lcdc & 0x04) ? 16 : 8;
    
    // ✅ NEW DEBUG: Print first few sprites in OAM once
    static bool printed_oam = false;
    if (!printed_oam) {
        std::cout << "\n=== OAM CONTENTS ===" << std::endl;
        for (int i = 0; i < 5; i++) {  // Check first 5 sprites
            uint16_t oam_addr = 0xFE00 + (i * 4);
            uint8_t y = memory->read(oam_addr);
            uint8_t x = memory->read(oam_addr + 1);
            uint8_t tile = memory->read(oam_addr + 2);
            uint8_t flags = memory->read(oam_addr + 3);
            
            std::cout << "Sprite " << i << ": "
                     << "Y=" << std::dec << (int)y << " (screen: " << ((int)y - 16) << ") "
                     << "X=" << (int)x << " (screen: " << ((int)x - 8) << ") "
                     << "Tile=0x" << std::hex << (int)tile << std::dec
                     << " Flags=0x" << std::hex << (int)flags << std::dec << std::endl;
        }
        std::cout << "Sprite height: " << sprite_height << std::endl;
        printed_oam = true;
    }
    
    // Read all sprites from OAM
    std::array<Sprite, 40> sprites;
    for (int i = 0; i < 40; i++) {
        uint16_t oam_addr = 0xFE00 + (i * 4);
        sprites[i].y = memory->read(oam_addr);
        sprites[i].x = memory->read(oam_addr + 1);
        sprites[i].tile = memory->read(oam_addr + 2);
        sprites[i].flags = memory->read(oam_addr + 3);
    }
    
    // Find sprites on current scanline (max 10 per line)
    std::array<int, 10> visible_sprites;
    int sprite_count = 0;
    
    for (int i = 0; i < 40 && sprite_count < 10; i++) {
        int sprite_y = sprites[i].y - 16;
        
        // Check if sprite is on this scanline
        if (scanline >= sprite_y && scanline < sprite_y + sprite_height) {
            visible_sprites[sprite_count++] = i;
        }
    }
    
    // ✅ DEBUG: Count total sprites found per frame
    static int total_sprites_found = 0;
    static int frame_counter = 0;
    total_sprites_found += sprite_count;
    
    if (scanline == 143) {  // Last visible scanline
        frame_counter++;
        if (frame_counter % 60 == 0) {  // Once per second
            std::cout << "Sprites found in last frame: " << total_sprites_found << std::endl;
        }
        total_sprites_found = 0;
    }
    
    // Draw sprites (in reverse order for priority)
    for (int i = sprite_count - 1; i >= 0; i--) {
        Sprite& sprite = sprites[visible_sprites[i]];
        
        int sprite_y = sprite.y - 16;
        int sprite_x = sprite.x - 8;
        
        // Get sprite attributes
        bool flip_y = sprite.flags & 0x40;
        bool flip_x = sprite.flags & 0x20;
        bool behind_bg = sprite.flags & 0x80;
        uint8_t palette_num = (sprite.flags & 0x10) ? 1 : 0;
        
        // Calculate which row of the sprite we're drawing
        int sprite_row = scanline - sprite_y;
        if (flip_y) {
            sprite_row = sprite_height - 1 - sprite_row;
        }
        
        // Get tile data
        uint16_t tile_addr = 0x8000 + (sprite.tile * 16) + (sprite_row * 2);
        
        if (sprite_height == 16) {
            tile_addr = 0x8000 + ((sprite.tile & 0xFE) * 16) + (sprite_row * 2);
        }
        
        uint8_t byte1 = memory->read(tile_addr);
        uint8_t byte2 = memory->read(tile_addr + 1);
        
        // Draw 8 pixels
        for (int x = 0; x < 8; x++) {
            int screen_x = sprite_x + x;
            
            if (screen_x < 0 || screen_x >= SCREEN_WIDTH) continue;
            
            int bit = flip_x ? x : (7 - x);
            uint8_t color_num = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
            
            if (color_num == 0) continue;
            
            uint8_t palette = memory->read(palette_num ? 0xFF49 : 0xFF48);
            uint8_t palette_color = (palette >> (color_num * 2)) & 0x03;
            
            uint32_t color;
            switch (palette_color) {
                case 0: color = 0xFFFFFFFF; break;
                case 1: color = 0xFFAAAAAA; break;
                case 2: color = 0xFF555555; break;
                case 3: color = 0xFF000000; break;
            }
            
            int fb_index = scanline * SCREEN_WIDTH + screen_x;
            if (behind_bg) {
                if (framebuffer[fb_index] == 0xFFFFFFFF) {
                    framebuffer[fb_index] = color;
                }
            } else {
                framebuffer[fb_index] = color;
            }
        }
    }
}

void drawTile(int tile_num, int x, int y) {
   uint16_t tile_addr = 0x8000 + (tile_num * 16);
   for (int row = 0; row < 8; row++) {
        uint8_t byte1 = memory->read(tile_addr + row * 2);
        uint8_t byte2 = memory->read(tile_addr + row * 2 + 1);
        for (int col = 0; col < 8; col++) {
            int bit = 7 - col;
            uint8_t color_num = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
            uint32_t color;
            switch (color_num) {
                case 0: color = 0xFFFFFFFF; break; // White
                case 1: color = 0xFFAAAAAA; break; // Light gray
                case 2: color = 0xFF555555; break; // Dark gray
                case 3: color = 0xFF000000; break; // Black
            }
            int pixel_x = x + col;
            int pixel_y = y + row;
            int fb_index = pixel_y * SCREEN_WIDTH + pixel_x;
            framebuffer[fb_index] = color;
            }
        }
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
    Timer timer;
    APU apu;
    std::array<bool, 8> button_states;
    
public:
    GameBoy() : cpu(&memory), ppu(&memory), timer(&memory), apu(&memory) {
        button_states.fill(false);
        memory.setAPU(&apu);
    }
    
    bool loadROM(const std::string& filename) {
        return memory.loadROM(filename);
    }
    
    int step() {
        int cycles = cpu.step();
        ppu.step(cycles);
        timer.step(cycles);
        apu.step(cycles);
        return cycles;
    }
    
    const std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT>& getScreen() {
        return ppu.getFramebuffer();
    }

    float getAudioSample() {
        return apu.generateSample();
    }

    void setButtonState(int button, bool pressed) {
        if (pressed && !button_states[button]) {
            // Button just pressed
            if (button < 4) {
                memory.pressButton(button);
            } else {
                memory.pressDirection(button - 4);
            }
            button_states[button] = true;
        } else if (!pressed && button_states[button]) {
            // Button just released
            if (button < 4) {
                memory.releaseButton(button);
            } else {
                memory.releaseDirection(button - 4);
            }
            button_states[button] = false;
        }
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    GameBoy gameboy;
    Display display;
    
    if (!gameboy.loadROM(argv[1])) {
        return 1;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 512;
    want.callback = nullptr;  // We'll use SDL_QueueAudio instead
    
    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audio_device == 0) {
        std::cout << "Failed to open audio: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_PauseAudioDevice(audio_device, 0);  // Start playing
    
    // Audio buffer
    std::vector<float> audio_buffer;
    audio_buffer.reserve(1024);
    
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            // Key pressed
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN: gameboy.setButtonState(Memory::BTN_START, true); break;
                    case SDLK_RSHIFT: gameboy.setButtonState(Memory::BTN_SELECT, true); break;
                    case SDLK_z: gameboy.setButtonState(Memory::BTN_A, true); break;
                    case SDLK_x: gameboy.setButtonState(Memory::BTN_B, true); break;
                    case SDLK_UP: gameboy.setButtonState(Memory::DIR_UP + 4, true); break;
                    case SDLK_DOWN: gameboy.setButtonState(Memory::DIR_DOWN + 4, true); break;
                    case SDLK_LEFT: gameboy.setButtonState(Memory::DIR_LEFT + 4, true); break;
                    case SDLK_RIGHT: gameboy.setButtonState(Memory::DIR_RIGHT + 4, true); break;
                }
            }
            
            // ✅ Handle key release
            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN: gameboy.setButtonState(Memory::BTN_START, false); break;
                    case SDLK_RSHIFT: gameboy.setButtonState(Memory::BTN_SELECT, false); break;
                    case SDLK_z: gameboy.setButtonState(Memory::BTN_A, false); break;
                    case SDLK_x: gameboy.setButtonState(Memory::BTN_B, false); break;
                    case SDLK_UP: gameboy.setButtonState(Memory::DIR_UP + 4, false); break;
                    case SDLK_DOWN: gameboy.setButtonState(Memory::DIR_DOWN + 4, false); break;
                    case SDLK_LEFT: gameboy.setButtonState(Memory::DIR_LEFT + 4, false); break;
                    case SDLK_RIGHT: gameboy.setButtonState(Memory::DIR_RIGHT + 4, false); break;
                }
            }
        }
        
        // ✅ Remove the updateButtonStates() call - we handle it manually now
        
        int cycles_this_frame = 0;
        while (cycles_this_frame < 70224) {
            cycles_this_frame += gameboy.step();
            static int audio_cycles = 0;
            audio_cycles += gameboy.step();
            if (audio_cycles >= 95) {
                audio_cycles -= 95;
                audio_buffer.push_back(gameboy.getAudioSample());
            }
        }

        if (!audio_buffer.empty()) {
            SDL_QueueAudio(audio_device, audio_buffer.data(), audio_buffer.size() * sizeof(float));
            audio_buffer.clear();
        }

        display.render(gameboy.getScreen());
        SDL_Delay(16); // Roughly 60 FPS
    }
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
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