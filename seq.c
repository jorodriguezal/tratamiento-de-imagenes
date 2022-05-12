#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


int width, height, channels;
unsigned char *input, *output;

void read_image(char *input_path);
void gray_filter(char *output_path);
void write_output(char *output_path, int output_channels);

void read_image(char *input_path) {
    input = (unsigned char *)stbi_load(input_path, &width, &height, &channels, 0);
    if(input == NULL) {
        perror("Error in loading the image");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
}

void gray_filter(char *output_path) {
    size_t input_size = width * height * channels;
    int gray_channels = channels == 4 ? 2 : 1; // Si la imagen es de 4 canales (RGB-alpha) los canales de la imagen en blanco y negro serán 2 (YA, Y = monocromático) si son 3 canales (RGB) el resultado es 1 canal (Y)
    size_t output_size = width * height * gray_channels;

    output = (unsigned char *)malloc(output_size);
    if(output == NULL) {
        perror("Unable to allocate memory for the gray image");
        exit(1);
    }
    // Itera cada pixel con apuntadores (RGB-alpha si son 4 canales. RGB si son 3) aplica la fórmula 
    for(unsigned char *p = input, *pg = output; p != input + input_size; p += channels, pg += gray_channels) {
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2))/3.0);
        if(channels == 4) {
            *(pg + 1) = *(p + 3);
        }
    }
    write_output(output_path, gray_channels);
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

int main(int argc, char **argv) { 
    read_image(*(argv + 1)); 
    
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);

    gray_filter(*(argv + 2));

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
