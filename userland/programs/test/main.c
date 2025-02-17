#include "main.h"
#include "../stdlib/vbe.h"
#include <stdio.h>
#include <stdlib.h>
#include "../stdlib/syscall.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>



void main()
{

    clear_screen(0);

    printf("Hey, im a userprogramm\n");

    while (1)
    {
        // shell implementation with a readline of user input comes here
        // print("$");

        // print("\n");
    }
}