#include "../stdlib/syscall.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    clear_screen(COLOR_RED);
    char inbuffer[300];


    printf("Senobhell v0.1 (pid: %d)\n", getpid());

    while(1)
    {
       printf("$");
       fflush(stdout);
       
       char* r = fgets(inbuffer, 300, stdin);

       if(r == NULL)
        continue;
        

    }


    return 0;
}