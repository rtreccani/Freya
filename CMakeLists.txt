cmake_minimum_required (VERSION 2.8.11)
project (Freya)

find_package(SDL2 COMPONENTS REQUIRED)

add_executable(Freya renderer/renderer.cpp ram/ram.cpp interrupt_controller/interrupts.cpp registers/registers.cpp cpu/cpu.cpp main.cpp)
target_include_directories(Freya PRIVATE renderer ram registers cpu interrupt_controller)
target_link_libraries(Freya SDL2)
target_compile_options(Freya PRIVATE -Wall -Werror)