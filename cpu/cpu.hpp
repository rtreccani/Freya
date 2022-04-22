#ifndef CPU_H
#define CPU_H

#include <registers.hpp>
#include <stddef.h>

#define MEM_RAM_START 0xC000

struct opcode{
    const char* str_name;
    int operand_length;
    reg_ind_t reg_ind;
    void(* opcode_pointer)(uint16_t, registers_t*, uint8_t*, reg_ind_t);
    // int ticks;
};

class cpu{
    public:
        uint8_t ram[8192];
        registers_t reg;
        uint8_t* p_rom;
        cpu(uint8_t*);
        int execute_opcode();
        void jump(int);
        void dump_registers();
};


#endif /*CPU_H*/