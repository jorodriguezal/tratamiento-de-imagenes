#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <mpi.h>
#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


int width, height, channels, gray_channels;
unsigned char *input, *output;
size_t input_size, output_size;
char *output_path;

void read_image(char *input_path);
void* gray_filter(void* params);
void write_output(char *output_path, int output_channels);


void read_image(char *input_path) {
    input = (unsigned char *)stbi_load(input_path, &width, &height, &channels, 0);
    if(input == NULL) {
        perror("Error in loading the image");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
}

void* gray_filter(void* i) {
    int n = *(int *)i;
    n++;
    printf("N = %i", n);
    int inp= input_size*(n);
    int oup=output_size*(n);    
    for(unsigned char *p = input + inp, *pg = output + oup; p != input + input_size*(n+1); p += channels, pg += gray_channels) {
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2))/3.0);
        if(channels == 4) 
            *(pg + 1) = *(p + 3);
    }
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
        printf("Output type is not jpg, png or bmp, defaulting to output.jpg\n");
        stbi_write_jpg("output.jpg", width, height, output_channels, output, 100);
    }
}

int main(int argc, char **argv) 
{ 
    printf("Holi");    

    //Inicializar MPI
    MPI_Init(&argc, &argv);

    //Número total de procesos
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    printf("World size = %i\n", world_size);

    // Número del hilo
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    printf("World Rank = %i\n", world_rank);

    //Path de la imagen
    read_image(*(argv + 1));

    //Número de canales
	gray_channels = channels == 4 ? 2 : 1;

    //Tamaño de matrices
	input_size = width * height * channels/world_size;
    printf("Input size = %u\n", input_size);
    output_size = width * height * gray_channels/world_size;
    printf("Output size = %u\n", output_size);
	
    printf("Toma de tiempos...");
    //Toma inicial de tiempos
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    printf("Toma de tiempos incial exitosa...");
    //Asignar output
    printf("Output a asignar...");
    output = (unsigned char *)malloc(output_size*world_size);
    printf("Output Asignado");
    output_path=*(argv + 2);

    //Manejo de erores
    if(output == NULL) {
        perror("Unable to allocate memory for the gray image");
        exit(1);
    }

    gray_filter(&world_size);

	write_output(output_path, gray_channels);


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

