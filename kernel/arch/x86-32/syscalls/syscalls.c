#include "syscalls.h"
#include "../interrupts/idt.h"
#include "../../../../drivers/video/vbe/vbe.h"
#include "../../../libk/libk.h"
#include "../fatfs/ff.h"
#include <sys/types.h>
#include "../../../../drivers/keyboard/keyboard.h"
#include "../kernel.h"
#include "playpal.h"
#include "../sys/process.h"

#define FRAMEBUFFER_ADDR ((volatile uint8_t *)0xE0000000)

extern volatile uint64_t pit_ticks;

uint32_t syscalls_amount = 0;

syscalls_fun_ptr syscall_functions[1024] = {NULL};

void init_syscalls()
{
    register_syscalls();
}

void add_syscalls(uint32_t syscal_number, syscalls_fun_ptr sys_function)
{
    syscall_functions[syscal_number] = sys_function;
    syscalls_amount++;
    
}

void syscall_0_print(struct Interrupt_registers *regs)
{

    char *user_string = regs->ebx;
    kernel_write("%s", user_string);
}

/*void syscall_1_load_process(struct Interrupt_registers *regs)
{
    char *user_program_name = regs->ebx;
    struct process *new_program = create_process(user_program_name);
    switch_task(new_program->head_thread->regs);
}*/

void syscall_2_clear_screen(struct Interrupt_registers *regs)
{
    uint32_t color = regs->ebx;
    clear_screen_sys_2(color);
    regs->eax = 1;
}

void syscall_3_write(struct Interrupt_registers *regs)
{
    int fd = (int)regs->ebx;
    const void *buf = (const void *)regs->ecx;
    size_t len = (size_t)regs->edx;
    regs->eax = write(fd, buf, len);
}

static void *heap_end = 0x00800000;
static void *heap_limit = 0x01800000;
#define ALIGN_UP(x, align) (((x) + (align - 1)) & ~(align - 1))

void syscall_4_sbrk(struct Interrupt_registers *regs)
{

    int increment = regs->ebx;

    void *old_heap_end = heap_end;
    void *new_heap_end = (void *)ALIGN_UP((uint32_t)old_heap_end + increment, 0x1000);

    if (new_heap_end > heap_limit)
    {
        regs->eax = (uint32_t)-1;
        return;
    }

    heap_end = new_heap_end;
    regs->eax = (uint32_t)old_heap_end;
}

void syscall_5_open(struct Interrupt_registers *regs)
{
    const char *name = (const char *)regs->ebx;
    int flags = (int)regs->ecx;

    int fd = open(name, flags);

    regs->eax = (uint32_t)fd;
}

void syscall_6_readdir(struct Interrupt_registers *regs)
{
    const char *path = (const char *)regs->ebx;

    DIR dir;
    FILINFO fno;
    uint32_t fresult;

    if (f_opendir(&dir, path) == FR_OK)
    {
        while (fresult = f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0)
        {
            kernel_write("Gefunden: %s\n", fno.fname);
        }
        f_closedir(&dir);
    }
    regs->eax = fresult;
}

void syscall_7_close(struct Interrupt_registers *regs)
{
    int file = regs->ebx;

    int res = close(file);

    regs->eax = res;
}

void syscall_8_read(struct Interrupt_registers *regs)
{
    int file = regs->ebx;
    void *buf = (void *)regs->ecx;
    size_t len = (size_t)regs->edx;

    // printf("syscall_8_read: file=%d, buf=%p, len=%u\n", file, buf, len);

    int br = read(file, buf, len);

    // printf(" -> read returned br=%d\n", br);

    regs->eax = br;
}

void syscall_9_lseek(struct Interrupt_registers *regs)
{
    int file = regs->ebx;
    off_t offset = (off_t)regs->ecx;
    int whence = regs->edx;

    off_t new_pos = lseek(file, offset, whence);
    regs->eax = (off_t)new_pos;
}

void syscall_10_stat(struct Interrupt_registers *regs)
{
    const char *path = (const char *)regs->ebx;
    struct stat *st = (struct stat *)regs->ecx;

    int res = stat(path, st);

    regs->eax = res;
}

void syscall_11_fstat(struct Interrupt_registers *regs)
{
    int file = regs->ebx;
    struct stat *stat = (struct stat *)regs->ecx;
    int res = fstat(file, stat);
    regs->eax = res;
}

void syscall_12_mkdir(struct Interrupt_registers *regs)
{
    const char *path = (const char *)regs->ebx;
    mode_t mode = (mode_t)regs->ecx;

    int res = mkdir(path, mode);
    regs->eax = res;
}

void syscall_13_get_key_from_buffer(struct Interrupt_registers *regs)
{
    uint32_t key = get_key_from_buffer();
    regs->eax = key;
}

void syscall_14_draw_frame_doom(struct Interrupt_registers *regs)
{
    const uint8_t *buffer = (const uint8_t *)regs->ebx;
    int pitch = regs->ecx; // should be 320
    int doomWidth = 320;
    int doomHeight = 200;

    int scaleX = 1024 / 320;                         // 1024/320 = 3
    int scaleY = 768 / 200;                          // 768/200  = 3
    int scale = (scaleX < scaleY) ? scaleX : scaleY; // 3

    memcpy(doomPalette, _acPLAYPAL, 768);

    for (int y = 0; y < doomHeight; y++)
    {
        for (int x = 0; x < doomWidth; x++)
        {
            int doom_index = y * pitch + x;
            int fb_index = (y * 1024 + x) * 4;
            uint8_t index = buffer[doom_index];
            uint8_t r = doomPalette[index * 3 + 0];
            uint8_t g = doomPalette[index * 3 + 1];
            uint8_t b = doomPalette[index * 3 + 2];

            // scale up to 1024×768-32Bit
            for (int sy = 0; sy < scale; sy++)
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    int fbX = x * scale + sx;
                    int fbY = y * scale + sy;
                    int fb_index = (fbY * 1024 + fbX) * 4;
                    // BGRA
                    FRAMEBUFFER_ADDR[fb_index + 0] = b;
                    FRAMEBUFFER_ADDR[fb_index + 1] = g;
                    FRAMEBUFFER_ADDR[fb_index + 2] = r;
                    FRAMEBUFFER_ADDR[fb_index + 3] = 255;
                }
            }
        }
    }
}
void syscall_15_get_ticks(struct Interrupt_registers* regs)
{
    regs->eax = pit_ticks;
}

void syscall_16_execve(struct Interrupt_registers* regs)
{

    const char* path = (const char*)regs->ebx;
    char** argv = (char**)regs->ecx;
    char** envp = (char**)regs->edx;

    char name [20];
    strcpy(name, path);
    printf("%s\n", name);
    

    uint32_t res = execve(name, argv, envp);

    //this point should never be reached, cu should starts its execution of the new process (thread)
    regs->eax = 1;
}

void syscall_17_getpid(struct Interrupt_registers* regs)
{
    //regs->eax = get_curr_pid();
}



void register_syscalls()
{
        kernel_write("Registering syscalls\n");
        add_syscalls(PRINT_SYSCALL, syscall_0_print);
        //add_syscalls(LOAD_PROCESS_SYSCALL, syscall_1_load_process);
        add_syscalls(CLEAR_SCREEN_SYSCALL, syscall_2_clear_screen);
        add_syscalls(WRITE_SYSCALL, syscall_3_write);
        add_syscalls(SBRK_SYSCALL, syscall_4_sbrk);
        add_syscalls(OPEN_SYSCALL, syscall_5_open);
        add_syscalls(READDIR_SYSCALL, syscall_6_readdir);
        add_syscalls(CLOSE_SYSCALL, syscall_7_close);
        add_syscalls(READ_SYSCALL, syscall_8_read);
        add_syscalls(LSEEK_SYSCALL, syscall_9_lseek);
        add_syscalls(STAT_SYSCALL, syscall_10_stat);
        add_syscalls(FSTAT_SYSCALL, syscall_11_fstat);
        add_syscalls(MKDRI_SYSCALL, syscall_12_mkdir);
        add_syscalls(GKBUFFER_SYSCALL, syscall_13_get_key_from_buffer);
        add_syscalls(DRAW_FRAME_DOOM_SYSCALL, syscall_14_draw_frame_doom);
        add_syscalls(GETTICKS_SYSCALL, syscall_15_get_ticks);
        add_syscalls(EXECVE_SYSCALL, syscall_16_execve);
        add_syscalls(GETPID_SYSCALL, syscall_17_getpid);
        kernel_write("A total of %d Syscalls have been registered\n", syscalls_amount -1);
}