#include "main.h"
#include "../stdlib/vbe.h"
#include <stdio.h>
#include <stdlib.h>
#include "../stdlib/syscall.h"

void main()
{

    //char input[1024];
    clear_screen(0);
    print("Welcome to the test programm 1 from senob!\n");
    //printf("Is this newlib or what");
    /*int a = 200;
    char buffer[12];
    char buf[30];
    
    char* string = "ich bin der string";
    int length = strlen(string);

    itoa(length, buf, 10);
    print(buf);

    itoa(a, buffer, 10);
    print(buffer);*/


    //char alloc[20];
    int* p = (int*)malloc(5000);
    print("funktioniert?\n");
    //itoa(p, buf, 16);
    //print(alloc);

    printf("hayyy, hier der pointer: %x\n", p);


    int a = 19000;
    printf("a ist : %d\n", a);

    void *p1 = malloc(32);
    printf("malloc(32) -> %p\n", p1);
    void *p2 = malloc(64);
    printf("malloc(64) -> %p\n", p2);
    void *p3 = malloc(128);
    printf("malloc(128) -> %p\n", p3);
    void *p4 = sbrk(0);
    printf("sbrk(0) after malloc -> %p\n", p4);
    void *ptr = sbrk(128);
    printf("sbrk(128) -> %p\n", ptr);
    void *heap1 = sbrk(0);
    printf("Heap before malloc: %p\n", heap1);

    void *p9 = malloc(8192);
    printf("big memory allocation: %p\n", p9);

    readdir("0:/");






    
    while (1)
    {
        //shell implementation with a readline of user input comes here
        //print("$");

        //print("\n");

    }
    
}