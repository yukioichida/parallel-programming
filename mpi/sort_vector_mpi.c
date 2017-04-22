#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 100000 //Tamanho do array
#define N_ARRAYS  1000 // Quantidade de arrays
#define MASTER    0    // id do mestre
#define POISON_PILL -2
#define FIRST_TASK -1
#define ARRAY_MSG 2 // tipo de mensagem que transmite um array
#define INDEX_MSG 1 // tipo de mensagem que transmite um índice

/* Função de que é usado pelo qsort */
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}
/* Método onde é o escravo que requisita a tarefa */
int main(int argc,char **argv){

  int n_tasks, task_id, exit_code, i, j, index, worker, array_to_send = 0, recv_arrays = 0;
  double t1, t2;  
  MPI_Status mpi_status;
  exit_code = MPI_Init(&argc,&argv);
  exit_code|= MPI_Comm_size(MPI_COMM_WORLD,&n_tasks);
  exit_code|= MPI_Comm_rank(MPI_COMM_WORLD,&task_id);

  if (exit_code != MPI_SUCCESS) {
    printf ("Error initializing MPI and obtaining task ID information\n");
    return 1;
  }

  if (task_id == MASTER){
    // ====================== MESTRE ============================
    t1 = MPI_Wtime();
    // Aloca as matrizes
    int (*bag_of_tasks)[ARRAY_SIZE] = malloc (N_ARRAYS * sizeof *bag_of_tasks);        
    int (*results)[ARRAY_SIZE] = malloc (N_ARRAYS * sizeof *results);

    // populando nros invertidos
    for (i = 0; i < N_ARRAYS; i++){
      for(j=0; j < ARRAY_SIZE; j++){
        bag_of_tasks [i][j] = (ARRAY_SIZE-j-1); 
      }
    }

    // Delega enquanto tem arrays para receber
    while(recv_arrays < N_ARRAYS) { 
      for (worker = 1; worker < n_tasks; worker++){  // Recebe as requisições de trabalho...
        if (recv_arrays < N_ARRAYS) {
          MPI_Recv(&index, 1, MPI_INT, worker, INDEX_MSG, MPI_COMM_WORLD, &mpi_status);
          if (index != FIRST_TASK){
              MPI_Recv(&results[index], ARRAY_SIZE, MPI_INT, worker, ARRAY_MSG, MPI_COMM_WORLD, &mpi_status);    
              recv_arrays++;
          }
        }
      }
      // ... e envia mais vetores
      for(worker = 1; worker < n_tasks; worker++){ 
        if (array_to_send < N_ARRAYS){
          MPI_Send(&array_to_send, 1, MPI_INT, worker, INDEX_MSG, MPI_COMM_WORLD);
          MPI_Send(&bag_of_tasks[array_to_send], ARRAY_SIZE, MPI_INT, worker, ARRAY_MSG, MPI_COMM_WORLD);
          array_to_send++;
        }
      }           
    }

    index = POISON_PILL; // enviando POISON PILL para matar os escravos
    for (worker = 1; worker < n_tasks; worker++){
      MPI_Send(&index, 1, MPI_INT, worker, INDEX_MSG, MPI_COMM_WORLD);
    }

    free(bag_of_tasks);
    free(results);
    t2 = MPI_Wtime(); 
    printf("[Master] Duration [%f]\n", t2-t1);

  } else {

    t1 = MPI_Wtime();
    // ================ SLAVE ===================
    int alive = 1;
    int array[ARRAY_SIZE];
    int index = FIRST_TASK;

    MPI_Send(&index, 1, MPI_INT, MASTER, INDEX_MSG, MPI_COMM_WORLD); // Ja pede uma tarefa de inicio
    do {
      MPI_Recv(&index, 1, MPI_INT, MASTER, 1, MPI_COMM_WORLD, &mpi_status);
      if (index == POISON_PILL) {
        //printf("[WORKER %d] Received POISON_PILL, ARGH!\n",task_id);
        alive = 0;
      } else {
        //printf("[WORKER %d] Received vector %d!\n", task_id, index);
        MPI_Recv(&array, ARRAY_SIZE, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD, &mpi_status);// O escravo recebe seu vetor, ...
        qsort(array, ARRAY_SIZE, sizeof(int), cmpfunc);// ... trabalha...                
        MPI_Send(&index, 1, MPI_INT, MASTER, INDEX_MSG, MPI_COMM_WORLD); // ... e envia para o mestre
        MPI_Send(&array, ARRAY_SIZE, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD);
      }
    } while (alive != 0);

    t2 = MPI_Wtime(); 
    printf("[Worker %d] Duration [%f]\n", task_id, t2-t1);

  }

  MPI_Finalize();

}