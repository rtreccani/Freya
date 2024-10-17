#include <interrupts.hpp>
#include <ram.hpp>
#include <stdio.h>

#define VLINE_ADDR 0xFF44
#define VLINE_MAX 153
#define TICKS_PER_VLINE 456

extern ram_t ram;
interrupts_t interrupts;

void interrupts_service(int ticks){
    if(ticks - interrupts.last_vline_tick_count >= TICKS_PER_VLINE){
        interrupts.last_vline_tick_count = ticks;
        if(interrupts.current_vline == 153){
            interrupts.current_vline = 0;
            interrupts.vsync_need_service = true;

        } else{
            interrupts.current_vline++;
        }
        ram_write8(VLINE_ADDR, interrupts.current_vline);
        printf("NEW VLINE JUST DROPPED\n");
    }
}