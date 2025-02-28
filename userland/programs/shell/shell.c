#include "../stdlib/syscall.h"
#include <stdio.h>
#include <string.h>

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
        
        if(strncmp(inbuffer, "0:/", 3) == 0)
        {
            uint32_t res;

            FILE* file = fopen(inbuffer, "rb");
            if(file == NULL)
            {
                printf("There is no such program like %s\n", inbuffer);
                continue;
            }else
            {
                res = execve(inbuffer, " ", " ");
            }
            
        }
        if(strncmp(inbuffer, "ls", 2) == 0)
        {
            readdir("0:/");     //TODO: get curr directory
        }
        

    }


    return 0;
}