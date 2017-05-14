#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROOT 0    // pid of first process
#define ORIGINAL_ARRAY_SIZE  1000000
#define MAIN_TAG 1

int *interleaving(int vetor[], int tam, int offset1, int offset2, int offset3){
  int *vetor_auxiliar;
  int i1, i2, i3, i_aux;
  vetor_auxiliar = (int *)malloc(sizeof(int) * tam);
  i1 = offset1;
  i2 = offset2;
  i3 = offset3;

  for (i_aux = 0; i_aux < tam; i_aux++) {
    // Using i1 if is lesser than others or all elements of i2 and i3 are on the vector
    if (((vetor[i1] <= vetor[i2]) && (i1 < i2) && (vetor[i1] <= vetor[i3])) || ((i2 == offset3) && (i3 == tam))){
      //printf("Fluxo1\n");
      vetor_auxiliar[i_aux] = vetor[i1++];
    }else if (((vetor[i2] <= vetor[i1]) && (i2 < i3) && (vetor[i2] <= vetor[i3])) || ((i1 == offset2) && (i3 == tam))){
      //printf("Fluxo2\n");
      vetor_auxiliar[i_aux] = vetor[i2++];
    }else
      if (i3 != tam){
        //printf("Fluxo3\n");
        vetor_auxiliar[i_aux] = vetor[i3++];
      }else{
        //printf("Fluxo4\n");
        //original interleaving, comparing offset 1 and 2 only
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

  tree_height = floor(log(n_process) / log(2))+1;
  if (n_process < 1) {
    printf("[ERROR] Should have at least two process to using parallel version.\n");
    return 1;
  }

  // each process will order a local part of array...
  delta = (ORIGINAL_ARRAY_SIZE / n_process) + 1;
  // if delta is very low, then the processes can conquer very early, leaving sub processes with no array to process
  if (delta < 5){
    printf("Invalid number of process for array_size %d\n", ORIGINAL_ARRAY_SIZE);
    return 1;
  }

  if (exit_code != MPI_SUCCESS) {
    printf ("Error initializing MPI and obtaining task ID information\n");
    return 1;
  }

  if (task_id == ROOT){
    /* Allocates the vector */
    array_size = ORIGINAL_ARRAY_SIZE;
    /* Populate the vector with inverted values */
    for (i = 0; i < array_size; i++) array[i] = array_size-i; 
    //printf("Tree height %d - Delta %d\n", tree_height, delta);
  } else {
    MPI_Recv(array, array_size, MPI_INT, MPI_ANY_SOURCE, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    MPI_Get_count(&mpi_status, MPI_INT, &array_size);
    parent_process = mpi_status.MPI_SOURCE;
    //printf("[Process %d] Received %d elements from process %d\n", task_id, array_size, parent_process);
    /*printf("[Process %d] Received vector: [", task_id);
    for (i = 0; i < array_size; i++)
      printf("%d ", array[i]);
    printf("]\n");
    */
  }

  if (array_size <= delta){
    bs(array_size, array); //conquer
    //printf("[Process %d] Conquer...\n", task_id);
  }else{
    process_left = (2*task_id) + 1;
    process_right = process_left + 1;
    int reduced_size = (array_size/3);
    /* offset2 is the second 1/3 part, offset3 is the third 1/3 part and limit is the remaining positions */
    int offset2_start = reduced_size, offset3_start = (2*reduced_size); // offsets start position
    int offset2_size = (offset3_start - offset2_start), offset3_size = (array_size - offset3_start); // offsets end

    /* Order the first 1/3 of vector */
    bs(reduced_size, array);

    /*printf("[Process %d] Local Ordered vector: [", task_id);
    for (i = 0; i < reduced_size; i++)
      printf("%d ", array[i]);
    printf("]\n");
    /* send others 1/3 vectors to sub processes */
    MPI_Send(&array[offset2_start], offset2_size, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD);
    MPI_Send(&array[offset3_start], offset3_size, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD);
    /* receive vectors from sub processes  */
    MPI_Recv(&array[offset2_start], offset2_size, MPI_INT, process_left, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);
    MPI_Recv(&array[offset3_start], offset3_size, MPI_INT, process_right, MAIN_TAG, MPI_COMM_WORLD, &mpi_status);

    int *fully_ordered_vector = interleaving(array, array_size, 0, offset2_start, offset3_start);
    /* copy fully ordered vector to process array */
    memcpy(array, fully_ordered_vector, array_size * sizeof(int));
    free(fully_ordered_vector);
  }

  if (task_id == ROOT){ /* The task has been finished */
    /*
    printf("Ordered vector: [");
    for (i = 0; i < ORIGINAL_ARRAY_SIZE; i++)
      printf("%d ", array[i]);
    printf("]\n");
    */

    t2 = MPI_Wtime();
    printf("[Process \t%d\t] Duration = \t %f\n", task_id, (t2-t1));
  } else { 
    /* Send vector to parent process */
    MPI_Send(&array[0], array_size, MPI_INT, parent_process, MAIN_TAG, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  free(array);

  //printf("[Process \t%d\t] Duration = \t %f\n", task_id, (t2-t1));
}
