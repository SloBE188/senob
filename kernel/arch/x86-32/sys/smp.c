#include "smp.h"
#include "mp.h"
#include "lapic.h"
#include "../io/io.h"
#include "../kernel.h"


extern uint32_t ncpus;


extern uint8_t _trampoline_start[];
extern uint8_t _trampoline_end[];

extern uint8_t _pMode_start[];
extern uint8_t _pMode_end[];

//Prepares the trampoline code for the starting APs
void prepareTrampolineCode()
{

    uint32_t size = _trampoline_end - _trampoline_start;
    memcpy((void*)0x7000, _trampoline_start, size);

    uint32_t sizepmm = _pMode_end - _pMode_start;
    memcpy((void*)0x8000, _pMode_start, sizepmm);

}

//A function which fully enables SMP
void smpInit()
{
    mpInit();
    lapicInit();
    prepareTrampolineCode();
    
    for (uint32_t i = 1; i < ncpus; i++)
    {
        if(i > 8)
            return;
        apStartup(i, 0x7000);
        PitWait(50);
    }

    /*PitWait(50);
    outb(0x21, 0xFD);       //only 0xFC so the irq0 and irq1 still work over the pic
    outb(0xA1, 0xFF);*/
    //lapicTimerInit();
}