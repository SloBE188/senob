#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "../stdlib/syscall.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>



void main()
{

    int res = clear_screen(COLOR_RED);
    
    printf("Hey, im a userprogramm\n");

    char buff[100];  
    int n = 10;
  
    printf("Enter a string: ");
  
    // Read input from the user
    fgets(buff, n, stdin);
    printf("You entered: %s", buff);

    while (1)
    {
        // shell implementation with a readline of user input comes here
        // print("$");

        // print("\n");
    }
}