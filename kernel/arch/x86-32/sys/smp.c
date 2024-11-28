#include "smp.h"
#include "../../../libk/stdiok.h"

#define MP_FLOATING_POINTER_ADDRESS 0xF5BA0
#define MP_CONFIGURATION_TABLE_ADDRESS 0xF5BB0
#define LOCAL_APIC  0xfee00000
#define MP_FLOATING_POINTER_SIGNATURE "_MP_"



// validates mp checksum
bool validate_mp_checksum(uint8_t *mp) 
{
    uint8_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += mp[i];
    }
    return sum == 0;
}

// searches the mp floating pointer
void* find_mp_floating_pointer(struct multiboot_info *mb_info) 
{
    if (!(mb_info->flags & (1 << 6))) {
        printf("Memory map not available.\n");
        return NULL;
    }

    uint32_t start_addr = 0x000F0000;
    uint32_t end_addr = 0x000FFFFF;
    for (uint32_t addr = start_addr; addr < end_addr; addr += 16) {
        char *ptr = (char *)addr;
        if (memcmp(ptr, MP_FLOATING_POINTER_SIGNATURE, 4) == 0) {
            if (validate_mp_checksum((uint8_t *)ptr)) {
                printf("mp floating pointer found at physical address: 0x%x\n", addr);
                return (void *)addr;
            }
        }
    }

    printf("no mp floating pointer found\n");
    return NULL;
}


void print_mp_stats()
{
    struct mp_floating_pointer_structure* mp_pointer = (struct mp_floating_pointer_structure*) MP_FLOATING_POINTER_ADDRESS;
    printf("length: %d\n features: %x\n", mp_pointer->length, mp_pointer->features);
    struct mp_configuration_table* table = (struct mp_configuration_table*) MP_CONFIGURATION_TABLE_ADDRESS;
    printf("entry count: %d\nlocal APIC: %x\n", table->entry_count, table->lapic_address);
    if(mp_pointer->features & (1 << 7))
    {
        printf("APIC is enabled");
    }else{
        printf("PIC still active :(");
    }

}