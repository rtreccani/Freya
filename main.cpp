#include <stdio.h>
#include <SFML/Graphics.hpp>
#include "registers.hpp"
#include <interrupts.hpp>
#include <ram.hpp>
#include "cpu.hpp"

// sf::RenderWindow screen(sf::VideoMode(160*3,144*3));

// void render_vram(uint8_t* p_vram){
//     screen.clear(sf::Color::Black);
const char* fname = "../tetris.gb";

int main(){
    class cpu cpu;
    // cpu.reg.SP = 0xFFFE;
    cpu.open_file(fname);
    while(true){
        char C = getchar();
        switch(C){

            case 'w':
                int watchpoint;
                printf("enter a watchpoint: ");
                scanf("%X", &watchpoint);
                printf("\n");
                while(cpu.reg.PC != watchpoint){
                    if(cpu.execute_opcode() < 0){
                        cpu.dump_registers();
                        exit(0);
                    }
                }
                break;

            case 'd':
                for(int i = 0; i < 10; i++){
                    cpu.execute_opcode();
                }
                break;

            case 'c':
                for(int i = 0; i < 100; i++){
                    cpu.execute_opcode();
                }
                break;

            case 'm':
                for(int i = 0; i < 1000; i++){
                    cpu.execute_opcode();
                }
                break;
            
            case 'r':
                cpu.dump_registers();
                break;
            
            case 'v':
                int v_line;
                printf("Enter a new vline: ");
                scanf("%d", &v_line);
                cpu.ram.write8(0xFF44, v_line & 0xFF);
                break;
            case 'i':
                int address;
                printf("Enter a memory address to inspect: ");
                scanf("%X", &address);
                printf("0x%02X\n", cpu.ram.read8(address));
                break;

            case 'g':
                while(cpu.execute_opcode()>=0){}
                cpu.dump_registers();
                exit(0);
                break;
            
            case 's':
                cpu.dump_stack();
                break;

            default:
                if(cpu.execute_opcode() < 0){
                    cpu.dump_registers();
                    exit(0);
                }
                break;
        }
    }
}