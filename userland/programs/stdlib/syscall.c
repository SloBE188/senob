#include <errno.h>
#include <sys/types.h>

extern int syscall_3_write(int fd, const void* buf, size_t len);
extern void* syscall_4_sbrk(int increment);

_READ_WRITE_RETURN_TYPE write(int fd, const void *buf, size_t len)
{
    syscall_3_write(fd, buf, len);
}

int open(const char *name, int flags, ...)
{

    int fd;
    __asm__ volatile(
        "mov $5, %%eax          \n"
        "mov %1, %%ebx          \n"     //name
        "mov %2, %%ecx          \n"     //flags
        "int $0x80              \n"             
        "mov %%eax, %0          \n"     //retourned fd
        : "=r" (fd)
        : "r" (name), "r" (flags)
        : "eax", "ebx", "ecx"
    );
    
    if(fd == -1)
        return -1;

    return fd;
}

int readdir(const char* path)
{
    int fresult;
    __asm__ volatile(
        "mov $6, %%eax          \n"
        "mov %1, %%ebx          \n"
        "int $0x80              \n"
        "mov %%eax, %0          \n"
        : "=r" (fresult)
        : "r" (path)
        : "eax", "ebx"
    );

    return fresult;
}

int close(int file)
{
    int res;
    __asm__ volatile(
        "mov $7, %%eax          \n"
        "mov %1, %%ebx          \n"
        "int $0x80              \n"
        "mov %%eax, %0          \n"
        : "=r" (res)
        : "r" (file)
    );

    return res;    
}

off_t lseek(int file, off_t offset, int whence)
{
    return 0;
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


int stat(const char *path, struct stat *st)
{
    return -1;
}

int fstat(int file, struct stat *st) 
{
    return -1;
}

int unlink(const char *name)
{
    return -1;
}

int mkdir(const char *path, mode_t mode)
{
    return -1;
}

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


void *sbrk(ptrdiff_t increment)
{
    long ret;
    __asm__ volatile(
        "mov $4, %%eax      \n"
        "mov %1, %%ebx      \n"
        "int $0x80          \n"
        "mov %%eax, %0      \n"
        : "=r" (ret)
        : "r" (increment)
        : "eax", "ebx"
    );

    if(ret == -1)
        return (void*) -1;

    return (void*) ret;
}