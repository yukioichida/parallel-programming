#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 100000 //Tamanho do array
#define N_ARRAYS  10 // Quantidade de arrays
#define DEFAULT_THREADS 4

/* Função de que é usado pelo qsort */
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}

/* Método onde é o escravo que requisita a tarefa */
int main(int argc,char **argv){
  int threads, task, thread_id, i, j, n = ARRAY_SIZE * N_ARRAYS;
  double t1, t2;  // tempos para medição de duração de execuções

  if (argc == 1){
    threads = DEFAULT_THREADS; //default
  }else if (argc == 2){
    threads = atoi(argv[1]);
  } else {
    printf("Usage: %s number_of_threads.\n", argv[0]);
    return 0;
  }
  printf("Run with %d threads...\n", threads);
  t1 = omp_get_wtime(); 

  /* Bag of tasks */
  int (*bag_of_tasks)[ARRAY_SIZE] = malloc (N_ARRAYS * sizeof *bag_of_tasks);

  for (i = 0; i < N_ARRAYS; i++){
    for(j = 0; j < ARRAY_SIZE; j++){ //última posição reservada
      bag_of_tasks [i][j] = n; 
      n--;
    }
  }

  omp_set_num_threads(threads);
  #pragma omp parallel private (task, i)
  #pragma omp for schedule (dynamic)
  for (task = 0; task < N_ARRAYS; task++){
    thread_id = omp_get_thread_num();
    //printf("[Thread %d] Ordering vector %d\n", thread_id, task);
    qsort(bag_of_tasks[task], ARRAY_SIZE, sizeof(int), cmpfunc);// ... trabalha...
  }

  t2 = omp_get_wtime(); 

  printf("Tempo de execução: %3.2f segundos \n", t2-t1);

  /*
  for(i = 0; i < N_ARRAYS; i++){
    printf("ORDERED - Vector number %d: [", i);
    for(j = 0; j < ARRAY_SIZE; j++){
      printf("%d ", bag_of_tasks[i][j]);
    }
    printf("]\n");
  }
  */

  free(bag_of_tasks);
}
