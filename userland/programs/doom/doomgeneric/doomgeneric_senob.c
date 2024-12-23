#include <stdint.h>
#include "../../kernel/libk/stdiok.h"
#include "../../kernel/libk/memory.h"
#include "../../kernel/arch/x86-32/interrupts/pit.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/video/vbe/vbe.h"
#include "doomgeneric.h"

extern uint64_t ticks;

extern struct vbe_info* globalvbeinfo;


void DG_Init() 
{

    if (globalvbeinfo == NULL) {
        while (1){}
    }


    DG_ScreenBuffer = (uint8_t *)kmalloc(globalvbeinfo->framebuffer_width * globalvbeinfo->framebuffer_height * 4);
    if (!DG_ScreenBuffer) {
        while (1) {}
    }


    clear_screen(COLOR_BLACK, globalvbeinfo);
}


int DG_GetKey() 
{
    uint32_t key = get_key_from_buffer();
    if (key == 0 || key > 0x7F) {
        return 0;
    }
    return (int)key;    //return ASCII value
}



void DG_DrawFrame() 
{
    uint32_t *framebuffer = (uint32_t *)0xE0000000;
    uint32_t *screenBuffer = (uint32_t *)DG_ScreenBuffer;

    memcpy(framebuffer, screenBuffer, globalvbeinfo->framebuffer_width * globalvbeinfo->framebuffer_height * 4);
}



uint32_t DG_GetTicksMs() 
{
    return ticks;
}

int main(int argc, char** argv)
{
    
    doomgeneric_Create(argc, argv);

    for (int i = 0; ; i++)
    {
        doomgeneric_Tick();
    }

}