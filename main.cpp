#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <chrono>

class Chip8{
public:
    uint16_t programCounter;
    uint8_t memory[4096];
    uint8_t registers[16];
    uint16_t registerI;
    uint16_t stack[16];
    int delayTimer;
    int soundTimer;
    uint8_t stackPointer;
    uint8_t keys[16];
    uint32_t graphics[64][32];
    uint16_t opcode;
    std::streamoff fileSize;
    std::mt19937 rng;
    std::uniform_int_distribution<uint8_t> distribution;


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
        memset(&registers, 0, 16);
        memset(&stack, 0, 32);
        programCounter = 0x200;

        // Load font into memory starting at address 0x50
        for(int i = 0; i < 80; i++){
            memory[0x50 + i] = font[i];
        }

        registerI = 0;
        delayTimer = 0;
        soundTimer = 0;
        stackPointer = 0;
        rng.seed(time(NULL));
    }

    // Loads ROM into memory, starting at address 0x200
    void loadROM(std::string fileName){
        // Open ROM
        std::ifstream ROM("../ROMs/" + fileName + ".ch8", std::ios::binary);
        ROM.seekg(0, std::ios::end);
        fileSize = ROM.tellg();
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
        opcode = memory[programCounter] << 8u | memory[programCounter + 1];
        programCounter += 2;
    }

    // TODO Decodes opcode by using a function table, and processes the instruction accordingly
    void decodeOpcode(){

    }

    // Clears Screen
    void OP_00E0(){
        for(int i = 0; i < 64; i++){
            for(int j = 0; j < 32; j++){
                graphics[i][j] = 0;
            }
        }
    }

    // Returns from subroutine
    void OP_00EE(){
        programCounter = stack[--stackPointer];
    }

    // Jumps to address NNN
    void OP_1NNN(){
        uint8_t NNN = opcode & 0x0FFFu;
        programCounter = NNN;
    }

    // Calls subroutine at NNN
    void OP_2NNN(){
        uint16_t NNN = opcode & 0x0FFFu;
        stack[stackPointer++] = programCounter;
        programCounter = NNN;
    }

    // Skips next instruction if Vx == NN
    void OP_3XNN(){
        uint8_t Vx = opcode & 0x0F00u;
        uint8_t NN = opcode & 0x00FFu;
        if(registers[Vx] == NN){
            programCounter += 2;
        }
    }

    // Skips next instruction if Vx != NN
    void OP_4XNN(){
        uint8_t Vx = opcode & 0x0F00u;
        uint8_t NN = opcode & 0x00FFu;
        if(registers[Vx] != NN){
            programCounter += 2;
        }
    }

    // Skips next instruction if Vx == Vy
    void OP_5XY0(){
        uint8_t Vx = opcode & 0x0F00u;
        uint8_t Vy = opcode & 0x00F0u;
        if(registers[Vx] == registers[Vy]){
            programCounter += 2;
        }
    }

    // Sets Vx to NN
    void OP_6XNN(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t NN = opcode & 0x00FFu;
        registers[Vx] = NN;
    }

    // Adds NN to Vx (does not update carry flag)
    void OP_7XNN(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t NN = opcode & 0x00FFu;
        registers[Vx] += NN;
    }

    // Sets Vx to value of Vy
    void OP_8XY0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        registers[Vx] = registers[Vy];
    }

    // Sets Vx to Vx OR Vy
    void OP_8XY1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        registers[Vx] |= registers[Vy];
    }

    // Sets Vx to Vx AND Vy
    void OP_8XY2(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        registers[Vx] &= registers[Vy];
    }

    // Sets Vx to Vx XOR Vy
    void OP_8XY3(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        registers[Vx] ^= registers[Vy];
    }

    // Vx += Vy, sets VF to 1 if there is an overflow
    void OP_8XY4(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        uint16_t sum = registers[Vx] + registers[Vy];

        if(sum > 0xFFu){
            registers[0xFu] = 1;
        }else{
            registers[0xFu] = 0;
        }
        registers[Vx] = sum;
    }

    // Vy is subtracted from Vx, sets VF to 0 if there is an underflow, 1 if not
    void OP_8XY5(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        uint8_t difference = registers[Vx] - registers[Vy];

        if(registers[Vx] >= registers[Vy]){
            registers[0xFu] = 1;
        }else{
            registers[0xFu] = 0;
        }
        registers[Vx] = difference;
    }

    // Stores least significant bit of Vx in VF, then shifts Vx to the right by 1
    void OP_8XY6(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        registers[0xFu] = registers[Vx] & 0b1u;
        registers[Vx] >>= 1u;
    }

    // Vx is subtracted from Vy and result stored in Vx, sets VF to 0 if there is an underflow, 1 if not
    void OP_8XY7(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        uint8_t difference = registers[Vy] - registers[Vx];

        if(registers[Vx] <= registers[Vy]){
            registers[0xFu] = 1;
        }else{
            registers[0xFu] = 0;
        }
        registers[Vx] = difference;
    }

    // Stores most significant bit of Vx in VF, then shifts Vx to the left by 1
    void OP_8XYE(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        registers[0xFu] = (registers[Vx] & 0b10000000u) >> 7u;
        registers[Vx] <<= 1u;
    }

    // Skips next instruction if Vx != Vy
    void OP_9XY0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if(registers[Vx] != registers[Vy]){
            programCounter += 2;
        }
    }

    // Sets registerI to NNN
    void OP_ANNN(){
        uint16_t NNN = opcode & 0x0FFF;
        registerI = NNN;
    }

    // Jumps to address NNN + V0
    void OP_BNNN(){
        uint16_t NNN = opcode & 0x0FFF;
        programCounter = NNN + registers[0];
    }

    // Sets Vx to bitwise AND of NN and a random number from 0 to 255
    void OP_CXNN(){
        uint8_t Vx = opcode & 0x0F00u;
        uint8_t NN = opcode & 0x00FFu;
        registers[Vx] = NN & distribution(rng);
    }

    // TODO Draws sprite at coordinate (Vx, Vy) that has a width of 8 pixels and height of N pixels, VF set to 1 if
    //  any pixels are flipped from set to unset, 0 if not
    void OP_DXYN(){

    }

    // TODO Skips next instruction if key stored in Vx is pressed
    void OP_EX9E(){

    }

    // TODO Skips next instruction if key stored in Vx is not pressed
    void OP_EXA1(){

    }

    // TODO Sets Vx to value of delayTimer
    void OP_FX07(){

    }

    // TODO Waits for key press, and then stores it in Vx, blocks all further instructions until key press
    void OP_FX0A(){

    }

    // TODO Sets delayTimer to Vx
    void OP_FX15(){

    }

    // TODO Sets soundTimer to Vx
    void OP_FX18(){

    }

    // TODO Adds Vx to registerI, VF does not change
    void OP_FX1E(){

    }

    // TODO Sets registerI to location of the sprite in character Vx
    void OP_FX29(){

    }

    // TODO Stores binary-coded decimal representation of Vx, hundreds digit at registerI, tens digit at
    //  registerI+1, ones at registerI+2
    void OP_FX33(){

    }

    // TODO Stores values from V0 to Vx in memory, inclusive, starting at registerI (registerI is unmodified)
    void OP_FX55(){

    }

    // TODO Fills values from V0 to Vx from memory, inclusive, starting at registerI (registerI is unmodified)
    void OP_FX65(){

    }

    void printMemory(){
        for(int i = 0; i < 128; i++){
            for(int j = 0; j < 32; j++){
                std::cout << std::hex << int(memory[i * 32 + j]) << " ";
            }
            std::cout << std::endl;
        }
    }

    void printROM(){
        bool flag = false;
        for(int i = 0; i < 128; i++){
            for(int j = 0; j < 32; j++){
                flag = i * 32 + j > 0x200 + fileSize;
                if(i * 32 + j >= 0x200 && !flag){
                    std::cout << std::hex << int(memory[i * 32 + j]) << " ";
                }
            }
            if(!flag){
                std::cout << std::endl;
            }
        }
    }

    void printInfo(){
        std::cout << "Program Counter: " << std::hex << programCounter << std::endl;

        std::cout << "Registers: ";
        for(int i = 0; i < 16; i++){
            std::cout << std::hex << int(registers[i]) << " ";
        }
        std::cout << std::endl;

        std::cout << "Stack: ";
        for(int i = 0; i < 16; i++){
            std::cout << std::hex << int(stack[i]) << " ";
        }
        std::cout << std::endl;

        std::cout << "Stack Pointer: " << std::hex << int(stackPointer) << std::endl;

        std::cout << std::endl;
    }
};

// TODO Video and Audio processing, use SDL?

void testSuite(){
    Chip8 chip = Chip8();
    chip.loadROM("test_suite");
    chip.opcode = 0x60FFu;
    chip.OP_6XNN();
    chip.opcode = 0x800E;
    chip.OP_8XYE();
    chip.printInfo();
}

int main() {
    testSuite();

    // TODO Initialize visual interface and chip, run cycles based on a delay (default at 60Hz), decrement counters
    return 0;
}