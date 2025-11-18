/**********************************************
*  Filename: mandemovie.c
*  Description: speedup mandelmovie
*  gcc -o mandelmovie mandelmovie.c
*  Author: Elton Emini
*  Date: 11/11/2025
***********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>

#define DFT_SIZE 50
#define MAX_PROC 1

int main (void)
{
    int active_proc = 0;

    for (int k= 0; k<DFT_SIZE ; k++)
    {
            
        if(active_proc >= MAX_PROC)
        {
            wait(NULL);
            active_proc--;
        }
        int pid = fork();
        if(pid == 0)
        {
            char buffer[100];
            sprintf(buffer, "./mandel -x %f -y %f -s %f -m %d -o mandel%d.jpg", -0.761574, -0.0847596,  k * .0005, 1000 + k * 100, DFT_SIZE - k);
            system(buffer);
            exit(0);
        } 
        else if(pid > 0)
        {
            active_proc++;
        }
    }
    while(active_proc < 0)
    {
        wait(NULL);
        active_proc--;
    }
    
    //char buffer2[100] = "ffmpeg -i mandel%d.jpg mandel.mpg";
    //system(buffer2);


    return 0;
}