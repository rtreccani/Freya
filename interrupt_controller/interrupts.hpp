#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdio.h>
#include <registers.hpp>
#include <ram.hpp>

typedef enum{
    INT_VBLANK,
    INT_LCDC_STATUS,
    INT_TIMER_OVERFLOW,
    INT_SER_TRANS_COMPLETE,
    INT_IO_NEGEDGE,
    INT_NONE,
} int_name_t;

typedef struct{
    int_name_t type;
    bool enabled;
    bool active;
} interrupt_context_t;


class interrupts{
    public:
        void service(int ticks);
        RAM* ram;
        void set_ram_ptr(RAM*);
         bool vsync_need_service = false;
    private:
        uint8_t current_vline = 0;
        int last_vline_tick_count = 0;

};

#endif