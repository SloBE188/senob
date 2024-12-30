/*
 * Copyright (C) 2024 Nils Burkard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "smp.h"
#include "../../../libk/stdiok.h"
#include "../../../libk/memory.h"

#define MP_FLOATING_POINTER_ADDRESS 0xF5BA0
#define MP_CONFIGURATION_TABLE_ADDRESS 0xF5BB0
#define LOCAL_APIC 0xfee00000
#define MP_FLOATING_POINTER_SIGNATURE "_MP_"

struct cpu cpus[MAX_CPUS];
uint32_t ncpus;
uint32_t ioapicid;

/*
extern struct idtr_t idtr;
extern struct gdt_ptr_struct gdt_ptr;
extern uint32_t kernel_directory[1024];
*/


// validates mp checksum
bool validate_mp_checksum(uint8_t *mp)
{
    uint8_t sum = 0;
    for (int i = 0; i < 16; i++)
    {
        sum += mp[i];
    }
    return sum == 0;
}

// searches the mp floating pointer
void *find_mp_floating_pointer(struct multiboot_info *mb_info)
{
    if (!(mb_info->flags & (1 << 6)))
    {
        printf("Memory map not available.\n");
        return NULL;
    }

    uint32_t start_addr = 0x000F0000;
    uint32_t end_addr = 0x000FFFFF;
    for (uint32_t addr = start_addr; addr < end_addr; addr += 16)
    {
        char *ptr = (char *)addr;
        if (memcmp(ptr, MP_FLOATING_POINTER_SIGNATURE, 4) == 0)
        {
            if (validate_mp_checksum((uint8_t *)ptr))
            {
                // printf("mp floating pointer found at physical address: 0x%x\n", addr);
                return (void *)addr;
            }
        }
    }

    printf("no mp floating pointer found\n");
    return NULL;
}

void mp_init(struct mp_configuration_table *table)
{
    uint8_t *entry_ptr = (uint8_t *)table + sizeof(struct mp_configuration_table);
    ncpus  = 0;
    ioapicid = 0;

    for (int i = 0; i < table->entry_count; i++)
    {
        uint8_t type = *entry_ptr;

        switch (type)
        {
        case 0:
        { // processor entry
            struct entry_processor *processor = (struct mp_processor_entry *)entry_ptr;
            if (ncpus < MAX_CPUS)
            {
                cpus[ncpus].id = processor->local_apic_id;
                cpus[ncpus].isbsp = processor->flags & 0x02;
                cpus[ncpus].lapic_version = processor->local_apic_version;
                printf("CPU %d: LAPIC Version=%d, Flags=%d (%s)\n",
                       cpus[ncpus].id,
                       cpus[ncpus].lapic_version,
                       cpus[ncpus].lapic_flags,
                       cpus[ncpus].isbsp ? "BSP" : "AP");

                ncpus++;
            }

            if (processor->flags & 0x02)
                printf("AP is active\n");
            else
            {
                printf("processor is disabled\n");
            }

            // printf("Processor %d\n", i);

            entry_ptr += 20; // processor entries are 20 bytes long
            break;
        }
        case 1:
        { // I/O-APIC entry
            struct entry_io_apic *io_apic = (struct mp_io_apic_entry *)entry_ptr;
            ioapicid = io_apic->id;
            printf("I/O-APIC: ID=%u, Address=0x%x\n", io_apic->id, io_apic->address);

            entry_ptr += 8; // I/O-APIC entries are 8 bytes long
            break;
        }
        default:
            printf("unknown: %u\n", type);
            entry_ptr += 8;
            break;
        }
    }
}

struct addr *smp_addresses(struct multiboot_info *mb_info)
{
    struct mp_floating_pointer_structure *mp_pointer = find_mp_floating_pointer(mb_info);
    struct mp_configuration_table *table = (struct mp_configuration_table *)mp_pointer->configuration_table;

    static struct addr addr;
    addr.floating_ptr_addr = (uint32_t *)mp_pointer;
    addr.mp_config_table_addr = mp_pointer->configuration_table;
    addr.local_apic = table->lapic_address;

    return &addr;
}

void prepare_trampoline_code();

void init_smp(uint32_t *floating_pointer_addr, uint32_t *mp_config_table_addr)
{
    struct mp_floating_pointer_structure *mp_pointer = (struct mp_floating_pointer_structure *)floating_pointer_addr;
    printf("length: %d\n features: %x\n", mp_pointer->length, mp_pointer->features);
    struct mp_configuration_table *table = (struct mp_configuration_table *)mp_config_table_addr;
    printf("entry count: %d\nlocal APIC: %x\n", table->entry_count, table->lapic_address);
    if (mp_pointer->features & (1 << 7))
    {
        printf("APIC is enabled");
    }
    else
    {
        printf("PIC still active :(\n");
    }
    //disable_pic();
    mp_init(table);
    prepare_trampoline_code();


    for (uint32_t i = 1; i < ncpus; i++)
    {
        ap_startup(i, 0x7000);
    }

}

void disable_pic(void)
{
    outb(0x20+1, 0xFF);     //master pic
    outb(0xA0+1, 0xFF);     //slave pic
}


extern uint8_t _trampoline_start[];
extern uint8_t _trampoline_end[];

extern uint8_t _pmmc_start[];
extern uint8_t _pmmc_end[];


void prepare_trampoline_code()
{

    uint32_t size = _trampoline_end - _trampoline_start;
    memcpy((void*)0x7000, _trampoline_start, size);

    uint32_t sizepmm = _pmmc_end - _pmmc_start;
    memcpy((void*)0x8000, _pmmc_start, sizepmm);

}

uint32_t get_local_apic_id_cpuid(void) 
{
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    // APIC-ID is in the bits 31:24 from ebx
    return (ebx >> 24) & 0xFF;
}

struct cpu* curr_cpu()
{
    uint32_t apic_id = get_local_apic_id_cpuid();
    struct cpu* cpu = &cpus[apic_id];

    return cpu;
}
