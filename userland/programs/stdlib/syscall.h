#ifndef SYSCALL_H
#define SYSCALL_H
#include <sys/types.h>

int readdir(const char* path);
int open(const char *name, int flags, ...);
int close(int file);
int read(int file, void *buf, size_t len);
int write(int fd, const void* buf, size_t len);
int mkdir(const char *path, mode_t mode);


#endif