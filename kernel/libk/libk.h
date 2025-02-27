#ifndef LIBK_H
#define LIBK_H


#include <stddef.h>
#include <sys/reent.h>
#include <sys/types.h>


int fstat(int file, struct stat *st);
int close(int file);
int open(const char *name, int flags, ...);
int read(int file, void *buf, size_t len);
_READ_WRITE_RETURN_TYPE write(int fd, const void *buf, size_t len);
off_t lseek(int file, off_t offset, int whence);
int stat(const char *path, struct stat *st);
int mkdir(const char *path, mode_t mode);
int execve(char *name, char **argv, char **env);


#endif