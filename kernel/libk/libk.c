#include <errno.h>
#include <sys/types.h>
#include "stdiok.h"
#include "../../drivers/video/vbe/vbe.h"
#include <unistd.h>
#include "../arch/x86-32/sys/spinlock.h"
#include "../arch/x86-32/sys/process.h"

#undef errno
extern int errno;


int write(int fd, const void *buf, size_t len)
{
    kernel_write(buf);
    return len;
}

void _exit(int status)
{
    process_exit(get_curr_pid());
}

int close (int file)
{
    
    errno = EBADF;
    return -1; /* Always fails */
}
