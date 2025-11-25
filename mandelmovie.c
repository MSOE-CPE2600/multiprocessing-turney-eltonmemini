/**********************************************
* Filename: mandelmovie.c
* Description: Multiprocess & Multithreaded Movie Generator
* gcc -o mandelmovie mandelmovie.c
***********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define DFT_SIZE 50
#define DEFAULT_MAX_PROC 12 

int main(int argc, char *argv[]) 
{
    int active_proc = 0;
    int max_proc = DEFAULT_MAX_PROC; 
    int num_threads = 1;

    // Parse command line arguments
    if (argc > 1) {
        max_proc = atoi(argv[1]);
        if (max_proc < 1) max_proc = 1;
    }
    
    if (argc > 2) {
        num_threads = atoi(argv[2]);
        if (num_threads < 1) num_threads = 1;
        if (num_threads > 20) num_threads = 20;
    }

    printf("Generating movie with %d max processes and %d threads per process.\n", max_proc, num_threads);

    int pid;
    for (int k = 0; k < DFT_SIZE; k++)
    {
        if (active_proc >= max_proc) {
            wait(NULL);
            active_proc--;
        }

        pid = fork();
        if (pid == 0) {
            // Child Process
            char x[50], y[50], s[50], m[50], o[50], t[50];

            sprintf(x, "%f", -0.761574);
            sprintf(y, "%f", -0.0847596);
            sprintf(s, "%f", (k * .0005) + 0.0001);
            sprintf(m, "%d", 1000 + k * 5);
            sprintf(o, "mandel%d.jpg", DFT_SIZE - k);
            sprintf(t, "%d", num_threads);

            // Execute mandel with the thread argument
            execlp("./mandel", "./mandel", "-x", x, "-y", y, "-s", s, "-m", m, "-o", o, "-n", t, NULL);
            
            perror("execlp failed");
            exit(1);
        } else if (pid > 0) {
            // Parent Process
            active_proc++;
        }
    }

    // Wait for remaining processes
    while (active_proc > 0) {
        wait(NULL);
        active_proc--;
    }
    
    return 0;
}