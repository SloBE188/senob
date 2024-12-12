#include <stdint.h>
#include "../multiboot.h"
#include "../kernel.h"
#include "../../../libk/stdiok.h"
#include "smp.h"
#include "apic.h"
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include "../io/io.h"
#include "../interrupts/pit.h"

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

volatile uint32_t *IA32_APIC_BASE = (volatile uint32_t *)0xFEE00000;

uint32_t lapic_read(int offset)
{
    return IA32_APIC_BASE[offset];
}

void lapicw(int offset, int value)
{
    IA32_APIC_BASE[offset] = value;
    // Didnt know that ;) modern CPUs use a writebuffer for optimization, so the value gets into the hardware a bit later.
    // if i now read from the memory, the CPU makes sure & forces, that the command before got executed
    IA32_APIC_BASE[ID];
}

void map_lapic()
{
    mem_map_page(0xFEE00000, 0xFEE00000, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
}

void lapic_timer_init()
{
    lapicw(TDCR, 0x3);
    lapicw(TICR, 0x1000);
    lapicw(TIMER, 0x20); // IRQ0 is 32 and the Timer IRQ is 0
}

void sync_arbitration_ids()
{
    lapicw(ICR1, 0);                    // All CPUs
    lapicw(ICR2, BCAST | INIT | LEVEL); // level triggered INIT IPI
    while (lapic_read(ICR1) & DELIVS)
        ;
    lapicw(ICRLO, BCAST | INIT | LEVEL | DEASSERT); // send deassert
    while (lapic_read(ICRLO) & DELIVS)
        ;
}

#define PIT_SCALE 1193180
#define PIT_CHANNEL_1_PORT 0x40
#define PIT_COMMAND 0x43

void lapic_init(void)
{
    init_pit(1000);
    map_lapic();

    // Enable the APIC threw the SVR Reg
    lapicw(SVR, ENABLE | 0xFF);
    uint32_t svr = lapic_read(SVR);
    printf("SVR: 0x%x\n", svr);

    uint32_t sivr = IA32_APIC_BASE[SVR];
    if (lapic_read(SVR) & ENABLE)
    {
        printf("APIC ist aktiviert!\n");
    }
    else
    {
        printf("APIC not activated :(\n");
    }

    // Set TPR to 0x00 -> accept all interrupts (without setting this, the APIC could block every interrupt)
    lapicw(TPR, 0x00);

    lapic_timer_init();

    uint32_t timer_div = lapic_read(TDCR);
    uint32_t timer_init = lapic_read(TICR);
    uint32_t lvt_timer = lapic_read(TIMER);

    printf("APIC Timer Divider: 0x%x\n", timer_div);
    printf("APIC Timer Initial Count: 0x%x\n", timer_init);
    printf("APIC LVT Timer: 0x%x\n", lvt_timer);

    // disable LINT0, LINT1 & ERROR
    lapicw(LINT0, MASKED);
    lapicw(LINT1, MASKED);
    lapicw(ERROR, MASKED);
    // lapicw(TIMER, MASKED);

    // clear error status reg, no idea why you have to do it twice
    lapicw(ESR, 0);
    lapicw(ESR, 0);

    lapicw(EOI, 0x00);

    lapicw(ESR, 0); // Clear ESR
    uint32_t esr = lapic_read(ESR);
    printf("LAPIC ESR: 0x%x\n", esr);

    // sync_arbitration_ids();
}

void configure_cmos_and_reset_vector(uint32_t trampoline_addr)
{
    outb(0x70, 0x0F);
    outb(0x71, 0x0A);

    // set warm reset vector
    uint16_t *warm_reset_vector = (uint16_t *)(0x40 << 4 | 0x67);
    warm_reset_vector[0] = trampoline_addr & 0xF;           // offset (low 16 bits)
    warm_reset_vector[1] = (trampoline_addr >> 4) & 0xFFFF; // segment (high 16 bits)
    printf("warm reset vector set: segment=0x%x, offset=0x%x\n",
           warm_reset_vector[1], warm_reset_vector[0]);
}

void delay(uint32_t microseconds)
{
    volatile uint32_t count = microseconds * 100; // Anpassung je nach CPU-Geschwindigkeit
    while (count--)
    {
        __asm__ volatile("nop");
    }
}

void ap_startup(uint32_t APIC_ID, uint32_t trampoline_addr)
{
    printf("starting ap startuf for CPU %d\n", APIC_ID);
    // configure_cmos_and_reset_vector(trampoline_addr);

    // send INIT IPI
    printf("sending INIT IPI\n");
    lapicw(ICR1, APIC_ID << 24);
    lapicw(ICR2, INIT | LEVEL | ASSERT);
    while (lapic_read(ICRLO) & DELIVS)
    {
        printf("waiting for INIT DELIVS to clear\n");
    }
    printf("INIT DELIVS Bit got deleted.\n");

    delay(1000);
    lapicw(ICR2, INIT | LEVEL);
    while (lapic_read(ICRLO) & DELIVS)
    {
        printf("waiting for DEASSERT DELIVS to clear\n");
    }
    printf("DEASSERT DELIVS Bit got deleted.\n");

    delay(1000);

    // send 2 SIPIs
    for (int i = 0; i < 2; i++)
    {
        lapicw(ICR1, APIC_ID << 24);
        lapicw(ICR2, STARTUP | (trampoline_addr >> 12));
        printf("SIPI #%d sent. trampoline address: 0x%x\n", i + 1, trampoline_addr >> 12);
        while (lapic_read(ICRLO) & DELIVS)
        {
            printf("waiting for SIPI DELIVS to clear\n");
        }
        printf("SIPI #%d DELIVS-Bit got deleted.\n", i + 1);
        delay(1000);
    }
}

void test_apic_timer()
{
    for (volatile int i = 0; i < 100000; i++)
        ; // Kurze Verzögerung

    uint32_t timer_curr = lapic_read(TCCR); // Aktueller Timer-Wert
    printf("APIC Timer Current Count: 0x%x\n", timer_curr);

    uint32_t siv = lapic_read(SVR);
    printf("SIV Register: 0x%x\n", siv);
    if (!(siv & 0x100))
    {
        printf("if APIC isnt active, activate");
        lapicw(SVR, siv | 0x100); // Aktiviert APIC
    }
}



#define APIC_BASE_MSR 0x1B
#define APIC_BASE_MASK 0xFFFFF000

//readmsr function
static inline void read_msr(uint32_t msr, uint32_t *low, uint32_t *high) 
{
    asm volatile (
        "rdmsr"
        : "=a"(*low), "=d"(*high)
        : "c"(msr)
    );
}


void check_apic_base_address() 
{
    uint32_t low, high;

    read_msr(APIC_BASE_MSR, &low, &high);

    // extract baseaddress from the msr data
    uint64_t apic_base = ((uint64_t)high << 32) | low;

    apic_base &= APIC_BASE_MASK;

    int apic_enabled = low & (1 << 11);

    printf("APIC Base Address: 0x%llx\n", apic_base);
    if (apic_enabled) {
        printf("APIC is active\n");
    } else {
        printf("APIC isnt active\n");
    }
}