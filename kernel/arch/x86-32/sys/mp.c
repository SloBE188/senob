#include "mp.h"
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

struct cpu cpus[MAX_CPUS];

uint32_t ncpus;
uint32_t ioapicid;


//validates mp checksum.
//@mp = pointer to the checksum.
bool validateMPChecksum(uint8_t *mp)
{
    uint8_t sum = 0;
    for (int i = 0; i < 16; i++)
    {
        sum += mp[i];
    }
    return sum == 0;
}

//Searches the mp floating pointer.
void *find_mp_floating_pointer()
{
    uint32_t startAddr = 0x000A0000;
    uint32_t endAddr = 0x000FFFFF;
    for (uint32_t addr = startAddr; addr < endAddr; addr += 16)
    {
        char *ptr = (char *)addr;
        if (memcmp(ptr, MP_FLOATING_POINTER_SIGNATURE, 4) == 0)
        {
            if (validateMPChecksum((uint8_t *)ptr))
            {
                printf("MP floating pointer found at physical address: 0x%x\n", addr);
                return (void *)addr;
            }
        }
    }

    printf("No mp floating pointer found\n");
    return NULL;
}


//Iterates threw the table and saves the found CPUs in the CPU array.
//@table = MP Configuration Table found in the Floating Pointer Structure in the mpInit function.
void mpSearch(struct mpConfigurationTable* table)
{
    uint8_t *entryPtr = (uint8_t *)table + sizeof(struct mpConfigurationTable);
    ncpus  = 0;
    ioapicid = 0;

    for (int i = 0; i < table->entryCount; i++)
    {
        uint8_t type = *entryPtr;

        switch (type)
        {
        case 0:
        { //Processor entry
            struct entryProcessor *processor = (struct entryProcessor *)entryPtr;
            if (ncpus < MAX_CPUS)
            {
                cpus[ncpus].id = processor->localAPICID;
                cpus[ncpus].isBSP = processor->flags & 0x02;
                cpus[ncpus].lapicVersion = processor->localAPICVersion;
                kernel_write("CPU %d: LAPIC Version=%d, Flags=%d (%s)\n",
                       cpus[ncpus].id,
                       cpus[ncpus].lapicVersion,
                       cpus[ncpus].lapicFlags,
                       cpus[ncpus].isBSP ? "BSP" : "AP");

                ncpus++;
            }

            if (processor->flags & 0x02)
                kernel_write("AP is active\n");
            else
            {
                kernel_write("Processor is disabled\n");
            }


            entryPtr += 20; //Processor entries are 20 bytes long
            break;
        }
        case 1:
        { //IO-APIC entry
            struct entryIOAPIC *IOAPIC = (struct entryIOAPIC *)entryPtr;
            IOAPIC = IOAPIC->id;
            printf("I/O-APIC: ID=%u, Address=0x%x\n", IOAPIC->id, IOAPIC->address);

            entryPtr += 8; //IO-APIC entries are 8 bytes long
            break;
        }

        //Not a processor or IO-APIC entry
        default:
            printf("unknown: %u\n", type);
            entryPtr += 8;
            break;
        }
    }
}

//Initializes MP tables.
void mpInit()
{
    struct mpFloatingPointerStructure* fpStructure = find_mp_floating_pointer();
    struct mpConfigurationTable* table = fpStructure->configurationTable;

    if (fpStructure->features & (1 << 7))
    {
        printf("IO-APIC enabled\n");
    }else
    {
        printf("PIC still active\n");
    }   
    

    mpSearch(table);


}