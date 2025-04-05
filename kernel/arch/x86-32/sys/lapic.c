#include "lapic.h"
#include <stdio.h>
#include <stdint.h>
#include "../mm/paging/paging.h"

#define ID (0x0020 / 4)    // Local APIC ID Reg
#define VER (0x0030 / 4)   // Local APIC version Reg
#define TPR (0x0080 / 4)   // Task priority Reg
#define EOI (0x00B0 / 4)   // EOI Reg
#define SVR (0x00F0 / 4)   // Superior Interrupt Vector Reg
#define ENABLE 0x00000100  // Unit Enable
#define ESR (0x0280 / 4)   // Error Status
#define ICRLO (0x0300 / 4) // Interrupt Command
#define INIT 0x00000500    // INIT/RESET
#define STARTUP 0x00000600 // Startup IPI
#define DELIVS 0x00001000  // Delivery status
#define ASSERT 0x00004000  // Assert interrupt (vs deassert)
#define DEASSERT 0x00000000
#define LEVEL 0x00008000 // Level triggered
#define BCAST 0x00080000 // Send to all APICs, including self.
#define BUSY 0x00001000
#define FIXED 0x00000000
#define ICR1 (0x0300 / 4)   // Interrupt Command Reg 0-31
#define ICR2 (0x0310 / 4)   // Interrupt Command Reg [32-63]
#define TIMER (0x0320 / 4)  // Local Vector Table 0 (TIMER)
#define X1 0x0000000B       // divide counts by 1
#define PERIODIC 0x00020000 // Periodic
#define PCINT (0x0340 / 4)  // Performance Counter LVT
#define LINT0 (0x0350 / 4)  // Local Vector Table 1 (LINT0)
#define LINT1 (0x0360 / 4)  // Local Vector Table 2 (LINT1)
#define ERROR (0x0370 / 4)  // Local Vector Table 3 (ERROR)
#define MASKED 0x00010000   // Interrupt masked
#define TICR (0x0380 / 4)   // Timer Initial Count
#define TCCR (0x0390 / 4)   // Timer Current Count
#define TDCR (0x03E0 / 4)   // Timer Divide Configuration

//LAPIC address
volatile uint32_t *IA32_APIC_BASE = (volatile uint32_t *)0xFEE00000;

//Write function for the LAPIC.
//@offset = offset for specifying the LAPIC-register. Final address = 0xFEE00000 + offset.
uint32_t lapicRead(int offset)
{
    return IA32_APIC_BASE[offset];
}

//Read function for the LAPIC.
//@offset = offset for specifying the LAPIC-register. Final address = 0xFEE00000 + offset.
//@value = value to write into the LAPIC-register.
void lapicWrite(uint32_t offset, uint32_t value)
{
    volatile uint32_t* reg = (volatile uint32_t*)(IA32_APIC_BASE + offset);
    *reg = value;
    (void)*(volatile uint32_t*)(IA32_APIC_BASE + ID); // write buffer flush
}


//Sends EOI to lapic.
void eoi() 
{
    lapicWrite(EOI, 0x00);
}


//Maps LocalAPIC into memory (virt 0xFEE00000).
void mapLAPIC()
{
    mem_map_page(0xFEE00000, 0xFEE00000, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
}


//Initializes the APIC-Timer.
void lapicTimerInit()
{
    lapicWrite(TDCR, 0x3);
    lapicWrite(TIMER, PERIODIC | 0x40);
    lapicWrite(TICR, 1000000);      //will be set to 62500 (100Hz)
}

//Initializes BSPs local APIC.
void lapicInit()
{
    mapLAPIC();

    //Enable the APIC threw the SVR Reg
    lapicWrite(SVR, ENABLE | 0xFF);
    uint32_t svr = lapicRead(SVR);
    printf("SVR: 0x%x\n", svr);

    uint32_t sivr = IA32_APIC_BASE[SVR];
    if (lapicRead(SVR) & ENABLE)
    {
        printf("APIC activated\n");
    }
    else
    {
        printf("APIC not activated :(\n");
    }

    lapicWrite(TPR, 0x00);  //Sets TPR to 0x00 (accept all interrupts)
    lapicWrite(LINT0, 0x700);   //0x700 = ExtInt for PIC
    lapicWrite(LINT1, MASKED);    //Maks it, dont need it

    //Clear ESR reg, have to do it twice
    lapicWrite(ESR, 0x00);
    lapicWrite(ESR, 0x00);

    lapicWrite(EOI, 0x00);  //Set EOI to 0x00, would probably not be needed here
}


//Sends IPI to the target CPU.
//@apicID = APIC-ID from the target AP.
void lapicSendIPI(uint32_t apic_id)
{
    lapicWrite(ICR2, apic_id << 24);

    lapicWrite(ICR1, INIT | LEVEL | ASSERT);    //assert
    while (lapicRead(ICR1) & DELIVS)
        ;    

    lapicWrite(ICR2, apic_id << 24);
    lapicWrite(ICR1, INIT | LEVEL); //Deassert 
    while (lapicRead(ICR1) & DELIVS)
        ;
}

//Sends SIPI to the target CPU.
//@apicID = APIC-ID from the target AP.
//@trampolineAddr = address of the trampoline code (code which first gets executed by the CPU after she wakes up). 
void lapicSendSIPI(uint32_t apicID, uint32_t trampolineAddr)
{
    lapicWrite(ICR2, apicID << 24);
    lapicWrite(ICR1, trampolineAddr >> 12 | STARTUP | ASSERT);

    while (lapicRead(ICR1) & DELIVS)
        ;
}

//Starts a new CPU (AP).
//@apicID = APIC-ID from the AP which should get started.
//@trampolineAddr = address of the trampoline code (code which first gets executed by the CPU after she wakes up). 
void apStartup(uint32_t apicID, uint32_t trampolineAddr)
{
    printf("Starting ap startup for CPU %d\n", apicID);

    lapicSendIPI(apicID);
    PitWait(10);

    for (int i = 0; i < 2; i++)
    {
        lapicSendSIPI(apicID, trampolineAddr);
        PitWait(1);
    }

    printf("CPU %d started!\n", apicID);
}
