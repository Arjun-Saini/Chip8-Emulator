cmake_minimum_required(VERSION 3.27)
project(Chip8_Emulator)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
add_executable(Chip8_Emulator main.cpp)
target_link_libraries(Chip8_Emulator PRIVATE SDL2::SDL2)