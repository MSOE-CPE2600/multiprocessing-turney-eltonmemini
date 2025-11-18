/**********************************************
* Filename: mandelmovie.c
* Description: speedup mandelmovie
* gcc -o mandelmovie mandelmovie.c
* Author: Elton Emini
* Date: 11/11/2025
***********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>

#define DFT_SIZE 50
#define DEFAULT_MAX_PROC 12 

int main (int argc, char *argv[]) 
{
    int active_proc = 0;
    int max_proc = DEFAULT_MAX_PROC; 

    if (argc > 1) 
    {
        max_proc = atoi(argv[1]);
        
        if (max_proc < 1) 
        {
            printf("Warning: Invalid max_proc. Resetting to 1.\n");
            max_proc = 1;
        }
        printf("Running with max processes set to: %d\n", max_proc);
    }
    else 
    {
        printf("No argument provided. Using default max processes: %d\n", max_proc);
    }

    int pid;

    for (int k= 0; k<DFT_SIZE ; k++)
    {
            
        // Use the variable max_proc instead of the define
        if(active_proc >= max_proc)
        {
            wait(NULL);
            active_proc--;
        }
        pid = fork();
        if(pid == 0)
        {
            char x[50], y[50], s[50], m[50], o[50];

            sprintf(x, "%f", -0.761574);
            sprintf(y, "%f", -0.0847596);
            sprintf(s, "%f", k * .0005);
            sprintf(m, "%d", 1000 + k * 100);
            sprintf(o, "mandel%d.jpg", DFT_SIZE - k);

            execlp("./mandel", "./mandel", "-x", x, "-y", y, "-s", s, "-m", m, "-o", o, NULL);
            exit(0);
        } 
        else if(pid > 0)
        {
            active_proc++;
        }
    }
    while(active_proc > 0)
    {
        wait(NULL);
        active_proc--;
    }
    
    //char buffer2[100] = "ffmpeg -i mandel%d.jpg mandel.mpg";
    //execlp(buffer2, NULL);


    return 0;
}