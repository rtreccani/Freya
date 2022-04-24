#ifndef RAM_H
#define RAM_H

#include <interrupts.hpp>

class RAM{
    public:
        void set_interrupt_backend(interrupts*);
        uint8_t read8(uint16_t);
        void write8(uint16_t, uint8_t);
    private:
        uint8_t data[8192];
};

#endif