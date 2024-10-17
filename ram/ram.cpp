#include <ram.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t ram_read8_internal(uint16_t);
static void ram_check_for_special_read(uint16_t native_address);
static void ram_check_for_special_write(uint16_t native_address, uint8_t value);

ram_t ram;

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

static void ram_check_for_special_write(uint16_t native_address, uint8_t value)
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

static void ram_check_for_special_read(uint16_t native_address)
{
    for(size_t i = 0; i < (sizeof(special_addresses)/sizeof(special_addresses[0])); i++)
    {
        if(native_address == special_addresses[i].native_address)
        {
            printf("reading 0x%02X from 0x%04X which is %s\n", ram_read8_internal(native_address), native_address, special_addresses[i].friendly_name);
            return;
        }
    }
}

uint8_t ram_read8(uint16_t native_address)
{
    ram_check_for_special_read(native_address);
    return ram_read8_internal(native_address);
}

static uint8_t ram_read8_internal(uint16_t native_address){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case ROM_0:
            return(ram.ROM_0_data[map.address]);
            break;
        case ROM_SW:
            return(ram.ROM_SW_data[map.address]);
            break;
        case VRAM:
            // printf("read from VRAM @0x%04X\n", native_address);
            return(ram.VRAM_data[map.address]);
            break;
        case RAM_SW:
            return(ram.RAM_SW_data[map.address]);
            break;
        case RAM_0:
            return(ram.RAM_0_data[map.address]);
            break;
        case RAM_0_ECHO:
            return(ram.RAM_0_data[map.address]);
            break;
        case SPRITE_ATTR:
            return(ram.SPRITE_ATTR_data[map.address]);
            break;
        case IO:
            return(ram.IO_data[map.address]);
            break;
        case INTERNAL_RAM_2:
            return(ram.INTERNAL_RAM_2_data[map.address]);
            break;
        case INTERRUPT_EN:
            return(ram.INTERRUPT_EN_data);
            break;
        default:
            printf("attempted read from non-valid section @0x%04X\n", native_address);
            fflush(stdout);
            return(0);
    }
}

uint16_t ram_read16(uint16_t native_address){
    uint16_t result = 0x0000;
    result += (ram_read8(native_address));
    result += (ram_read8(native_address+1) << 8);
    return result;
    printf("16 bit read of 0x%04X, result 0x%04X\n",native_address, result);
}

void ram_write16(uint16_t native_address, uint16_t value){
    printf("16 bit write of 0x%04X to 0x%04X\n", value, native_address);
    ram_write8(native_address, value & 0xFF);
    ram_write8(native_address+1, value>>8 & 0xFF);
}

void ram_write8(uint16_t native_address, uint8_t val){
    ram_check_for_special_write(native_address, val);
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            // printf("write to VRAM @0x%04X\n", native_address);
            ram.VRAM_data[map.address] = val;
            break;
        case RAM_SW:
            ram.RAM_SW_data[map.address] = val;
            break;
        case RAM_0:
            ram.RAM_0_data[map.address] = val;
            break;
        case RAM_0_ECHO:
            ram.RAM_0_data[map.address] = val;
            break;
        case SPRITE_ATTR:
            ram.SPRITE_ATTR_data[map.address] = val;
            break;
        case IO:
            ram.IO_data[map.address] = val;
            break;
        case INTERNAL_RAM_2:
            ram.INTERNAL_RAM_2_data[map.address] = val;
            break;
        case INTERRUPT_EN:
            ram.INTERRUPT_EN_data = val;
            break;
        default:
            printf("attempted write to non-valid section @0x%04X\n", native_address);
    }
}

void ram_inc(uint16_t native_address){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            ram.VRAM_data[map.address]++;
            break;
        case RAM_SW:
            ram.RAM_SW_data[map.address]++;
            break;
        case RAM_0:
            ram.RAM_0_data[map.address]++;
            break;
        case SPRITE_ATTR:
            ram.SPRITE_ATTR_data[map.address]++;
            break;
        case IO:
            ram.IO_data[map.address]++;
            break;
        case INTERNAL_RAM_2:
            ram.INTERNAL_RAM_2_data[map.address]++;
            break;
        case INTERRUPT_EN:
            ram.INTERRUPT_EN_data++;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void ram_dec(uint16_t native_address){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            ram.VRAM_data[map.address]--;
            break;
        case RAM_SW:
            ram.RAM_SW_data[map.address]--;
            break;
        case RAM_0:
            ram.RAM_0_data[map.address]--;
            break;
        case SPRITE_ATTR:
            ram.SPRITE_ATTR_data[map.address]--;
            break;
        case IO:
            ram.IO_data[map.address]--;
            break;
        case INTERNAL_RAM_2:
            ram.INTERNAL_RAM_2_data[map.address]--;
            break;
        case INTERRUPT_EN:
            ram.INTERRUPT_EN_data--;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void ram_add(uint16_t native_address, uint8_t val){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            ram.VRAM_data[map.address] += val;
            break;
        case RAM_SW:
            ram.RAM_SW_data[map.address] += val;
            break;
        case RAM_0:
            ram.RAM_0_data[map.address] += val;
            break;
        case SPRITE_ATTR:
            ram.SPRITE_ATTR_data[map.address] += val;
            break;
        case IO:
            ram.IO_data[map.address] += val;
            break;
        case INTERNAL_RAM_2:
            ram.INTERNAL_RAM_2_data[map.address] += val;
            break;
        case INTERRUPT_EN:
            ram.INTERRUPT_EN_data += val;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void ram_sub(uint16_t native_address, uint8_t val){
    ram_map_t map = map_to_section(native_address);
    switch(map.section){
        case VRAM:
            ram.VRAM_data[map.address] -= val;
            break;
        case RAM_SW:
            ram.RAM_SW_data[map.address] -= val;
            break;
        case RAM_0:
            ram.RAM_0_data[map.address] -= val;
            break;
        case SPRITE_ATTR:
            ram.SPRITE_ATTR_data[map.address] -= val;
            break;
        case IO:
            ram.IO_data[map.address] -= val;
            break;
        case INTERNAL_RAM_2:
            ram.INTERNAL_RAM_2_data[map.address] -= val;
            break;
        case INTERRUPT_EN:
            ram.INTERRUPT_EN_data -= val;
            break;
        default:
            printf("attempted add to non-valid section @0x%04X\n", native_address);
    }
}

void ram_copy_cartridge(uint8_t* p_rom){
    memcpy(&ram.ROM_0_data, p_rom, ROM_0_END-ROM_0_START);
    memcpy(&ram.ROM_SW_data, p_rom + ROM_SW_START, ROM_SW_END - ROM_SW_START);
}