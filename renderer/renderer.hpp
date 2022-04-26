#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <ram.hpp>
#include <stdio.h>



class renderer{
    public:
        SDL_Window* win;
        SDL_Renderer* ren;
        RAM* p_ram;
        renderer();
        void set_ram(RAM*);
        void render_tile(int, int, int);
        void render_all_tiles();
        void blank();
        void render_screen();
        int scale = 2;
};


#endif