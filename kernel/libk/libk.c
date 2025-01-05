#include <errno.h>
#include <sys/types.h>
#include "stdiok.h"
#include "../../drivers/video/vbe/vbe.h"
#include <unistd.h>
#include "../arch/x86-32/sys/spinlock.h"
#include "../arch/x86-32/sys/process.h"
#include <sys/stat.h>

#undef errno
extern int errno;

extern struct process *root;

int write(int fd, const void *buf, size_t len)
{
    kernel_write(buf);
    return len;
}

void exit(int status)
{
    process_exit(get_curr_pid());
}

void _exit(int status)
{
    process_exit(get_curr_pid());
}

int close(int file)
{

    errno = EBADF;
    return -1; /* Always fails */
}

int execve (const char *__path, char * const __argv[], char * const __envp[])
{
    errno = ENOMEM;
    return -1; /* Always fails */
}

int fork()
{
    struct process *calling_proc = rb_search(root, get_curr_pid());

    if (calling_proc == NULL)
    {
        kernel_write("fork failed, the calling process couldnt be found\n");
        return;
    }

    struct process *proc;

    if (calling_proc->isuserproc == 1)
    {
        proc = create_process(calling_proc->filename);
    }
    else
    {
        proc = create_kernel_process(calling_proc->head_thread->regs.eip);
    }

    kernel_write("Fork successfull\n");

    return proc->pid;
}

int fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int getpid()
{
    return get_curr_pid();
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

int link(const char *old, const char *new)
{
    errno = EMLINK;
    return -1; /* Always fails */
}

off_t lseek(int file, off_t offset, int whence)
{
    return 0;
}

int open(const char *name, int flags, int mode)
{
    errno = ENOSYS;
    return -1; /* Always fails */
}

int read(int file, void *buf, size_t len)
{
    return 0; /* EOF */
}


void* sbrk(ptrdiff_t nbytes)
{

}

int stat(const char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int times(struct tms *buf)
{
    errno = EACCES;
    return -1;
}

int unlink(const char *name)
{
    errno = ENOENT;
    return -1; /* Always fails */
}

int wait(int *status)
{
    errno = ECHILD;
    return -1; /* Always fails */
}

static void outbyte(char c)
{
    _uart_putc(c);
}
