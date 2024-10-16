#include <ram.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ram_map_t map_to_section(uint16_t native_address){
    ram_map_t ret;
    if(native_address <= ROM_0_END){
        ret.section = ROM_0;
        ret.address = native_address - ROM_0_START;

    } else
    if(native_address <= ROM_SW_END){
        ret.section = ROM_SW;
        ret.address = native_address - ROM_SW_START;
    } else
    if(native_address <= VRAM_END){
        ret.section = VRAM;
        ret.address = native_address - VRAM_START;
    } else
    if(native_address <= RAM_SW_END){
        ret.section = RAM_SW;
        ret.address = native_address - RAM_SW_START;
    } else
    if(native_address <= RAM_0_END){
        ret.section = RAM_0;
        ret.address = native_address - RAM_0_START;
    } else
    if(native_address <= RAM_0_ECHO_END){
        ret.section = RAM_0_ECHO;
        ret.address = native_address - RAM_0_ECHO_START;
    } else
    if(native_address <= SPRITE_ATTR_END){
        ret.section = SPRITE_ATTR;
        ret.address = native_address - SPRITE_ATTR_START;
    } else
    if(native_address <= EMPTY_IO_END){
        ret.section = EMPTY_IO;
        ret.address = native_address - EMPTY_IO_START;
    } else
    if(native_address <= IO_END){
        ret.section = IO;
        ret.address = native_address - IO_START;
    } else
    if(native_address <= EMPTY_IO_2_END){
        ret.section = EMPTY_IO_2;
        ret.address = native_address - EMPTY_IO_2_START;
    } else
    if(native_address <= INTERNAL_RAM_2_END){
        ret.section = INTERNAL_RAM_2;
        ret.address = native_address - INTERNAL_RAM_2_START;
    } else
    if(native_address <= INTERRUPT_EN_REGISTER){
        ret.section = INTERRUPT_EN;
        ret.address = native_address - INTERRUPT_EN_REGISTER;
    }
    else{
        printf("Somehow managed to get invalid memory!!!\n");
    }
    return(ret);
}

special_memory_t special_addresses[] = {
    {0xFF41, "LCDC Status Reg",                                 NULL, NULL},
    {0xFF42, "LCD Scroll Y",                                    NULL, NULL},
    {0xFF43, "LCD Scroll X",                                    NULL, NULL},
    {0xFFFF, "Interrupt Enable",                                NULL, NULL},
    {0x2000, "Memory Bank Controller (Unused)",                 NULL, NULL},
    {0xFF00, "Joypad",                                          NULL, NULL},
    {0xFF01, "Serial Transfer Data",                            NULL, NULL},
    {0xFF02, "Serial Transfer Control",                         NULL, NULL},
    {0xFF04, "Divider Register",                                NULL, NULL},
    {0xFF05, "Timer Counter",                                   NULL, NULL},
    {0xFF06, "Timer Modulo",                                    NULL, NULL},
    {0xFF07, "Timer Control",                                   NULL, NULL},
    {0xFF0F, "Interrupt Flags",                                 NULL, NULL},
    {0xFF10, "Channel 1 Sweep Register",                        NULL, NULL},
    {0xFF11, "Channel 1 Sound Length/Wave Pattern Duty",        NULL, NULL},
};

void RAM::check_for_special_write(uint16_t native_address, uint8_t value)
{
    for(size_t i = 0; i < (sizeof(special_addresses)/sizeof(special_addresses[0])); i++)
    {
        if(native_address == special_addresses[i].native_address)
        {
            printf("writing 0x%02X to 0x%04X which is %s\n", value, native_address, special_addresses[i].friendly_name);
            return;
        }
    }
}

void RAM::check_for_special_read(uint16_t native_address)
{
    for(size_t i = 0; i < (sizeof(special_addresses)/sizeof(special_addresses[0])); i++)
    {
        if(native_address == special_addresses[i].native_address)
        {
            printf("reading 0x%02X from 0x%04X which is %s\n", read8_internal(native_address), native_address, special_addresses[i].friendly_name);
            return;
        }
    }
}

uint8_t RAM::read8(uint16_t native_address)
{
    check_for_special_read(native_address);
    return read8_internal(native_address);
}

uint8_t RAM::read8_internal(uint16_t native_address){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case ROM_0:
            return(ROM_0_data[map.address]);
            break;
        case ROM_SW:
            return(ROM_SW_data[map.address]);
            break;
        case VRAM:
            // printf("read from VRAM @0x%04X\n", native_address);
            return(VRAM_data[map.address]);
            break;
        case RAM_SW:
            return(RAM_SW_data[map.address]);
            break;
        case RAM_0:
            return(RAM_0_data[map.address]);
            break;
        case RAM_0_ECHO:
            return(RAM_0_data[map.address]);
            break;
        case SPRITE_ATTR:
            return(SPRITE_ATTR_data[map.address]);
            break;
        case IO:
            return(IO_data[map.address]);
            break;
        case INTERNAL_RAM_2:
            return(INTERNAL_RAM_2_data[map.address]);
            break;
        case INTERRUPT_EN:
            return(INTERRUPT_EN_data);
            break;
        default:
            printf("attempted read from non-valid section @0x%04X\n", native_address);
            fflush(stdout);
            return(0);
    }
}

uint16_t RAM::read16(uint16_t native_address){
    uint16_t result = 0x0000;
    result += (read8(native_address));
    result += (read8(native_address+1) << 8);
    return result;
    printf("16 bit read of 0x%04X, result 0x%04X\n",native_address, result);
}

void RAM::write16(uint16_t native_address, uint16_t value){
    printf("16 bit write of 0x%04X to 0x%04X\n", value, native_address);
    write8(native_address, value & 0xFF);
    write8(native_address+1, value>>8 & 0xFF);
}

void RAM::write8(uint16_t native_address, uint8_t val){
    check_for_special_write(native_address, val);
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            // printf("write to VRAM @0x%04X\n", native_address);
            VRAM_data[map.address] = val;
            break;
        case RAM_SW:
            RAM_SW_data[map.address] = val;
            break;
        case RAM_0:
            RAM_0_data[map.address] = val;
            break;
        case RAM_0_ECHO:
            RAM_0_data[map.address] = val;
            break;
        case SPRITE_ATTR:
            SPRITE_ATTR_data[map.address] = val;
            break;
        case IO:
            IO_data[map.address] = val;
            break;
        case INTERNAL_RAM_2:
            INTERNAL_RAM_2_data[map.address] = val;
            break;
        case INTERRUPT_EN:
            INTERRUPT_EN_data = val;
            break;
        default:
            printf("attempted write to non-valid section @0x%04X\n", native_address);
    }
}

void RAM::inc(uint16_t native_address){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            VRAM_data[map.address]++;
            break;
        case RAM_SW:
            RAM_SW_data[map.address]++;
            break;
        case RAM_0:
            RAM_0_data[map.address]++;
            break;
        case SPRITE_ATTR:
            SPRITE_ATTR_data[map.address]++;
            break;
        case IO:
            IO_data[map.address]++;
            break;
        case INTERNAL_RAM_2:
            INTERNAL_RAM_2_data[map.address]++;
            break;
        case INTERRUPT_EN:
            INTERRUPT_EN_data++;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void RAM::dec(uint16_t native_address){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            VRAM_data[map.address]--;
            break;
        case RAM_SW:
            RAM_SW_data[map.address]--;
            break;
        case RAM_0:
            RAM_0_data[map.address]--;
            break;
        case SPRITE_ATTR:
            SPRITE_ATTR_data[map.address]--;
            break;
        case IO:
            IO_data[map.address]--;
            break;
        case INTERNAL_RAM_2:
            INTERNAL_RAM_2_data[map.address]--;
            break;
        case INTERRUPT_EN:
            INTERRUPT_EN_data--;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void RAM::add(uint16_t native_address, uint8_t val){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            VRAM_data[map.address] += val;
            break;
        case RAM_SW:
            RAM_SW_data[map.address] += val;
            break;
        case RAM_0:
            RAM_0_data[map.address] += val;
            break;
        case SPRITE_ATTR:
            SPRITE_ATTR_data[map.address] += val;
            break;
        case IO:
            IO_data[map.address] += val;
            break;
        case INTERNAL_RAM_2:
            INTERNAL_RAM_2_data[map.address] += val;
            break;
        case INTERRUPT_EN:
            INTERRUPT_EN_data += val;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void RAM::sub(uint16_t native_address, uint8_t val){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            VRAM_data[map.address] -= val;
            break;
        case RAM_SW:
            RAM_SW_data[map.address] -= val;
            break;
        case RAM_0:
            RAM_0_data[map.address] -= val;
            break;
        case SPRITE_ATTR:
            SPRITE_ATTR_data[map.address] -= val;
            break;
        case IO:
            IO_data[map.address] -= val;
            break;
        case INTERNAL_RAM_2:
            INTERNAL_RAM_2_data[map.address] -= val;
            break;
        case INTERRUPT_EN:
            INTERRUPT_EN_data -= val;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void RAM::copy_cartridge(uint8_t* p_rom){
    memcpy(&ROM_0_data, p_rom, ROM_0_END-ROM_0_START);
    memcpy(&ROM_SW_data, p_rom + ROM_SW_START, ROM_SW_END - ROM_SW_START);
}