#include <errno.h>
#include <sys/types.h>
#include "stdiok.h"
#include "../../drivers/video/vbe/vbe.h"

extern struct vbe_info* globalvbeinfo;

#undef errno
extern int errno;

int close(int file)
{
    return -1;
}

int execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

int fork(void)
{
    errno = EAGAIN;
    return -1;
}

int fstat(int file, struct stat* st)
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

int link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

int lseek(int file, int ptr, int dir)
{
    return 0;
}

int open(int file, char *ptr, int len)
{
    return 0;
}

int read(int file, char *ptr, int len)
{
    return 0;
}

caddr_t sbrk(int incr)
{
    return incr;
}

int stat(char *file, struct stat *st)
{
    return 0;
}

int times(struct tms *buf)
{
    return -1;
}

int unlink(char *name) 
{
  errno = ENOENT;
  return -1; 
} 

int wait(int *status) {
  errno = ECHILD;
  return -1;
}

//void draw_char(uint32_t x, uint32_t y, char c, uint32_t color, struct vbe_info* vbeinfo)

int write(int file, char *ptr, int len) 
{

    for (uint32_t i = 0; i < len; i++)
    {
        char c = ptr[i];
        draw_char(100, 200, c, COLOR_BLUE, globalvbeinfo);
    }
    

  return len;
}

void _exit(int status)
{
    for(;;){}
}