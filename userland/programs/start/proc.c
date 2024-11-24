#include "proc.h"

void load_proc(const char* programname)
{
    syscall_1_load_proc(programname);
}