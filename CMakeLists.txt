cmake_minimum_required (VERSION 2.8.11)
project (Freya)

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

add_executable(Freya interrupt_controller/interrupts.cpp ram/ram.cpp registers/registers.cpp cpu/cpu.cpp main.cpp)
target_include_directories(Freya PRIVATE ram registers cpu interrupt_controller)
target_link_libraries(Freya sfml-graphics sfml-audio)