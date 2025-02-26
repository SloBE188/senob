#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "../stdlib/syscall.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>



void main()
{

    clear_screen(COLOR_BLUE);
    
    printf("Hey, im a userprogramm\n");


    printf("enter smth\n");
    char buff[100];  
    int n = 20;

  
    // Read input from the user
    fgets(buff, n, stdin);
    printf("entered: %s", buff);

    uint32_t ticks = get_ticks_doom();
    printf("ticks: %d\n", ticks);

    uint32_t key = get_key_from_buffer();
    printf("key: %d\n", key);
    char* program = "0:/test.bin";

    execve(program, " ", " ");

    while(1){}
}