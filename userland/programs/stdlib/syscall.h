#ifndef SYSCALL_H
#define SYSCALL_H

int readdir(const char* path);
int open(const char *name, int flags, ...);
int close(int file);
int read(int file, void *buf, size_t len);


#endif