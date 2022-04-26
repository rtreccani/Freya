#include <SDL2/SDL.h>
#include <renderer.hpp>
#include <ram.hpp>
#include <stdio.h>

int decode_2bpp(uint8_t lb, uint8_t hb, int x){
    uint8_t retval = 0;
    retval += ((lb & (0x01 << x)) >> x);
    retval += ((lb & (0x01 << x)) >> (x-1));
    return retval;
}

renderer::renderer(){
    SDL_Init(SDL_INIT_VIDEO | SDL_FLIP_HORIZONTAL);
    SDL_CreateWindowAndRenderer(32*8*scale, 32*8*scale, 0, &win, &ren);
    blank();
}

void renderer::blank(){
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);
}

void renderer::set_ram(RAM* p_ram_new){
    p_ram = p_ram_new;
}

#define BYTES_PER_TILE 16
#define BYTES_PER_LINE 2

void renderer::render_tile(int x_offset, int y_offset, int tile_offset){
    uint8_t tilemap[64];
    int br;
    SDL_Rect a;
    uint16_t ram_offset;
    for(int v = 0; v < 8; v++){ //2 bytes per vhop
        ram_offset = (tile_offset * BYTES_PER_TILE) + (v * BYTES_PER_LINE);
        for(int h = 0; h < 8; h++){
            tilemap[(8*v)+h] = decode_2bpp(p_ram->read8(0x8000 + ram_offset), p_ram->read8(0x8001 + ram_offset), h);
            br = 63*tilemap[h+(8*v)];
            SDL_SetRenderDrawColor(ren, br, br, br, 255);
            a = {
            ((x_offset + 1) * 8 * scale) + (8-(h * scale)), /*tile offset + pixel offset*/
            (y_offset * 8 * scale) + (v * scale), /*tile offset + pixel offset*/
            scale,
            scale,
            };
            SDL_RenderFillRect(ren, &a);
        }
    }
}

void renderer::render_all_tiles(){
    blank();
    for(int y = 0; y < 15; y++){
        for(int x = 0; x < 15; x++){
            renderer::render_tile(x, y, x+(16*y));
            // SDL_Delay(25);
        }
    }
}

void renderer::render_screen(){
    blank();
    scale = 2;
    for(uint16_t y = 0; y < 32; y++){
        for(uint16_t x = 0; x < 32; x++){
            render_tile(x, y, p_ram->read8(0x9800 + (x+(32*y))));
        }
    }
    SDL_RenderPresent(ren);
}