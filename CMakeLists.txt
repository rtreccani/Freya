cmake_minimum_required (VERSION 2.8.11)
project (Freya)

find_package(SDL2 COMPONENTS REQUIRED)

add_executable(Freya ram/ram.cpp interrupt_controller/interrupts.cpp registers/registers.cpp cpu/cpu.cpp main.cpp)
target_include_directories(Freya PRIVATE ram registers cpu interrupt_controller)
target_link_libraries(Freya SDL2)