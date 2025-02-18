#ifndef SYSCALL_H
#define SYSCALL_H
#include <sys/types.h>

#define COLOR_BLACK         0x000000
#define COLOR_WHITE         0xFFFFFF
#define COLOR_RED           0xFF0000
#define COLOR_GREEN         0x00FF00
#define COLOR_BLUE          0x0000FF
#define COLOR_LIGHT_GREY    0xD3D3D3

int readdir(const char* path);
int open(const char *name, int flags, ...);
int close(int file);
int read(int file, void *buf, size_t len);
int write(int fd, const void* buf, size_t len);
int mkdir(const char *path, mode_t mode);
int clear_screen(uint32_t color);


#endif