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

    while(1){}
}