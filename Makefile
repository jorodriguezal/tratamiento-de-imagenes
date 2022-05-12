all:
	gcc seq.c -o sq -lm 
	gcc pthread.c -o par_pt -lm -pthread
	gcc OMP.c -o par_omp -lm -fopenmp 

