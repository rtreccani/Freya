#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include "registers.hpp"
#include <interrupts.hpp>
#include <ram.hpp>
#include "cpu.hpp"

SDL_Window *win;// = SDL_CreateWindow("Hello World!", 100, 100, 80, 80, SDL_WINDOW_SHOWN);
SDL_Renderer *ren;// = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

int decode_2bpp(uint8_t lb, uint8_t hb, int x){
    uint8_t retval = 0;
    retval += ((lb & (0x01 << x)) >> x);
    retval += ((lb & (0x01 << x)) >> (x-1));
    return retval;
}

void render_tile(RAM* p_ram, int offset){
        //render
    uint8_t tilemap[64];
    for(int v = 0; v < 8; v++){ //2 bytes per vhop
        for(int h = 0; h < 8; h++){
            tilemap[(8*v)+h] = decode_2bpp(p_ram->read8(0x8000 + (offset * 16) + 2*v), p_ram->read8(0x8001 + (offset * 16) + 2*v), h);
        }
    }

    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++){
            printf("%02X ", tilemap[x+(8*y)]);
            int br = 63*tilemap[x+(8*y)];
            SDL_SetRenderDrawColor(ren, br, br, br, 255);
            const SDL_Rect a {(16*offset)+((16-(2*x))), 2*y, 2, 2};
            SDL_RenderFillRect(ren, &a);
        }
        printf("\n");
    }
    SDL_RenderPresent(ren);
}
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
                SDL_Init(SDL_INIT_VIDEO | SDL_FLIP_HORIZONTAL);
                SDL_CreateWindowAndRenderer(800, 18, 0, &win, &ren);
                SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
                SDL_RenderClear(ren);
                SDL_RenderPresent(ren);
                for(int offset = 0; offset < 50; offset++){
                    render_tile(&cpu.ram, offset);
                }
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