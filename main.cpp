#include <stdio.h>
#include <SFML/Graphics.hpp>
#include "registers.hpp"
#include "cpu.hpp"


int main(){

    FILE* rom_ptr;
    rom_ptr = fopen("../tetris.gb", "rb");
    if(rom_ptr == NULL){
        printf("file not opened :(\n");
        return(-1);
    }
    fseek(rom_ptr, 0, SEEK_END);
    int rom_size = ftell(rom_ptr);
    printf("rom size: %d\n", rom_size);
    fseek(rom_ptr, 0, SEEK_SET);

    uint8_t* rom = (uint8_t*)malloc(rom_size);
    fread(rom, 1, rom_size, rom_ptr);
    printf("first few bytes %X, %X, %X\n", rom[0], rom[1], rom[2]);

    fclose(rom_ptr);

    cpu cpu(rom);
    do{
        printf("PC: 0x%04X\n", cpu.reg.PC);
        getchar();
    } while(cpu.execute_opcode() >= 0);
    cpu.dump_registers();

}