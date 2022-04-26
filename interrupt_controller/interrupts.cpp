#include <interrupts.hpp>
#include <ram.hpp>
#include <stdio.h>

#define VLINE_ADDR 0xFF44
#define VLINE_MAX 153
#define TICKS_PER_VLINE 456

void interrupts::set_ram_ptr(RAM* p_ram){
    ram = p_ram;
}

void interrupts::service(int ticks){
    if(ticks - last_vline_tick_count >= TICKS_PER_VLINE){
        last_vline_tick_count = ticks;
        if(current_vline == 153){
            current_vline = 0;
            vsync_need_service = true;
            
        } else{
            current_vline++;
        }
        ram->write8(VLINE_ADDR, current_vline);
        printf("NEW VLINE JUST DROPPED\n");
    }
}