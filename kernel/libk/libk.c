#include <errno.h>
#include <sys/types.h>
#include "stdiok.h"
#include "../../drivers/video/vbe/vbe.h"
#include <unistd.h>

extern struct vbe_info *globalvbeinfo;

#undef errno
extern int errno;

int close(int file)
{
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1; /* Always fails */
}

int fork(void)
{
    errno = EAGAIN;
    return -1;
}

int fstat(int file, struct stat *st)
{
    return 0;
}

int getpid(void)
{
    return 1;
}

int isatty(int file)
{
    return 1;
}

int kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _link(char *old,
          char *new)
{
    errno = EMLINK;
    return -1; /* Always fails */
}

off_t lseek(int file,
          off_t offset,
          int whence)
{
    return 0;
}

int open(int file, char *ptr, int len)
{
    return 0;
}

int read(int fd,
         void *buf,
         size_t len)
{
    return 0; /* EOF */
}

void *sbrk(ptrdiff_t incr)
{
    return 0;
}

int stat(char *file, struct stat *st)
{
    return 0;
}

int times(struct tms *buf)
{
    return -1;
}

int _unlink(char *name)
{
    errno = ENOENT;
    return -1; /* Always fails */
}

int wait(int *status)
{
    errno = ECHILD;
    return -1;
}

// void draw_char(uint32_t x, uint32_t y, char c, uint32_t color, struct vbe_info* vbeinfo)

int write(int fd, const void *buf, size_t len)
{
    kernel_write(buf);
    return len;
}

void _exit(int status)
{
    for (;;)
    {
    }
}