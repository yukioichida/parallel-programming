#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 9600 //Tamanho do array
#define N_ARRAYS  960 // Quantidade de arrays

/* Função de que é usado pelo qsort */
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}

void bs(int n, int * vetor){
  int c=0, d, troca, trocou =1;
  while (c < (n-1) & trocou ) {
    trocou = 0;
    for (d = 0 ; d < n - c - 1; d++)
      if (vetor[d] > vetor[d+1]) {
          troca = vetor[d];
          vetor[d] = vetor[d+1];
          vetor[d+1] = troca;
          trocou = 1;
      }
      c++;
  }
}


int main(int argc,char **argv){
	clock_t begin = clock();

	int i,j;
  // Aloca as matrizes, com a última posição reservada para o índice
    int (*bag_of_tasks)[ARRAY_SIZE] = malloc (N_ARRAYS * sizeof *bag_of_tasks);
    // Define a última posição como identificador do array
    // populando nros invertidos
    for (i = 0; i < N_ARRAYS; i++){
      for(j = 0; j < ARRAY_SIZE; j++){ //última posição reservada
        bag_of_tasks [i][j] = (ARRAY_SIZE-j); 
      }
    }
/*
    for(i = 0; i < N_ARRAYS; i++){
    	printf("Vectores DESORDENADOS %d [", i);
		for(j = 0; j < ARRAY_SIZE; j++){
			printf("%d ", bag_of_tasks[i][j]);
		}
		printf("]\n");
    }
*/
    for (i=0; i < N_ARRAYS; i++){
      bs(ARRAY_SIZE, &bag_of_tasks[i][0]);
    }
/*
    for(i = 0; i < N_ARRAYS; i++){
    	printf("Vectores ordenados %d [", i);
		for(j = 0; j < ARRAY_SIZE; j++){
			printf("%d ", bag_of_tasks[i][j]);
		}
		printf("]\n");
    }
*/
    free(bag_of_tasks);

    clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Duration: %3.2f\n", time_spent);
}
