#ifndef CPU_H
#define CPU_H

#include <registers.hpp>
#include <stddef.h>

struct opcode{
    const char* str_name;
    int operand_length;
    reg_ind_t reg_ind;
    void(* opcode_pointer)(uint16_t, registers_t*, reg_ind_t);
    // int ticks;
};

class cpu{
    public:
        registers_t reg;
        uint8_t* p_rom;
        cpu(uint8_t*);
        int execute_opcode();
        void jump(int);
        void dump_registers();
};


#endif /*CPU_H*/