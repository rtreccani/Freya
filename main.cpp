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
    // printf("first few bytes %X, %X, %X\n", rom[0], rom[1], rom[2]);

    fclose(rom_ptr);

    cpu cpu(rom);
    cpu.reg.SP = 0xFFFE;
    do{
        // printf("PC: 0x%04X\n", cpu.reg.PC);
        char C = getchar();
        if(C == 'r'){
            cpu.dump_registers();
        }
        else if (C == 'd'){
            for(int i = 0; i < 10; i++){
                cpu.execute_opcode();
            }
        }
        else if (C == 'c'){
            for(int i = 0; i < 100; i++){
                cpu.execute_opcode();
            }
        }
        else if (C == 'm'){
            for(int i = 0; i < 1000; i++){
                cpu.execute_opcode();
            }
        }
        else if (C == 'w'){
            int watchpoint;
            printf("Enter an address to watch for: ");
            scanf("%X", &watchpoint);
            while(cpu.reg.PC <= watchpoint){
                if(cpu.execute_opcode() < 0){
                    cpu.dump_registers();
                    exit(0);
                }// 29B
            }
        }
        else if (C == 'v'){
            int v_line;
            printf("Enter a new vline: ");
            scanf("%d", &v_line);
            cpu.ram[0xFF44 - MEM_RAM_START] = (v_line & 0xFF);
        }
        else if (C == 'i'){
            int address;
            printf("Enter a memory address to inspect: ");
            scanf("%X", &address);
            printf("0x%02X\n", cpu.ram[address - MEM_RAM_START]);
        }

    } while(cpu.execute_opcode() >= 0);
    cpu.dump_registers();

}