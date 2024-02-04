#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

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
        // Clear memory
        memset(&memory, 0, 4096);
        programCounter = 0x200;

        // Load font into memory starting at address 0x50
        for(int i = 0; i < 80; i++){
            memory[0x50 + i] = font[i];
        }

        indexRegister = 0;
        delayTimer = 0;
        soundTimer = 0;
        stackPointer = 0;
    }

    // Loads ROM into memory, starting at address 0x200
    void loadROM(std::string fileName){
        // Open ROM
        std::ifstream ROM("../ROMs/" + fileName + ".ch8", std::ios::binary);
        ROM.seekg(0, std::ios::end);
        std::streamoff fileSize = ROM.tellg();
        ROM.seekg(0, std::ios::beg);

        // Load ROM into buffer
        char buffer[fileSize];
        memset(buffer, 0, fileSize);
        ROM.read(buffer, fileSize);

        // Copy buffer into memory
        for(int i = 0; i < fileSize; i++){
            memory[0x200 + i] = buffer[i];
        }
    }

    // Reads the next opcode, each opcode takes 2 bytes of memory
    void getOpcode(){
        opcode = memory[programCounter] << 8 | memory[programCounter + 1];
        programCounter += 2;
    }

    // TODO Decodes opcode by using a function table, and processes the instruction accordingly
    void decodeOpcode(){

    }

    // TODO Clears Screen
    void OP_00E0(uint16_t op){

    }

    // TODO Returns from subroutine
    void OP_00EE(uint16_t op){

    }

    // TODO Jumps to address NNN
    void OP_1NNN(uint16_t op){

    }

    // TODO Calls subroutine at NNN
    void OP_2NNN(uint16_t op){

    }

    // TODO Skips next instruction if Vx == NN
    void OP_3XNN(uint16_t op){

    }

    // TODO Skips next instruction if Vx != NN
    void OP_4XNN(uint16_t op){

    }

    // TODO Skips next instruction if Vx == Vy
    void OP_5XY0(uint16_t op){

    }

    // TODO Sets Vx to NN
    void OP_6XNN(uint16_t op){

    }

    // TODO Adds NN to Vx (does not update carry flag)
    void OP_7XNN(uint16_t op){

    }

    // TODO Sets Vx to value of Vy
    void OP_8XY0(uint16_t op){

    }

    // TODO Sets Vx to Vx OR Vy
    void OP_8XY1(uint16_t op){

    }

    // TODO Sets Vx to Vx AND Vy
    void OP_8XY2(uint16_t op){

    }

    // TODO Sets Vx to Vx XOR Vy
    void OP_8XY3(uint16_t op){

    }

    // TODO Adds Vx and Vy, sets VF to 1 if there is an overflow
    void OP_8XY4(uint16_t op){

    }

    // TODO Vy is subtracted from Vx, sets VF to 1 if there is an underflow, 0 if not
    void OP_8XY5(uint16_t op){

    }

    // TODO Stores least significant bit of Vx in VF, then then shifts Vx to the right by 1
    void OP_8XY6(uint16_t op){

    }

    // TODO Vx is subtracted from Vy and result stored in Vx, sets VF to 1 if there is an underflow, 0 if not
    void OP_8XY7(uint16_t op){

    }

    // TODO Stores most significant bit of Vx in VF, then then shifts Vx to the left by 1
    void OP_8XYE(uint16_t op){

    }

    // TODO Skips next instruction if Vx != Vy
    void OP_9XY0(uint16_t op){

    }

    // TODO Sets indexRegister to NNN
    void OP_ANNN(uint16_t op){

    }

    // TODO Jumps to address NNN + V0
    void OP_BNNN(uint16_t op){

    }

    // TODO Sets Vx to to bitwise AND of NN and a random number from 0 to 255
    void OP_CXNN(uint16_t op){

    }

    // TODO Draws sprite at coordinate (Vx, Vy) that has a width of 8 pixels and height of N pixels, VF set to 1 if
    //  any pixels are flipped from set to unset, 0 if not
    void OP_DXYN(uint16_t op){

    }

    // TODO Skips next instruction if key stored in Vx is pressed
    void OP_EX9E(uint16_t op){

    }

    // TODO Skips next instruction if key stored in Vx is not pressed
    void OP_EXA1(uint16_t op){

    }

    // TODO Sets Vx to value of delayTimer
    void OP_FX07(uint16_t op){

    }

    // TODO Waits for key press, and then stores it in Vx, blocks all further instructions until key press
    void OP_FX0A(uint16_t op){

    }

    // TODO Sets delayTimer to Vx
    void OP_FX15(uint16_t op){

    }

    // TODO Sets soundTimer to Vx
    void OP_FX18(uint16_t op){

    }

    // TODO Adds Vx to indexRegister, VF does not change
    void OP_FX1E(uint16_t op){

    }

    // TODO Sets indexRegister to location of the sprite in character Vx
    void OP_FX29(uint16_t op){

    }

    // TODO Stores binary-coded decimal representation of Vx, hundreds digit at indexRegister, tens digit at
    //  indexRegister+1, ones at indexRegister+2
    void OP_FX33(uint16_t op){

    }

    // TODO Stores values from V0 to Vx in memory, inclusive, starting at indexRegister (indexRegister is unmodified)
    void OP_FX55(uint16_t op){

    }

    // TODO Fills values from V0 to Vx from memory, inclusive, starting at indexRegister (indexRegister is unmodified)
    void OP_FX65(uint16_t op){

    }

    void printMemory(){
        for(int i = 0; i < 128; i++){
            for(int j = 0; j < 32; j++){
                std::cout << std::hex << int(memory[i * 32 + j]) << " ";
            }
            std::cout << std::endl;
        }
    }
};

// TODO Video and Audio processing, use SDL?

int main() {
    Chip8 chip = Chip8();
    chip.loadROM("test_opcode");

    // TODO Initialize visual interface and chip, run cycles based on a delay (default at 60Hz), decrement counters
    return 0;
}