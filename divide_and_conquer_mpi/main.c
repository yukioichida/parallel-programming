#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELTA       10 // Quantidade mínima para iniciar o passo de conquista
#define ROOT        0    // id da raiz
#define ARRAY_SIZE  40

int *interleaving(int vetor[], int tam){
  int *vetor_auxiliar;
  int i1, i2, i_aux;
  vetor_auxiliar = (int *)malloc(sizeof(int) * tam);
  i1 = 0;
  i2 = tam / 2;
  for (i_aux = 0; i_aux < tam; i_aux++) {
    if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2))) || (i2 == tam))
      vetor_auxiliar[i_aux] = vetor[i1++];
    else
      vetor_auxiliar[i_aux] = vetor[i2++];
  }
  return vetor_auxiliar;
}

void bs(int n, int * vetor){
  int c=0, d, troca, trocou =1;
  while (c < (n-1) & trocou ) {
    trocou = 0;
    for (d = 0 ; d < n - c - 1; d++){
      if (vetor[d] > vetor[d+1]) {
        troca      = vetor[d];
        vetor[d]   = vetor[d+1];
        vetor[d+1] = troca;
        trocou = 1;
      }
      c++;
    }
  }
}


/* Método onde é o escravo que requisita a tarefa */
int main(int argc,char **argv){

  int n_tasks, task_id, exit_code, i, j;
  double t1, t2;  // tempos para medição de duração de execuções
  MPI_Status mpi_status;
  int *array;

  exit_code = MPI_Init(&argc,&argv);
  exit_code|= MPI_Comm_size(MPI_COMM_WORLD,&n_tasks);
  exit_code|= MPI_Comm_rank(MPI_COMM_WORLD,&task_id);

  if (exit_code != MPI_SUCCESS) {
    printf ("Error initializing MPI and obtaining task ID information\n");
    return 1;
  }

  if (task_id == FIRST){
    t1 = MPI_Wtime();
    array = (int *) malloc(sizeof(int) * ARRAY_SIZE);
    /* popula o vetor totalmente invertido */
    for (int i = 0; i < ARRAY_SIZE; i++){
      array[i] = ARRAY_SIZE-i; 
    }
    t2 = MPI_Wtime();
    printf("[] Duration [%f]\n", task_id, t2-t1);
  } else {

    t1 = MPI_Wtime();
    // ================ SLAVE ===================

    t2 = MPI_Wtime(); 
    printf("[WORKER %d] Duration [%f] - Tasks [%d]\n", task_id, t2-t1, task_executed);
  }
  MPI_Finalize();

}