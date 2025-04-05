#ifndef MP_H
#define MP_H

#include <stdint.h>

#define MAX_CPUS 8
#define MP_FLOATING_POINTER_SIGNATURE "_MP_"

struct mpFloatingPointerStructure 
{
    char signature[4];  //_MP_
    uint32_t configurationTable;   //Physical address of the configuration table
    uint8_t length;
    uint8_t mpSpecificationRevision;
    uint8_t checksum;   //Is a check entry, should all add up to 0 (module 256 = 0), makes sure the table is valid
    uint8_t defaultConfiguration; //If this is not zero then configuration_table should be 
                                   //ignored (smth is wrong, maybe the system doesnt support mp tables or there is a fault)
    uint32_t features; // If bit 7 is set then the IMCR is present and that means that PIC mode is being used
};


struct mpConfigurationTable 
{
    char signature[4]; //"PCMP"
    uint16_t length;
    uint8_t mpSpecificationRevision;
    uint8_t checksum;   //Is a check entry, should all add up to 0 (module 256 = 0), makes sure the table is valid
    char oemID[8];
    char productID[12];
    uint32_t oemTable;
    uint16_t oemTableSize;
    uint16_t entryCount;   //Represents the amount of entries in the table
    uint32_t lapicAddress; //Address of the local apic (BSP)
    uint16_t extendedTableLength;
    uint8_t extendedTableChecksum;
    uint8_t reserved;
};


struct entryProcessor 
{
    uint8_t type;   //0
    uint8_t localAPICID;  //APIC-ID
    uint8_t localAPICVersion;
    uint8_t flags;  //If bit 0 is clear, this cpu must be ignored
                    //If bit 1 is set = BSP
    uint32_t signature;
    uint32_t featureFlags;
    uint64_t reserved;
};

struct entryIOAPIC
{
    uint8_t type;   //2
    uint8_t id;
    uint8_t version;
    uint8_t flags; //If bit 0 is set, entry has to be ignored
    uint32_t address;   //Address of the IO-APIC
};

struct cpu
{
    uint32_t id;        //ID from the cpu (lapic id)
    uint32_t lapicBase;    //for the bsp normally 0xfee00000
    uint32_t lapicVersion;
    uint32_t lapicFlags;   //to check if its the bsp or not
    struct process* proc;
    uint32_t isBSP;         //1 = yes, 0 = no
    char cpuName;          //CPU 0... for the user
};

extern uint32_t ncpus;

void mpInit();


#endif