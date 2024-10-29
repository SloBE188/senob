#ifndef KERNEL_H
#define KERNEL_H

#define NULL 0

void dummyfunction1();
void dummyfunction2();



void kernel_panic(const char* message);
#define assert(condition) \
    do { \
        if (!(condition)) { \
            kernel_panic("Assertion failed: " #condition); \
        } \
    } while (0)

#endif