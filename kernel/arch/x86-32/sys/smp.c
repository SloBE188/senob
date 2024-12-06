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


extern struct idtr_t idtr;
extern struct gdt_ptr_struct gdt_ptr;
extern uint32_t kernel_directory[1024];


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
    ncpus = 0;
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
            printf("Unbekannter Eintragstyp: %u\n", type);
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

void print_mp_stats(uint32_t *floating_pointer_addr, uint32_t *mp_config_table_addr)
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

    mp_init(table);
    prepare_trampoline_code();
}

void disable_pic(void)
{
    outb(0x20+1, 0xFF);     //master pic
    outb(0xA0+1, 0xFF);     //slave pic
}

struct trampoline_data {
    struct idtr_t idtr;
    struct gdt_ptr_struct gdt_ptr;
    uint32_t kernel_directory[1024];
};

extern void _startcpu();

//void* trampolinecodeaddr, uint64_t size
void prepare_trampoline_code()
{
    struct trampoline_data* data = (struct trampoline_data*)0x5000;

    memcpy(&data->idtr, &idtr, sizeof(struct idtr_t));
    memcpy(&data->gdt_ptr, &gdt_ptr, sizeof(struct gdt_ptr_struct));
    memcpy(data->kernel_directory, kernel_directory, sizeof(kernel_directory));

    memset((void*)0x7000, 0x00, 512);
    memcpy((void*)0x7000, _startcpu, 512);
}