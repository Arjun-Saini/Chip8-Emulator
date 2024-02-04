#include <iostream>

class Chip8{
public:
    uint16_t programCounter;
    uint8_t memory[4096];
    uint8_t registers[16];
    uint16_t indexRegister;
    uint16_t stack[16];
    int delayTimer;
    int soundTimer;
    uint8_t stackPointer;
    uint8_t keys[16];
    uint32_t graphics[64 * 32];
    uint16_t opcode;

    uint8_t font[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    Chip8(){
        programCounter = 0x200;

        // TODO Load font into memory starting at address 0x50
    }

    // TODO Loads ROM into memory, starting at address 0x200
    void loadROM(char const* file){

    }

    // TODO Reads the next opcode, each opcode takes 2 bytes of memory
    void nextOpcode(){

    }

    // TODO Decodes opcode by using a function table, and processes the instruction accordingly
    void decodeOpcode(){

    }

    // TODO Write opcode instructions


};

// TODO Video and Audio processing, use SDL?

int main() {
    // TODO Initialize visual interface and chip, run cycles based on a delay (default at 60Hz), decrement counters
    return 0;
}