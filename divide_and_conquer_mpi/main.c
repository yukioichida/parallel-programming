#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELTA 10 // delta value
#define ROOT 0    // pid of first process
#define ORIGINAL_ARRAY_SIZE  20
#define MAIN_TAG 1

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


int main(int argc,char **argv){

  int n_tasks, task_id, exit_code, i, j, process_left, process_right, half_vector, parent_process;
  int array_size = ORIGINAL_ARRAY_SIZE;
  MPI_Status mpi_status;
  int *array = (int *) malloc((ORIGINAL_ARRAY_SIZE) * sizeof *array);

  exit_code = MPI_Init(&argc,&argv);
  exit_code|= MPI_Comm_size(MPI_COMM_WORLD,&n_tasks);
  exit_code|= MPI_Comm_rank(MPI_COMM_WORLD,&task_id);

  if (exit_code != MPI_SUCCESS) {
    printf ("Error initializing MPI and obtaining task ID information\n");
    return 1;
  }

  if (task_id == ROOT){
    /* Allocates the vector */
    array_size = ORIGINAL_ARRAY_SIZE;
    /* Populate the vector with inverted values */
    for (i = 0; i < array_size; i++) array[i] = array_size-i; 
  } else {
    MPI_Recv(array, array_size, MPI_INT, MPI_ANY_SOURCE, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    
    MPI_Get_count(&mpi_status, MPI_INT, &array_size);


    parent_process = mpi_status.MPI_SOURCE;
    printf("[Process %d] Received %d elements from process %d\n", task_id, array_size, parent_process);
  }

  if (array_size <= DELTA){
    bs(array_size, array);
  }else{
    process_left = (2*task_id) + 1;
    process_right = process_left + 1;
    half_vector = (array_size/2);
    /* send vectors to sub processes */
    MPI_Send(&array[0], half_vector, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD);
    MPI_Send(&array[half_vector], half_vector, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD);
    /* receive vectors from sub processes  */
    MPI_Recv(&array[0], half_vector, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    MPI_Recv(&array[half_vector], half_vector, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);

    int *fully_ordered_vector = interleaving(array, array_size);
    /* copy fully ordered vector to process array */
    memcpy(array, fully_ordered_vector, array_size * sizeof(int));
    free(fully_ordered_vector);
  }

  
  if (task_id == ROOT){
    /* The task has been finished */
    printf("Vetor ordenado: [\n");
    for (i = 0; i < ORIGINAL_ARRAY_SIZE; i++)
      printf("%d ", array[i]);
    printf("]\n");
  } else { 
    /* Send vector  */
    MPI_Send(&array[0], array_size, MPI_INT, parent_process, MAIN_TAG, MPI_COMM_WORLD);
  }
  printf("[Process %d]  Termina o processo...\n", task_id);

  MPI_Finalize();
  free(array);

}