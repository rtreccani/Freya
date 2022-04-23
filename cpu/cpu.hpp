#ifndef CPU_H
#define CPU_H

#include <registers.hpp>
#include <stddef.h>

#define MEM_RAM_START 0xC000

struct opcode{
    const char* str_name;
    int operand_length;
    reg_ind_t reg_ind;
    reg_ind_t reg_ind_two;
    void(* opcode_pointer)(uint16_t, registers*, uint8_t*, reg_ind_t, reg_ind_t);
};

class cpu{
    public:
        uint8_t ram[8192];
        registers reg;
        uint8_t* p_rom;
        cpu(uint8_t*);
        int execute_opcode();
        void jump(int);
        void dump_registers();
        uint16_t get_operand(opcode);
        uint64_t ticks;
};


#endif /*CPU_H*/