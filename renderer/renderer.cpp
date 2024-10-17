#include <SDL2/SDL.h>
#include <renderer.hpp>
#include <ram.hpp>
#include <stdio.h>

extern ram_t ram;

renderer_t renderer = {
    .scale = 2,
};

static int decode_2bpp(uint8_t lb, uint8_t hb, int x){
    uint8_t retval = 0;
    retval += ((lb & (0x01 << x)) >> x);
    retval += ((lb & (0x01 << x)) >> (x-1));
    return retval;
}

void renderer_init(){
    SDL_Init(SDL_INIT_VIDEO | SDL_FLIP_HORIZONTAL);
    SDL_CreateWindowAndRenderer(32*8*renderer.scale, 32*8*renderer.scale, 0, &renderer.win, &renderer.ren);
    renderer_blank();
}

void renderer_blank(){
    SDL_SetRenderDrawColor(renderer.ren, 0, 0, 0, 0);
    SDL_RenderClear(renderer.ren);
    SDL_RenderPresent(renderer.ren);
}

#define BYTES_PER_TILE 16
#define BYTES_PER_LINE 2

static void renderer_render_tile(int x_offset, int y_offset, int tile_offset){
    uint8_t tilemap[64];
    int br;
    SDL_Rect a;
    uint16_t ram_offset;
    for(int v = 0; v < 8; v++){ //2 bytes per vhop
        ram_offset = (tile_offset * BYTES_PER_TILE) + (v * BYTES_PER_LINE);
        for(int h = 0; h < 8; h++){
            tilemap[(8*v)+h] = decode_2bpp(ram_read8(0x8000 + ram_offset), ram_read8(0x8001 + ram_offset), h);
            br = 63*tilemap[h+(8*v)];
            SDL_SetRenderDrawColor(renderer.ren, br, br, br, 255);
            a = {
            ((x_offset + 1) * 8 * renderer.scale) + (8-(h * renderer.scale)), /*tile offset + pixel offset*/
            (y_offset * 8 * renderer.scale) + (v * renderer.scale), /*tile offset + pixel offset*/
            renderer.scale,
            renderer.scale,
            };
            SDL_RenderFillRect(renderer.ren, &a);
        }
    }
}

void renderer_render_all_tiles(){
    renderer_blank();
    for(int y = 0; y < 15; y++){
        for(int x = 0; x < 15; x++){
            renderer_render_tile(x, y, x+(16*y));
            // SDL_Delay(25);
        }
    }
}

void renderer_render_screen(){
    renderer_blank();
    renderer.scale = 2;
    for(uint16_t y = 0; y < 32; y++){
        for(uint16_t x = 0; x < 32; x++){
            renderer_render_tile(x, y, ram_read8(0x9800 + (x+(32*y))));
        }
    }
    SDL_RenderPresent(renderer.ren);
}