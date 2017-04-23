#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 100000 //Tamanho do array
#define N_ARRAYS  1000 // Quantidade de arrays

/* Função de que é usado pelo qsort */
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}

int main(int argc,char **argv){
	clock_t begin = clock();

	int i, j;
	// Aloca as matrizes
  int (*bag_of_tasks)[ARRAY_SIZE] = malloc(N_ARRAYS * sizeof *bag_of_tasks);

  for (i = 0; i < N_ARRAYS; i++){
    for(j = 0; j < ARRAY_SIZE; j++){
      bag_of_tasks[i][j] = (ARRAY_SIZE-j-1); // populando nros invertidos
    }
  }
  
  /*
  for (i = 0; i < N_ARRAYS; i++){
  	printf("Array %d : [", i);
    for(j = 0; j < ARRAY_SIZE; j++){
      printf("%d ", bag_of_tasks[i][j]);
    }
    printf("]\n");
  }*/
  
  for (i = 0; i < N_ARRAYS; i++){
  	qsort(bag_of_tasks[i], ARRAY_SIZE, sizeof(int), cmpfunc);// ... trabalha...  
	}

	/*
  for (i = 0; i < N_ARRAYS; i++){
  	printf("Array %d : [", i);
    for(j=0; j < ARRAY_SIZE; j++){
      printf("%d ", bag_of_tasks [i][j]);
    }
    printf("]\n");
  }*/

  free(bag_of_tasks);
  clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[Sequential] Duration [%f]\n", time_spent);
}