#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "omp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

int width, height, channels, gray_channels;
unsigned char *input, *output;
size_t input_size, output_size;
char *output_path;

void read_image(char *input_path);
void write_output(char *output_path, int output_channels);

void read_image(char *input_path)
{
    input = (unsigned char *)stbi_load(input_path, &width, &height, &channels, 0);
    if (input == NULL)
    {
        perror("Error in loading the image");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
}


void write_output(char *output_path, int output_channels)
{
    char *dot = strrchr(output_path, '.');
    char *ext = dot + 1;

    if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0)
        stbi_write_jpg(output_path, width, height, output_channels, output, 100);
    else if (strcmp(ext, "png") == 0)
        stbi_write_png(output_path, width, height, output_channels, output, width * output_channels);
    else if (strcmp(ext, "bmp") == 0)
        stbi_write_bmp(output_path, width, height, output_channels, output);
    else
    {
        printf("Output type is not jpg, png or bmp, defaulting to output.jpg\n");
        stbi_write_jpg("output.jpg", width, height, output_channels, output, 100);
    }


    stbi_image_free(input);
    stbi_image_free(output);
}

int main(int argc, char **argv)
{
    //leer imagen
    read_image(*(argv + 1));
    
    //hilos a usar
    int threads = atoi(*(argv + 3));

    //canales
    gray_channels = channels == 4 ? 2 : 1;

    //tamaños de input y output por hilo
    input_size = width * height * channels / threads;
    output_size = width * height * gray_channels / threads;

    //memoria para salida
    output = (unsigned char *)malloc(output_size * threads);

    //archivo de destino
    output_path = *(argv + 2);
    if (output == NULL)
    {
        perror("Unable to allocate memory for the gray image");
        exit(1);
    }

    //tiempo inicial
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);

    //paralelización con OpenMP
    omp_set_num_threads(threads);
#pragma omp parallel
    {
        int n, inp, oup;
        n = omp_get_thread_num();
        inp = input_size * (n);
        oup = output_size * (n);

        //asigna a cada pixel de destino el promedio de sus canales (filtro gris)
        for (unsigned char *p = input + inp, *pg = output + oup; p != input + input_size * (n + 1); p += channels, pg += gray_channels)
        {
            *pg = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
            if (channels == 4)
                *(pg + 1) = *(p + 3);
        }
    }

    //medida de tiempo de ejecución
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Seconds taken: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    //devuelve imagen
    write_output(output_path, gray_channels);
}

// Referencias
//
// [1] El tutorial que seguí se encuentra en el link https://solarianprogrammer.com/2019/06/10/c-programming-reading-writing-images-stb_image-libraries/
// Se editó para que sea modular y más bonito, pero esto va en progreso
