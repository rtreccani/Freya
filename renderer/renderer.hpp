#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <ram.hpp>
#include <stdio.h>


void renderer_init();
static void renderer_render_tile(int, int, int);
void renderer_render_all_tiles();
void renderer_blank();
void renderer_render_screen();


typedef struct{
    public:
        SDL_Window* win;
        SDL_Renderer* ren;
        int scale = 2;
} renderer_t;

#endif