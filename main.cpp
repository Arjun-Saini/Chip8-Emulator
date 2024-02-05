#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <chrono>
#include <SDL.h>

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
        memset(&graphics, 0, 64 * 32 * 4);
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
    void loadROM(std::string execPath, std::string fileName){
        std::filesystem::path fs_execPath(execPath);
        // Executable in folder
        std::string fullPath = fs_execPath.parent_path().parent_path().string() + "/ROMs/" + fileName + ".ch8";

        // Executable not in folder
//        std::string fullPath = p.parent_path().string() + "/ROMs/" + fileName + ".ch8";

        // Open ROM
        std::ifstream ROM(fullPath, std::ios::binary);
        ROM.seekg(0, std::ios::end);
        fileSize = ROM.tellg();
        ROM.seekg(0, std::ios::beg);

        // Load ROM into buffer
        char buffer[fileSize];
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
        uint16_t NNN = opcode & 0x0FFFu;

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
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t NN = opcode & 0x00FFu;

        if(registers[Vx] == NN){
            programCounter += 2;
        }
    }

    // Skips next instruction if Vx != NN
    void OP_4XNN(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t NN = opcode & 0x00FFu;

        if(registers[Vx] != NN){
            programCounter += 2;
        }
    }

    // Skips next instruction if Vx == Vy
    void OP_5XY0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

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

    // Draws sprite at coordinate (Vx, Vy) that has a width of 8 pixels and height of N pixels, sprite data is
    //  read from memory starting at registerI, VF set to 1 if any pixels are flipped from set to unset, 0 if not
    void OP_DXYN(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        uint8_t N = opcode & 0x000Fu;

        uint8_t xPos = registers[Vx];
        uint8_t yPos = registers[Vy];

        registers[0xFu] = 0;

        for(int i = 0; i < N; i++){
            if(yPos + i >= 32){
                break;
            }
            uint8_t spriteRow = memory[registerI + i];
            for(int j = 0; j < 8; j++){
                if(xPos + i >= 64){
                    break;
                }
                uint8_t bit = (spriteRow >> (7 - j)) & 0b1u;
                uint32_t* target = &graphics[xPos + j][yPos + i];
                if(bit && *target){
                    registers[0xFu] = 1;
                }
                *target ^= bit;
            }
        }
    }

    // Skips next instruction if key stored in Vx is pressed
    void OP_EX9E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        if(keys[registers[Vx]]){
            programCounter += 2;
        }
    }

    // Skips next instruction if key stored in Vx is not pressed
    void OP_EXA1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        if(!keys[registers[Vx]]){
            programCounter += 2;
        }
    }

    // Sets Vx to value of delayTimer
    void OP_FX07(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        registers[Vx] = delayTimer;
    }

    // Waits for key press, and then stores it in Vx, blocks all further instructions until key press
    void OP_FX0A(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        if(keys[0]){
            registers[Vx] = 0;
        }else if(keys[1]){
            registers[Vx] = 1;
        }else if(keys[2]){
            registers[Vx] = 2;
        }else if(keys[3]){
            registers[Vx] = 3;
        }else if(keys[4]){
            registers[Vx] = 4;
        }else if(keys[5]){
            registers[Vx] = 5;
        }else if(keys[6]){
            registers[Vx] = 6;
        }else if(keys[7]){
            registers[Vx] = 7;
        }else if(keys[8]){
            registers[Vx] = 8;
        }else if(keys[9]){
            registers[Vx] = 9;
        }else if(keys[10]){
            registers[Vx] = 10;
        }else if(keys[11]){
            registers[Vx] = 11;
        }else if(keys[12]){
            registers[Vx] = 12;
        }else if(keys[13]){
            registers[Vx] = 13;
        }else if(keys[14]){
            registers[Vx] = 14;
        }else if(keys[15]){
            registers[Vx] = 15;
        }else{
            programCounter -= 2;
        }
    }

    // Sets delayTimer to Vx
    void OP_FX15(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        delayTimer = registers[Vx];
    }

    // Sets soundTimer to Vx
    void OP_FX18(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        soundTimer = registers[Vx];
    }

    // Add Vx to registerI, VF does not change
    void OP_FX1E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        registerI += registers[Vx];
    }

    // Sets registerI to location of the font data for character Vx
    void OP_FX29(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        registerI = registers[Vx] * 5 + 0x50;
    }

    // Stores binary-coded decimal representation of Vx, hundreds digit at registerI, tens digit at
    //  registerI+1, ones at registerI+2
    void OP_FX33(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t number = registers[Vx];

        memory[registerI + 2] = number % 10;
        number /= 10;
        memory[registerI + 1] = number % 10;
        number /= 10;
        memory[registerI] = number % 10;
    }

    // Stores values from V0 to Vx in memory, inclusive, starting at registerI (registerI is unmodified)
    void OP_FX55(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for(int i = 0; i <= Vx; i++){
            memory[registerI + i] = registers[i];
        }
    }

    // Fills values from V0 to Vx from memory, inclusive, starting at registerI (registerI is unmodified)
    void OP_FX65(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for(int i = 0; i <= Vx; i++){
            registers[i] = memory[registerI + i];
        }
    }

    // Decodes and processes opcode
    void decodeOpcode(){
        switch (opcode & 0xF000u) {
            case 0x0000u:
                switch (opcode & 0x000Fu) {
                    case 0x0u:
                        OP_00E0();
                        break;
                    case 0xEu:
                        OP_00EE();
                        break;
                    default: break;
                }
                break;
            case 0x1000u:
                OP_1NNN();
                break;
            case 0x2000u:
                OP_2NNN();
                break;
            case 0x3000u:
                OP_3XNN();
                break;
            case 0x4000u:
                OP_4XNN();
                break;
            case 0x5000u:
                OP_5XY0();
                break;
            case 0x6000u:
                OP_6XNN();
                break;
            case 0x7000u:
                OP_7XNN();
                break;
            case 0x8000u:
                switch (opcode & 0x000Fu) {
                    case 0x0u:
                        OP_8XY0();
                        break;
                    case 0x1u:
                        OP_8XY1();
                        break;
                    case 0x2u:
                        OP_8XY2();
                        break;
                    case 0x3u:
                        OP_8XY3();
                        break;
                    case 0x4u:
                        OP_8XY4();
                        break;
                    case 0x5u:
                        OP_8XY5();
                        break;
                    case 0x6u:
                        OP_8XY6();
                        break;
                    case 0x7u:
                        OP_8XY7();
                        break;
                    case 0xEu:
                        OP_8XYE();
                        break;
                    default: break;
                }
                break;
            case 0x9000u:
                OP_9XY0();
                break;
            case 0xA000u:
                OP_ANNN();
                break;
            case 0xB000u:
                OP_BNNN();
                break;
            case 0xC000u:
                OP_CXNN();
                break;
            case 0xD000u:
                OP_DXYN();
                break;
            case 0xE000u:
                switch (opcode & 0x000Fu) {
                    case 0x1u:
                        OP_EXA1();
                        break;
                    case 0xEu:
                        OP_EX9E();
                        break;
                    default: break;
                }
                break;
            case 0xF000u:
                switch (opcode & 0x00FFu) {
                    case 0x07u:
                        OP_FX07();
                        break;
                    case 0x0Au:
                        OP_FX0A();
                        break;
                    case 0x15u:
                        OP_FX15();
                        break;
                    case 0x18u:
                        OP_FX18();
                        break;
                    case 0x1Eu:
                        OP_FX1E();
                        break;
                    case 0x29u:
                        OP_FX29();
                        break;
                    case 0x33u:
                        OP_FX33();
                        break;
                    case 0x55u:
                        OP_FX55();
                        break;
                    case 0x65u:
                        OP_FX65();
                        break;
                    default: break;
                }
                break;
            default: break;
        }
    }

    // Emulates a single processor cycle
    void cycle(){
        getOpcode();
        decodeOpcode();

        if(delayTimer > 0){
            delayTimer--;
        }
        if(soundTimer > 0){
            soundTimer--;
        }
    }

    // Prints full content of memory
    void printMemory(){
        for(int i = 0; i < 128; i++){
            for(int j = 0; j < 32; j++){
                std::cout << std::hex << int(memory[i * 32 + j]) << " ";
            }
            std::cout << std::endl;
        }
    }

    // Prints section of memory after 0x200 that contains data
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

    // Prints information about system variables
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

        std::cout << "Register I: " << std::hex << int(registerI) << std::endl;

        std::cout << std::endl;
    }

    // Prints graphics array
    void printGraphics(){
        for(int i = 0; i < 32; i++){
            for(int j = 0; j < 64; j++){
                std::cout << graphics[j][i];
            }
            std::cout << std::endl;
        }
    }
};

uint64_t getTime(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

// TODO Video and Audio processing, use SDL?

int main(int argc, char * argv[]) {
    // Initialize visual interface and chip, run cycles based on a delay (default at 60Hz)
    Chip8 cpu = Chip8();
    cpu.loadROM(argv[0], "Breakout");
    int scale = 10;
    const int Hz = 60;
    const int ms_delta = 1000 / Hz;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(64 * scale, 32 * scale, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, scale, scale);
    SDL_Point points[64 * 32];

    uint64_t lastTime = getTime();
    uint64_t currentTime;
    
    bool isRunning = true;
    while(isRunning){
        currentTime = getTime();

        // TODO play sound

        // Update key inputs
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT){
                isRunning = false;
            }else if(e.type == SDL_KEYDOWN){
                switch (e.key.keysym.sym) {
                    case SDLK_1:
                        cpu.keys[0x1] = 1;
                        break;
                    case SDLK_2:
                        cpu.keys[0x2] = 1;
                        break;
                    case SDLK_3:
                        cpu.keys[0x3] = 1;
                        break;
                    case SDLK_4:
                        cpu.keys[0xC] = 1;
                        break;
                    case SDLK_q:
                        cpu.keys[0x4] = 1;
                        break;
                    case SDLK_w:
                        cpu.keys[0x5] = 1;
                        break;
                    case SDLK_e:
                        cpu.keys[0x6] = 1;
                        break;
                    case SDLK_r:
                        cpu.keys[0xD] = 1;
                        break;
                    case SDLK_a:
                        cpu.keys[0x7] = 1;
                        break;
                    case SDLK_s:
                        cpu.keys[0x8] = 1;
                        break;
                    case SDLK_d:
                        cpu.keys[0x9] = 1;
                        break;
                    case SDLK_f:
                        cpu.keys[0xE] = 1;
                        break;
                    case SDLK_z:
                        cpu.keys[0xA] = 1;
                        break;
                    case SDLK_x:
                        cpu.keys[0x0] = 1;
                        break;
                    case SDLK_c:
                        cpu.keys[0xB] = 1;
                        break;
                    case SDLK_v:
                        cpu.keys[0xF] = 1;
                        break;
                }
            }else if(e.type == SDL_KEYUP){
                switch (e.key.keysym.sym) {
                    case SDLK_1:
                        cpu.keys[0x1] = 0;
                        break;
                    case SDLK_2:
                        cpu.keys[0x2] = 0;
                        break;
                    case SDLK_3:
                        cpu.keys[0x3] = 0;
                        break;
                    case SDLK_4:
                        cpu.keys[0xC] = 0;
                        break;
                    case SDLK_q:
                        cpu.keys[0x4] = 0;
                        break;
                    case SDLK_w:
                        cpu.keys[0x5] = 0;
                        break;
                    case SDLK_e:
                        cpu.keys[0x6] = 0;
                        break;
                    case SDLK_r:
                        cpu.keys[0xD] = 0;
                        break;
                    case SDLK_a:
                        cpu.keys[0x7] = 0;
                        break;
                    case SDLK_s:
                        cpu.keys[0x8] = 0;
                        break;
                    case SDLK_d:
                        cpu.keys[0x9] = 0;
                        break;
                    case SDLK_f:
                        cpu.keys[0xE] = 0;
                        break;
                    case SDLK_z:
                        cpu.keys[0xA] = 0;
                        break;
                    case SDLK_x:
                        cpu.keys[0x0] = 0;
                        break;
                    case SDLK_c:
                        cpu.keys[0xB] = 0;
                        break;
                    case SDLK_v:
                        cpu.keys[0xF] = 0;
                        break;
                }
            }
        }

        if(currentTime - lastTime >= ms_delta){
            lastTime = currentTime;
            cpu.cycle();

            // Clear screen
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            // Draw pixels
            for(int i = 0; i < 64; i++){
                for(int j = 0; j < 32; j++){
                    if(cpu.graphics[i][j]){
                        SDL_RenderDrawPoint(renderer, i, j);
                    }
                }
            }
            SDL_RenderPresent(renderer);
        }
    }

    return 0;
}