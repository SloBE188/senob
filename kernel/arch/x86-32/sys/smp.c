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
                //printf("mp floating pointer found at physical address: 0x%x\n", addr);
                return (void *)addr;
            }
        }
    }

    printf("no mp floating pointer found\n");
    return NULL;
}

void parse_mp_entries(struct mp_configuration_table* table) {
    uint8_t* entry_ptr = (uint8_t*) table + sizeof(struct mp_configuration_table);

    for (int i = 0; i < table->entry_count; i++) {
        uint8_t type = *entry_ptr;

        switch (type) {
            case 0: {  // processor entry
                struct entry_processor* processor = (struct mp_processor_entry*) entry_ptr;
                printf("Processor: LAPIC ID=%u, LAPIC Version=%u, Flags=%u\n",
                       processor->local_apic_id, processor->local_apic_version, processor->flags);
                
                if(processor->flags == 0x02)
                    printf("AP is active\n");
                else
                {
                    printf("processor is disabled\n");
                }
                

                entry_ptr += 20;short  // processor entries are 20 bytes long
                break;
            }
            case 1: {  // I/O-APIC entry
                struct entry_io_apic* io_apic = (struct mp_io_apic_entry*) entry_ptr;
                printf("I/O-APIC: ID=%u, Address=0x%x\n", io_apic->id, io_apic->address);

                entry_ptr += 8;  // I/O-APIC entries are 8 bytes long
                break;
            }
            default:
                printf("Unbekannter Eintragstyp: %u\n", type);
                entry_ptr += 8;
                break;
        }
    }
}


struct addr* smp_addresses(struct multiboot_info *mb_info) 
{
    struct mp_floating_pointer_structure* mp_pointer = find_mp_floating_pointer(mb_info);
    
    struct addr* addr;
    addr->floating_ptr_addr = (uint32_t*)find_mp_floating_pointer(mb_info);
    addr->mp_config_table_addr = mp_pointer->configuration_table;

    return addr;
}


void print_mp_stats(uint32_t* floating_pointer_addr, uint32_t* mp_config_table_addr)
{
    struct mp_floating_pointer_structure* mp_pointer = (struct mp_floating_pointer_structure*) floating_pointer_addr;
    printf("length: %d\n features: %x\n", mp_pointer->length, mp_pointer->features);
    struct mp_configuration_table* table = (struct mp_configuration_table*) mp_config_table_addr;
    printf("entry count: %d\nlocal APIC: %x\n", table->entry_count, table->lapic_address);
    if(mp_pointer->features & (1 << 7))
    {
        printf("APIC is enabled");
    }else{
        printf("PIC still active :(\n");
    }

    parse_mp_entries(table);

}

