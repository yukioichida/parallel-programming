#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 3 //Tamanho do array
#define N_ARRAYS  4 // Quantidade de arrays

/* Função de que é usado pelo qsort */
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}

int main(int argc,char **argv){
	// Aloca as matrizes
    int (*bag_of_tasks)[N_ARRAYS] = malloc (ARRAY_SIZE * sizeof *bag_of_tasks);

    for (i = 0; i < N_ARRAYS; i++){
      for(j=0; j < ARRAY_SIZE; j++){
        bag_of_tasks [i][j] = (ARRAY_SIZE-j-1); // populando nros invertidos
      }
    }

    qsort(array, ARRAY_SIZE, sizeof(int), cmpfunc);// ... trabalha...  
    
    free(bag_of_tasks);
    free(results);
}