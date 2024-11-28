#ifndef SMP_H
#define SMP_H


#include <stdint.h>
#include "../multiboot.h"
#include "../kernel.h"

struct mp_floating_pointer_structure {
    char signature[4];
    uint32_t configuration_table;
    uint8_t length; // In 16 bytes (e.g. 1 = 16 bytes, 2 = 32 bytes)
    uint8_t mp_specification_revision;
    uint8_t checksum; // This value should make all bytes in the table equal 0 when added together
    uint8_t default_configuration; // If this is not zero then configuration_table should be 
                                   // ignored and a default configuration should be loaded instead
    uint32_t features; // If bit 7 is then the IMCR is present and PIC mode is being used, otherwise 
                       // virtual wire mode is; all other bits are reserved
};

struct mp_configuration_table {
    char signature[4]; // "PCMP"
    uint16_t length;
    uint8_t mp_specification_revision;
    uint8_t checksum; // Again, the byte should be all bytes in the table add up to 0
    char oem_id[8];
    char product_id[12];
    uint32_t oem_table;
    uint16_t oem_table_size;
    uint16_t entry_count; // This value represents how many entries are following this table
    uint32_t lapic_address; // This is the memory mapped address of the local APICs 
    uint16_t extended_table_length;
    uint8_t extended_table_checksum;
    uint8_t reserved;
};

struct entry_processor {
    uint8_t type; // Always 0
    uint8_t local_apic_id;
    uint8_t local_apic_version;
    uint8_t flags; // If bit 0 is clear then the processor must be ignored
                   // If bit 1 is set then the processor is the bootstrap processor
    uint32_t signature;
    uint32_t feature_flags;
    uint64_t reserved;
};

struct entry_io_apic {
    uint8_t type; // Always 2
    uint8_t id;
    uint8_t version;
    uint8_t flags; // If bit 0 is set then the entry should be ignored
    uint32_t address; // The memory mapped address of the IO APIC is memory
};

void print_mp_stats();
void* find_mp_floating_pointer(struct multiboot_info *mb_info);

#endif