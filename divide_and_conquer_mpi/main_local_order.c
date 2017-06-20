#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROOT 0    // pid of first process
#define ORIGINAL_ARRAY_SIZE  40
#define MAIN_TAG 1

int *interleaving(int vetor[], int tam, int offset1, int offset2, int offset3){
  int *vetor_auxiliar;
  int i1, i2, i3, i_aux;
  vetor_auxiliar = (int *)malloc(sizeof(int) * tam);
  i1 = offset1;
  i2 = offset2;
  i3 = offset3;
  for (i_aux = 0; i_aux < tam; i_aux++) {
    if (((vetor[i1] <= vetor[i2]) && (i1 < i2) && (vetor[i1] <= vetor[i3])) || ((i2 == offset3) && (i3 == tam))){
      vetor_auxiliar[i_aux] = vetor[i1++];
    }else if (((vetor[i2] <= vetor[i1]) && (i2 < i3) && (vetor[i2] <= vetor[i3])) || ((i1 == offset2) && (i3 == tam))){
      vetor_auxiliar[i_aux] = vetor[i2++];
    }else
      if (i3 != tam){
        vetor_auxiliar[i_aux] = vetor[i3++];
      }else{
        if (((vetor[i1] <= vetor[i2]) && (i1 < i2)))
          vetor_auxiliar[i_aux] = vetor[i1++];
        else 
          vetor_auxiliar[i_aux] = vetor[i2++];
      }
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

  int n_process, task_id, exit_code, i, j, process_left, process_right, half_vector, parent_process, tree_height, delta, min_delta;
  int array_size = ORIGINAL_ARRAY_SIZE;
  double t1, t2;  // tempos para medição de duração de execuções
  MPI_Status mpi_status;
  int *array = (int *) malloc((ORIGINAL_ARRAY_SIZE) * sizeof *array);

  exit_code = MPI_Init(&argc,&argv);
  exit_code|= MPI_Comm_size(MPI_COMM_WORLD,&n_process);
  exit_code|= MPI_Comm_rank(MPI_COMM_WORLD,&task_id);
  t1 = MPI_Wtime();

  /* Delta vai ser do tamanho da porção que cada processo irá receber */
  delta = (ORIGINAL_ARRAY_SIZE / n_process) + 1;
  if (exit_code != MPI_SUCCESS) {
    printf ("Error initializing MPI and obtaining task ID information\n");
    return 1;
  }

  if (task_id == ROOT){
    array_size = ORIGINAL_ARRAY_SIZE;
    for (i = 0; i < array_size; i++) array[i] = array_size-i;
  } else {
    MPI_Recv(array, array_size, MPI_INT, MPI_ANY_SOURCE, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    MPI_Get_count(&mpi_status, MPI_INT, &array_size);
    parent_process = mpi_status.MPI_SOURCE;
  }
  /* Divide ou conquista */
  if (array_size <= delta){
    bs(array_size, array);
  }else{
    process_left = (2*task_id) + 1;
    process_right = process_left + 1;
    /* Separa o vetor em 3 partes */
    int local_part = delta;
    int remaining =  array_size - local_part;
    /* offset2 é a segunda parte, offset3 é a terceiraparte e limit é o restante do vetor */
    int offset2_start = local_part;
    int offset3_start = delta + (remaining/2);
    int offset2_size = (offset3_start - offset2_start);
    int offset3_size = (array_size - offset3_start); 

    /* send others 1/3 vectors to sub processes */
    MPI_Send(&array[offset2_start], offset2_size, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD);
    MPI_Send(&array[offset3_start], offset3_size, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD);

    /* Order the local vector, the first offset, while the subprocesses works */
    bs(delta, array);
    /* receive vectors from sub processes  */
    MPI_Recv(&array[offset2_start], offset2_size, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    MPI_Recv(&array[offset3_start], offset3_size, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);

    int *fully_ordered_vector = interleaving(array, array_size, 0, offset2_start, offset3_start);
    /* copy fully ordered vector to process array */
    memcpy(array, fully_ordered_vector, array_size * sizeof(int));
    free(fully_ordered_vector);
  }

  if (task_id == ROOT){ 
    t2 = MPI_Wtime();
    printf("[Process \t%d\t] Duration = \t %f\n", task_id, (t2-t1));
  } else { 
    /* Send vector to parent process */
    MPI_Send(&array[0], array_size, MPI_INT, parent_process, MAIN_TAG, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  free(array);
}
