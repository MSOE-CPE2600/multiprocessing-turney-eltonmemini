/// 
//  mandel.c
//  Multithreaded Mandelbrot generator
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "jpegrw.h"

// Data structure to pass arguments to worker threads
typedef struct {
    imgRawImage* img;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int max;
    int start_row;
    int end_row;
} ThreadArgs;

// Local routines
static int iteration_to_color(int i, int max);
static int iterations_at_point(double x, double y, int max);
static void compute_image(imgRawImage *img, double xmin, double xmax, double ymin, double ymax, int max, int threads);
static void show_help();
static void* compute_thread(void* args);

int main(int argc, char *argv[])
{
    char c;

    // Default configuration values
    const char *outfile = "mandel.jpg";
    double xcenter = 0;
    double ycenter = 0;
    double xscale = 4;
    double yscale = 0; 
    int    image_width = 1000;
    int    image_height = 1000;
    int    max = 1000;
    int    threads = 1;

    // Parse command line arguments
    while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:h"))!=-1) {
        switch(c) 
        {
            case 'x': xcenter = atof(optarg); break;
            case 'y': ycenter = atof(optarg); break;
            case 's': xscale = atof(optarg); break;
            case 'W': image_width = atoi(optarg); break;
            case 'H': image_height = atoi(optarg); break;
            case 'm': max = atoi(optarg); break;
            case 'o': outfile = optarg; break;
            case 'n': 
                threads = atoi(optarg); 
                if (threads < 1) threads = 1;
                if (threads > 20) threads = 20;
                break;
            case 'h': show_help(); exit(1); break;
        }
    }

    // Calculate y scale
    yscale = xscale / image_width * image_height;

    // Display configuration
    printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d threads=%d outfile=%s\n",
           xcenter, ycenter, xscale, yscale, max, threads, outfile);

    // Create and initialize image
    imgRawImage* img = initRawImage(image_width, image_height);
    setImageCOLOR(img, 0);

    // Compute the Mandelbrot image using threads
    compute_image(img, xcenter-xscale/2, xcenter+xscale/2, ycenter-yscale/2, ycenter+yscale/2, max, threads);

    // Save and free
    storeJpegImageFile(img, outfile);
    freeRawImage(img);

    return 0;
}

/*
 * Thread worker function.
 * Computes a specific band of rows in the image.
 */
void* compute_thread(void* arguments) {
    ThreadArgs* args = (ThreadArgs*)arguments;
    int width = args->img->width;
    int height = args->img->height;

    for(int j = args->start_row; j < args->end_row; j++) {
        for(int i = 0; i < width; i++) {
            double x = args->xmin + i * (args->xmax - args->xmin) / width;
            double y = args->ymin + j * (args->ymax - args->ymin) / height;

            int iters = iterations_at_point(x, y, args->max);
            setPixelCOLOR(args->img, i, j, iteration_to_color(iters, args->max));
        }
    }
    return NULL;
}

/*
 * Calculates iterations at a specific point.
 */
int iterations_at_point(double x, double y, int max)
{
    double x0 = x;
    double y0 = y;
    int iter = 0;

    while((x*x + y*y <= 4) && iter < max) {
        double xt = x*x - y*y + x0;
        double yt = 2*x*y + y0;
        x = xt;
        y = yt;
        iter++;
    }
    return iter;
}

/*
 * Orchestrates the computation by spawning threads.
 */
void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int threads)
{
    pthread_t thread_ids[threads];
    ThreadArgs args[threads];
    int height = img->height;

    for (int i = 0; i < threads; i++) {
        args[i].img = img;
        args[i].xmin = xmin;
        args[i].xmax = xmax;
        args[i].ymin = ymin;
        args[i].ymax = ymax;
        args[i].max = max;

        // Divide the image height among threads
        args[i].start_row = i * height / threads;
        if (i == threads - 1) {
            args[i].end_row = height; // Last thread takes remainder
        } else {
            args[i].end_row = (i + 1) * height / threads;
        }

        if (pthread_create(&thread_ids[i], NULL, compute_thread, &args[i]) != 0) {
            perror("Failed to create thread");
            exit(1);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }
}

/*
 * Maps iteration count to color.
 */
int iteration_to_color(int iters, int max)
{
    int color = 0xFFFFFF * iters / (double)max;
    return color;
}

void show_help()
{
    printf("Use: mandel [options]\n");
    printf("Where options are:\n");
    printf("-m <max>     The maximum number of iterations per point. (default=1000)\n");
    printf("-x <coord>   X coordinate of image center point. (default=0)\n");
    printf("-y <coord>   Y coordinate of image center point. (default=0)\n");
    printf("-s <scale>   Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
    printf("-W <pixels>  Width of the image in pixels. (default=1000)\n");
    printf("-H <pixels>  Height of the image in pixels. (default=1000)\n");
    printf("-o <file>    Set output file. (default=mandel.bmp)\n");
    printf("-n <threads> Number of threads to use (1-20). (default=1)\n");
    printf("-h           Show this help text.\n");
}