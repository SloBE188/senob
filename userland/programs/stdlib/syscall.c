#include <errno.h>
#include <sys/types.h>

extern int syscall_3_write(int fd, const void* buf, size_t len);
extern void syscall_4_sbrk(int increment);

_READ_WRITE_RETURN_TYPE write(int fd, const void *buf, size_t len)
{
    syscall_3_write(fd, buf, len);
}

int open(const char *name, int flags, ...)
{
    return -1;
}

int close(int file)
{
    return -1;
}

off_t lseek(int file, off_t offset, int whence)
{
    return 0;
}

int read(int file, void *buf, size_t len)
{
    return -1;
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
    errno = EINVAL;
    return -1; /* Always fails */
}

void _exit(int status)
{
    for(;;){}
}


void *sbrk(ptrdiff_t increment)
{
    syscall_4_sbrk(increment);
}