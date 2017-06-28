#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROOT 0    // pid of first process
#define ORIGINAL_ARRAY_SIZE  100
#define MAIN_TAG 1
#define PRINT 1

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

int main(int argc,char *argv[]){

  int n_process, task_id, exit_code, i, j, process_left, process_right, half_vector, parent_process, tree_height, delta;
  int array_size = ORIGINAL_ARRAY_SIZE;
  double t1, t2;  // tempos para medição de duração de execuções
  MPI_Status mpi_status;
  int *array = (int *) malloc((ORIGINAL_ARRAY_SIZE) * sizeof *array);

  exit_code = MPI_Init(&argc,&argv);
  exit_code|= MPI_Comm_size(MPI_COMM_WORLD,&n_process);
  exit_code|= MPI_Comm_rank(MPI_COMM_WORLD,&task_id);
  if (exit_code != MPI_SUCCESS) {
    printf ("Error initializing MPI and obtaining task ID information\n");
    return 1;
  }
  t1 = MPI_Wtime();

  /* Calculando a altura da árvore e o DELTA */
  tree_height = floor(log(n_process) / log(2))+1;
  if (tree_height > 1) // Avoiding division by zero
    delta = ORIGINAL_ARRAY_SIZE / (pow(2, tree_height-1));
  else{
    printf("[ERROR] Should have at least two process to using parallel version.\n");
    return 1;
  }
  /* Verifica se aloca o vetor ou se recebe o vetor do processo superior*/
  if (task_id == ROOT){
    array_size = ORIGINAL_ARRAY_SIZE;
    for (i = 0; i < array_size; i++) array[i] = array_size-i; 
    printf("Tree height %d - Delta %d\n", tree_height, delta);
  } else {
    MPI_Recv(array, array_size, MPI_INT, MPI_ANY_SOURCE, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    MPI_Get_count(&mpi_status, MPI_INT, &array_size);
    parent_process = mpi_status.MPI_SOURCE;
  }
  /* Divisão ou Conquista */
  if (array_size <= delta){
    bs(array_size, array);
  }else{
    process_left = (2*task_id) + 1;
    process_right = process_left + 1;
    half_vector = (array_size/2);
    /* Envia os vetores para os subprocessos */
    MPI_Send(&array[0], half_vector, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD);
    MPI_Send(&array[half_vector], half_vector, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD);
    /* Recebe os resultados dos subprocessos */
    MPI_Recv(&array[0], half_vector, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    MPI_Recv(&array[half_vector], half_vector, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    /* Ajusta as duas partes */
    int *fully_ordered_vector = interleaving(array, array_size);
    memcpy(array, fully_ordered_vector, array_size * sizeof(int));
    free(fully_ordered_vector);
  }

  if (task_id == ROOT){
    t2 = MPI_Wtime();
    #if PRINT == 1
    printf("Print enabled...\n");
    printf("Result vector: [");
    for (i = 0; i < ORIGINAL_ARRAY_SIZE; i++){
      printf("%d ", array[i]);
    }
    printf("]\n");
    #endif
  } else { 
    /* Envia para o processo superior*/
    MPI_Send(&array[0], array_size, MPI_INT, parent_process, MAIN_TAG, MPI_COMM_WORLD);
  }
  MPI_Finalize();
  free(array);  
}