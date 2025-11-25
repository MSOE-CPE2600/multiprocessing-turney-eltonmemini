/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include "jpegrw.h"

// Struct to pass arguments to threads
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

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int threads );
static void show_help();
void *compute_thread(void *arguments); // Worker function prototype


int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;
	int    threads = 1; // Default to 1 thread

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				threads = atoi(optarg);
				if (threads < 1) threads = 1;
				if (threads > 20) threads = 20;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d threads=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max,threads,outfile);

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(image_width,image_height);

	// Fill it with a black
	setImageCOLOR(img,0);

	// Compute the Mandelbrot image
	compute_image(img,xcenter-xscale/2,xcenter+xscale/2,ycenter-yscale/2,ycenter+yscale/2,max, threads);

	// Save the image in the stated file.
	storeJpegImageFile(img,outfile);

	// free the mallocs
	freeRawImage(img);

	return 0;
}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Worker thread function to compute a specific region
*/
void *compute_thread(void *arguments)
{
	ThreadArgs *args = (ThreadArgs *)arguments;

	int i, j;
	int width = args->img->width;
	int height = args->img->height; // Used for scale calcs, but loop limit is args->end_row

	// For every pixel in the assigned region...
	for(j = args->start_row; j < args->end_row; j++) {

		for(i = 0; i < width; i++) {

			// Determine the point in x,y space for that pixel.
			double x = args->xmin + i*(args->xmax - args->xmin)/width;
			double y = args->ymin + j*(args->ymax - args->ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,args->max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(args->img,i,j,iteration_to_color(iters,args->max));
		}
	}
	pthread_exit(NULL);
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int threads)
{
	pthread_t thread_ids[threads];
	ThreadArgs thread_args[threads];

	int height = img->height;
	int rows_per_thread = height / threads;

	int i;

	// Create threads
	for (i = 0; i < threads; i++) {
		thread_args[i].img = img;
		thread_args[i].xmin = xmin;
		thread_args[i].xmax = xmax;
		thread_args[i].ymin = ymin;
		thread_args[i].ymax = ymax;
		thread_args[i].max = max;

		thread_args[i].start_row = i * rows_per_thread;
		
		// Handle the last thread taking the remainder
		if (i == threads - 1) {
			thread_args[i].end_row = height;
		} else {
			thread_args[i].end_row = (i + 1) * rows_per_thread;
		}

		if (pthread_create(&thread_ids[i], NULL, compute_thread, &thread_args[i]) != 0) {
			perror("Failed to create thread");
			exit(1);
		}
	}

	// Join threads
	for (i = 0; i < threads; i++) {
		pthread_join(thread_ids[i], NULL);
	}
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-n <threads> Number of threads to use (1-20). (default=1)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}