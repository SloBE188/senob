#include "main.h"
#include "../stdlib/vbe.h"
#include <stdio.h>
#include <stdlib.h>

void main()
{

    //char input[1024];
    clear_screen(0);
    print("Welcome to the test programm 1 from senob!\n");
    //printf("Is this newlib or what");
    int a = 200;
    char buffer[12];
    char buf[30];
    
    char* string = "ich bin der string";
    int length = strlen(string);

    itoa(length, buf, 10);
    print(buf);

    itoa(a, buffer, 10);
    print(buffer);

    
    while (1)
    {
        //shell implementation with a readline of user input comes here
        //print("$");

        //print("\n");

    }
    
}