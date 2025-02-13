#include <errno.h>
#include <sys/types.h>
#include "stdiok.h"
#include "../../drivers/video/vbe/vbe.h"
#include <unistd.h>
#include "../arch/x86-32/sys/spinlock.h"
#include "../arch/x86-32/sys/process.h"
#include <sys/stat.h>
#include "../arch/x86-32/fatfs/ff.h"
#include <fcntl.h>
#include <sys/types.h>
#include <stddef.h>
#include "memory.h"
#include "../arch/x86-32/mm/heap/heap.h"

#undef errno
extern int errno;

#define MAX_FILES 32
#define _MAX_LFN 255
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern struct process *root;

// struct for additionally saving the paths
typedef struct
{
    FIL *fil;
    TCHAR path[_MAX_LFN + 1];
} FileEntry;

FIL *file_table[MAX_FILES] = {0};
FileEntry file_entries[MAX_FILES] = {0};

static int map_fresult_to_errno(FRESULT res)
{
    switch (res)
    {
    case FR_OK:
        return 0;
    case FR_DISK_ERR:
        return EIO;
    case FR_INT_ERR:
        return EIO;
    case FR_NOT_READY:
        return ENODEV;
    case FR_NO_FILE:
        return ENOENT;
    case FR_NO_PATH:
        return ENOENT;
    case FR_INVALID_NAME:
        return EINVAL;
    case FR_DENIED:
        return EACCES;
    case FR_EXIST:
        return EEXIST;
    case FR_INVALID_OBJECT:
        return EBADF;
    case FR_WRITE_PROTECTED:
        return EROFS;
    case FR_INVALID_DRIVE:
        return EINVAL;
    case FR_NOT_ENABLED:
        return ENODEV;
    case FR_NO_FILESYSTEM:
        return ENODEV;
    case FR_MKFS_ABORTED:
        return EIO;
    case FR_TIMEOUT:
        return ETIMEDOUT;
    case FR_LOCKED:
        return EBUSY;
    case FR_NOT_ENOUGH_CORE:
        return ENOMEM;
    case FR_TOO_MANY_OPEN_FILES:
        return EMFILE;
    case FR_INVALID_PARAMETER:
        return EINVAL;
    default:
        return EIO;
    }
}

// searches free file descriptor
static int get_free_fd()
{
    for (int fd = 0; fd < MAX_FILES; fd++)
    {
        if (file_table[fd] == NULL)
        {
            return fd;
        }
    }
    return -1;
}

// translation from open flags from newlib to fatfs flags
static BYTE translate_flags(int flags)
{
    BYTE mode = FA_READ;

    if ((flags & O_RDWR) == O_RDWR)
    {
        mode = FA_READ | FA_WRITE;
    }
    else if (flags & O_WRONLY)
    {
        mode = FA_WRITE;
    }

    if (flags & O_CREAT)
    {
        mode |= FA_CREATE_ALWAYS;
    }

    if (flags & O_APPEND)
    {
        mode |= FA_OPEN_APPEND;
    }

    if (flags & O_TRUNC)
    {
        mode |= FA_CREATE_ALWAYS;
    }

    return mode;
}

int open(const char *name, int flags, ...)
{  
    
    if (name == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    int fd = get_free_fd();
    if (fd < 0)
    {
        errno = EMFILE; // too many open files
        return -1;
    }

    file_entries[fd].fil = kmalloc(sizeof(FIL));
    strncpy(file_entries[fd].path, name, _MAX_LFN);
    file_entries[fd].path[_MAX_LFN] = '\0';
    printf("added file entry: %s\n", file_entries[fd].path);

    FRESULT res = f_open(file_entries[fd].fil, file_entries[fd].path, translate_flags(flags));
    if (res != FR_OK)
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    file_table[fd] = file_entries[fd].fil;


    return fd;
}

int close(int file)
{

    if (file < 0 || file >= MAX_FILES || file_table[file] == NULL)
    {
        errno = EBADF;
        return -1;
    }

    FRESULT res = f_close(file_table[file]);
    if (res != FR_OK)
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    file_table[file] = NULL;
    file_entries[file].path[0] = '\0';
    return 0;
}



off_t lseek(int file, off_t offset, int whence)
{
    if (file < 0 || file >= MAX_FILES || file_table[file] == NULL)
    {
        errno = EBADF;
        return (off_t)-1;
    }

    FRESULT res;
    FSIZE_t new_pos = 0;
    FILINFO fno;

    switch (whence)
    {
    case SEEK_SET:
        res = f_lseek(file_table[file], (FSIZE_t)offset);
        if (res != FR_OK)
        {
            errno = map_fresult_to_errno(res);
            return (off_t)-1;
        }
        new_pos = f_tell(file_table[file]);
        break;

    case SEEK_CUR:
        res = f_lseek(file_table[file], file_table[file]->fptr + offset);
        if (res != FR_OK)
        {
            errno = map_fresult_to_errno(res);
            return (off_t)-1;
        }
        new_pos = f_tell(file_table[file]);
        break;

    case SEEK_END:
        res = f_stat(file_entries[file].path, &fno);
        if (res != FR_OK)
        {
            errno = map_fresult_to_errno(res);
            return (off_t)-1;
        }
        res = f_lseek(file_table[file], fno.fsize + offset);
        if (res != FR_OK)
        {
            errno = map_fresult_to_errno(res);
            return (off_t)-1;
        }
        new_pos = f_tell(file_table[file]);
        break;

    default:
        errno = EINVAL;
        return (off_t)-1;
    }

    return (off_t)new_pos;
}

int read(int file, void *buf, size_t len)
{
    if (file < 0 || file >= MAX_FILES || file_table[file] == NULL || buf == NULL)
    {
        errno = EBADF;
        return -1;
    }

    UINT br;
    FRESULT res = f_read(file_table[file], buf, len, &br);
    if (res != FR_OK)
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    return br;
}

int stat(const char *path, struct stat *st)
{
    if (path == NULL || st == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    FILINFO fno;
    FRESULT res = f_stat(path, &fno);
    if (res != FR_OK)
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    st->st_size = fno.fsize;
    st->st_mode = (fno.fattrib & AM_DIR) ? S_IFDIR : S_IFREG;

    return 0;
}

int fstat(int file, struct stat *st) 
{

    if (file < 0 || file >= MAX_FILES || file_table[file] == NULL) 
    {
        errno = EBADF;
        return -1;
    }
    
    /*for (size_t i = 0; i < MAX_FILES; i++)
    {
        printf("file entry %d: %s\n", i, file_entries[i].path);
    }*/
    return stat(file_entries[file].path, st);

}

_READ_WRITE_RETURN_TYPE write(int fd, const void *buf, size_t len)
{

    if (buf == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // STDOUT & STDERR -> kernel write output
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
    {
        char temp_buf[len + 1];
        memcpy(temp_buf, buf, len);
        temp_buf[len] = '\0';
        kernel_write(temp_buf);
        return len;
    }

    // FATfs write operations
    if (fd < 0 || fd >= MAX_FILES || file_table[fd] == NULL)
    {
        errno = EBADF;
        return -1;
    }

    UINT bw;
    FRESULT res = f_write(file_table[fd], buf, len, &bw);
    if (res != FR_OK)
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    return (ssize_t)bw;
}

int unlink(const char *name)
{
    if (name == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    FRESULT res = f_unlink(name);
    if (res != FR_OK)
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    return 0;
}

int mkdir(const char *path, mode_t mode)
{
    if (path == NULL) 
    {
        errno = EINVAL;
        return -1;
    }

    FRESULT res = f_mkdir(path);
    if (res != FR_OK) 
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    return 0;
}

/*
int chdir(const char *path)
{
    if (path == NULL) 
    {
        errno = EINVAL;
        return -1;
    }

    FRESULT res = f_chdir(path);
    if (res != FR_OK) 
    {
        errno = map_fresult_to_errno(res);
        return -1;
    }

    return 0;
}

char *getcwd(char *buf, size_t size)
{
    if (buf == NULL || size == 0) 
    {
        errno = EINVAL;
        return NULL;
    }

    FRESULT res = f_getcwd(buf, size);
    if (res != FR_OK) 
    {
        errno = map_fresult_to_errno(res);
        return NULL;
    }

    return buf;
}
*/
int dup(int fd)
{
    //search free fd
    int new_fd = get_free_fd();
    if (new_fd < 0) 
    {
        errno = EMFILE;
        return -1;
    }

    //copy reference file
    file_table[new_fd] = file_table[fd];
    file_entries[new_fd].path[0] = '\0';

    return new_fd;
}

int dup2(int oldfd, int newfd)
{
    if (oldfd < 0 || oldfd >= MAX_FILES || file_table[oldfd] == NULL) 
    {
        errno = EBADF;
        return -1;
    }

    if (newfd < 0 || newfd >= MAX_FILES) 
    {
        errno = EBADF;
        return -1;
    }

    if (file_table[newfd] != NULL) 
    {
        close(newfd);
    }

    file_table[newfd] = file_table[oldfd];
    strncpy(file_entries[newfd].path, file_entries[oldfd].path, _MAX_LFN);
    file_entries[newfd].path[_MAX_LFN] = '\0';

    return newfd;
}





void exit(int status)
{
    process_exit(get_curr_pid());
}

void _exit(int status)
{
    process_exit(get_curr_pid());
}

int execve(const char *__path, char *const __argv[], char *const __envp[])
{
    errno = ENOMEM;
    return -1; /* Always fails */
}

// TODO should be improved with copying the regs and the user & kernel stack from the parent process
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

static void* heap_end = 0xC3000000;
static void* heap_limit = 0xC7000000;

void *sbrk(ptrdiff_t increment)
{
    void* old_heap_end = heap_end;
    void* new_heap_end = old_heap_end + increment;

    if (new_heap_end > heap_limit)
    {
        return (void*) -1;      //no more memory;
    }
    
    heap_end = new_heap_end;
    return old_heap_end;

}

int times(struct tms *buf)
{
    errno = EACCES;
    return -1;
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
