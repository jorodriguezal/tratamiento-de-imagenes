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

//Declaraciones
int width, height, channels, gray_channels;
unsigned char *input, *output;
size_t input_size, output_size;
char *output_path;

void read_image(char *input_path);
void* gray_filter(void* params);
void write_output(char *output_path, int output_channels);

//lee la imagen en la dirección ingresada
void read_image(char *input_path) {

    //cargar la imagen (asigna a los argumentos dados los valores correspondientes a alto, ancho y canales de la imagen)
    input = (unsigned char *)stbi_load(input_path, &width, &height, &channels, 0);

    //si no se pudo cargar laimagen
    if(input == NULL) {
        perror("Error in loading the image");
        exit(1);
    }
    //impresión de información de la imagen
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
}


//filtro gris
void* gray_filter(void* thread_index) {

    //se asignan posiciones de lectura y escritura según el nùmero de hilo y el tamaño de lo que le corresponde a cada uno
    int n_thread = *(int *)thread_index;
    int inp_begin = input_size*(n_thread);
    int oup_begin = output_size*(n_thread);

    //se itera entre la posición inicial y final designada para el hilo
    for(unsigned char *p = input + inp_begin , *pg = output + oup_begin ; p != input + input_size*(n_thread+1); p += channels, pg += gray_channels) {

        //se realiza transformación en el pixel
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2))/3.0);

        //si se tienen 4 canales
        if(channels == 4) {
            *(pg + 1) = *(p + 3);
        }
    }

    //liberar número del hilo
    free(thread_index);
}

//escribir los nuevos datos en el archivo con la dirección de salida
void write_output(char *output_path, int output_channels) {
    
    //obtener último punto del archivo para encontrar extensión
    char *dot = strrchr(output_path, '.');
    char *ext = dot + 1;

    //escribe según el tipo de archivo deseado
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
    //cargar la imagen
	read_image(*(argv + 1));

    //hilos ingresados
	int threads = atoi(*(argv + 3));
    pthread_t ar_threads[threads];

    //canales
	gray_channels = channels == 4 ? 2 : 1;

    //se definen tamaños de segmento a asignar a cada hilo
	input_size = width * height * channels/threads;
    output_size = width * height * gray_channels/threads;
	
    //valores para calcular el tiempo de ejecución
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);

    //para guardar datos de la imagen con filtro
    output = (unsigned char *)malloc(output_size*threads);

    //patrón de salida
    output_path=*(argv + 2);

    //error en malloc
    if(output == NULL) {
        perror("Unable to allocate memory for the gray image");
        exit(1);
    }

    //ciclo para crear hilos
    for (int i=0;i<threads;i++)
    { 
        //apuntador a la iteración actual
        int *n = malloc(sizeof(int));
        *n = i;
        //lanzar hilos
        pthread_create(&ar_threads[i], NULL, gray_filter, n);
    }

    //termina cuando acaben los hilos
    for (int i=0; i < threads; i++){
        pthread_join(ar_threads[i], NULL);
    }

    //se transforma información resultante en imagen
    write_output(output_path, gray_channels);

    //se obtiene el tiempo de ejecución
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Seconds taken: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    //se libera espacio de la imagen
    stbi_image_free(input);
    stbi_image_free(output);
}
