#include <errno.h>
#include <sys/types.h>
#include <stdint.h>


int clear_screen(uint32_t color)
{
    int res;
    __asm__ volatile(
        "int $0x80"
        : "=a"(res)
        : "a"(2), "b"(color)
        : "memory", "cc"
    );

    return res;
}

int write(int fd, const void* buf, size_t len)
{
    int bw;
    __asm__ volatile(
        "int $0x80"
        : "=a"(bw)
        : "a"(3), "b"(fd), "c"(buf), "d"(len)
        : "memory", "cc"
    );

    return bw;
}


void *sbrk(ptrdiff_t increment)
{
    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(4), "b"(increment)
        : "memory", "cc"
    );

    if(ret == -1)
        return (void*) -1;

    return (void*) ret;

}





int open(const char* name, int flags, ...)
{
    int fd;
    __asm__ volatile(
        "int $0x80"
        : "=a"(fd)
        : "a"(5), "b"(name), "c"(flags)
        : "memory", "cc"
    );

    if(fd == -1)
        return -1;

    return fd;

}


int readdir(const char* path)
{
    int fresult;
    __asm__ volatile(
        "int $0x80"
        : "=a"(fresult)
        : "a"(6), "b"(path)
        : "memory", "cc"
    );

    return fresult;
}


int close(int file)
{
    int res;
    __asm__ volatile(
        "int $0x80"
        : "=a"(res)
        : "a"(7), "b"(file)
        : "memory", "cc"
    );
}

/*
 This version does not work because it isnt guaranteed that the compiler will
 put the params in the right registers, so i have to speciofically do that, pretty stupid
int read(int file, void *buf, size_t len)
{
    int br;
    __asm__ volatile(
        "mov $8, %%eax          \n"
        "mov %1, %%ebx          \n" //file
        "mov %2, %%ecx          \n" //buf
        "mov %3, %%edx          \n" //len
        "int $0x80              \n"
        "mov %%eax, %0          \n"
        : "=r" (br)
        : "r" (file), "r" (buf), "r" (len)
    );

    return br;
}
*/

int read(int file, void *buf, size_t len)
{
    int br;
    asm volatile (
        "int $0x80"
        : "=a"(br)
        : "a"(8),
          "b"(file),
          "c"(buf),
          "d"(len)
        : "memory", "cc"
    );
    return br;
}

off_t lseek(int file, off_t offset, int whence)
{
    off_t new_pos;
    __asm__ volatile(
        "int $0x80"
        : "=a"(new_pos)
        : "a"(9), "b"(file), "c"(offset), "d"(whence)
        : "memory", "cc"

    );

    return new_pos;

}



int stat(const char *path, struct stat *st)
{
    int res;
    __asm__ volatile(
        "int $0x80"
        : "=a"(res)
        : "a"(10), "b"(path), "c"(st)
        : "memory", "cc"
    );

    return res;
}

int fstat(int file, struct stat *st) 
{
    int res;
    __asm__ volatile(
        "int $0x80"
        : "=a"(res)
        : "a"(11), "b"(file), "c"(st)
        : "memory", "cc"
    );
    return res;
}

int mkdir(const char *path, mode_t mode)
{
    int res;
    __asm__ volatile(
        "int $0x80"
        : "=a"(res)
        : "a"(12), "b"(path), "c"(mode)
        : "memory", "cc"
    );

    return res;
}

uint32_t get_key_from_buffer() 
{
    uint32_t key;
    __asm__ volatile(
        "int $0x80"
        : "=a"(key)
        : "a"(13)
        : "memory", "cc"
    );

    return key;
}

void draw_frame_doom(const uint8_t* buffer, int pitch) 
{
    __asm__ volatile(
        "int $0x80"
        :
        : "a"(14), "b"(buffer), "c"(pitch)
        : "memory", "cc"
    );
}

uint32_t get_ticks_doom(void) 
{
    uint32_t ticks;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ticks)
        : "a"(15)
        : "memory", "cc"
    );
    return ticks;
}

int execve(const char *name, char** argv, char** envp)
{
    uint32_t res;
    __asm__ volatile(
        "int $0x80"
        : "=a"(res)
        : "a"(16), "b"(name), "c"(argv), "d"(envp)
        : "memory", "cc"
    );

    //this point should never be reached, execution should start in the new process (thread)
    return res;
}

int unlink(const char *name)
{
    return -1;
}


int link(const char *old, const char *new)
{
    return -1;
}
void _fini()
{}


int dup(int fd)
{
    return -1;
}

int dup2(int oldfd, int newfd)
{
    return -1;
}

int getpid()
{
    return -1;
}

int isatty(int file)
{
    return 1;
}

int kill(int pid, int sig)
{
    //errno = EINVAL;
    return -1; /* Always fails */
}


void _exit(int status)
{
    for(;;){}

}