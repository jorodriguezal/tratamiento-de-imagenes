#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include<pthread.h>
#include<unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
//%cflags:-lpthread -lm -D_DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

int width, height, channels;
unsigned char *input, *output;
size_t input_size, output_size;
int gray_channels,channels ;
char *output_path;

void read_image(char *input_path);
void* gray_filter(void* params);
void write_output(char *output_path, int output_channels);

//Estructura de parámetros para gray_filter
/*struct Params{
    size_t inp;
    size_t oup;
    int n;
    char *output_path;
};*/

void read_image(char *input_path) {
    input = (unsigned char *)stbi_load(input_path, &width, &height, &channels, 0);
    if(input == NULL) {
        perror("Error in loading the image");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
}

void* gray_filter(void* i ) {
    //struct Params *args = (struct Params*)params;
	
    int n = *(int *)i;
    printf("n = %d", n);
    int inp= input_size*(n);
    int oup=output_size*(n);    
    //printf("hilo %i input %i outut %i size %ld",n, inp, oup, output_size);
    // Esto aún no lo entiendo, todo el código me lo encontré por internet
    for(unsigned char *p = input+ inp , *pg = output+ oup ; p != input+ input_size*(n+1); p += channels, pg += gray_channels) {
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2))/3.0);
        if(channels == 4) {
            *(pg + 1) = *(p + 3);
        }
    }
    
   write_output(output_path, gray_channels);
   //free(params);
}

void write_output(char *output_path, int output_channels) {
    char *dot = strrchr(output_path, '.');
    char *ext = dot + 1;

    if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0)
        stbi_write_jpg(output_path, width, height, output_channels, output, 100);
    else if (strcmp(ext, "png") == 0)
        stbi_write_png(output_path, width, height, output_channels, output, width * output_channels);
    else if (strcmp(ext, "bmp") == 0)
        stbi_write_bmp(output_path, width, height, output_channels, output);
    else {
        printf("Output type is  not jpg, png or bmp, defaulting to output.jpg\n");
        stbi_write_jpg("output.jpg", width, height, output_channels, output, 100);
    }
}

int main(int argc, char **argv) 
{ 
	read_image(*(argv + 1));
	int threads = atoi(*(argv + 3));
    pthread_t ar_threads[threads]; 
	gray_channels = channels == 4 ? 2 : 1;
	input_size = width * height * channels/threads;
    output_size = width * height * gray_channels/threads;
	int inp,oup;
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    output = (unsigned char *)malloc(output_size*threads);
    output_path=*(argv + 2);
    if(output == NULL) {
        perror("Unable to allocate memory for the gray image");
        exit(1);
    }

    //struct Params *params;
    for (int i=0;i<threads;i++)
    { 
        //params = malloc(sizeof(struct Params));
    	//params->inp= input_size*(i);
    	//params->oup=output_size*(i);
        //params->output_path=*(argv + 2);
    	//printf("hilo %i\n",i);
        int *n = malloc(sizeof(int));
        *n = i;
        pthread_create(&ar_threads[i], NULL, gray_filter, /*(void *)*/n);
    	
    	
    	printf("fin hilo %i\n",i);
    	//input_size += input_size;
    	//output_size +=output_size;
    }
    for (int i=0; i < threads; i++){
        pthread_join(ar_threads[i], NULL);
    }
	//write_output(output_path, gray_channels);
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Seconds taken: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    stbi_image_free(input);
    stbi_image_free(output);
	
}


// Referencias
//
// [1] El tutorial que seguí se encuentra en el link https://solarianprogrammer.com/2019/06/10/c-programming-reading-writing-images-stb_image-libraries/
// Se editó para que sea modular y más bonito, pero esto va en progreso

