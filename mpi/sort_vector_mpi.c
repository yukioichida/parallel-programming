#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ARRAY_SIZE 4 //Tamanho do array
#define N_ARRAYS  1 // Quantidade de arrays
#define MASTER    0    // id do mestre
#define POISON_PILL -2
#define FIRST_TASK -1
#define ARRAY_MSG 2 // tipo de mensagem que transmite um array

/* Função de que é usado pelo qsort */
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}

// função que remove o índice do vetor, que está na primeira posição
void remove_index(int *array, int array_length){
   int i;
   for(i = 0; i < array_length - 1; i++) array[i] = array[i + 1];
}

void add_index(int *array, int index, int array_length){
   int i;
   for(i = array_length-1; i > 0 - 1; i--) array[i] = array[i - 1];
   array[0] = index;
}


/* Método onde é o escravo que requisita a tarefa */
int main(int argc,char **argv){

  int n_tasks, task_id, exit_code, i, j, index, worker, array_to_send = 0, msg_size = (ARRAY_SIZE+1), task_executed = 0;
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
    int (*bag_of_tasks)[msg_size] = malloc (N_ARRAYS * sizeof *bag_of_tasks);        
    int (*results)[msg_size] = malloc (N_ARRAYS * sizeof *results);

    //buffer usado para transmissão de mensagem entre mestre e escravos
    int buffer[msg_size];

    // Define a primeira posição como identificador do array
    for (i=0; i < N_ARRAYS; i++){
      bag_of_tasks[i][0] = i;
    }

    // populando nros invertidos
    for (i = 0; i < N_ARRAYS; i++){
      for(j = 1; j <= ARRAY_SIZE; j++){
        //lembrando q a primeira posição já está ocupada pelo índice do vetor
        bag_of_tasks [i][j] = (ARRAY_SIZE-j)*(i+1); 
      }
    }

    // TODO: temporario
    /*for (i = 0; i < N_ARRAYS; i++){
      printf("[MASTER] Vetor %d [", i);
      for(j = 0; j <= ARRAY_SIZE; j++){
        printf("%d ", bag_of_tasks[i][j]);
      }
      printf("]\n");
    }*/


    // Delega enquanto tem arrays para receber
    while(array_to_send <= N_ARRAYS) { 
      MPI_Recv(&buffer, msg_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
      index = buffer[0];
      printf("[MASTER] Vetor no buffer recebido: [");
      for(j = 0; j <= ARRAY_SIZE; j++){
        printf("%d ", buffer[j]);
      }
      printf("]\n");
      if (index != FIRST_TASK){
        // Copia o resultado retornado do escravo pro saco de tarefas
        memcpy(results[index], buffer, msg_size * sizeof(int));
        printf("[MASTER] Resultado copiado: [");
        for(j = 0; j <= ARRAY_SIZE; j++){
          printf("%d ", results[index][j]);
        }
        printf("]\n");
      }

      printf("[MASTER] Vetor %d a ser enviando para %d: [", array_to_send, mpi_status.MPI_SOURCE);
      for(j = 0; j <= ARRAY_SIZE; j++){
        printf("%d ", bag_of_tasks[array_to_send][j]);
      }
      printf("]\n");
      // Verifica antes de enviar, pois o loop é executado sempre um passo a mais para recuperar a resposta dos escravos
      if (array_to_send < N_ARRAYS){
        MPI_Send(&bag_of_tasks[array_to_send], msg_size, MPI_INT, mpi_status.MPI_SOURCE, ARRAY_MSG, MPI_COMM_WORLD);        
      }
      array_to_send++;
    }

    index = POISON_PILL; // enviando POISON PILL para matar os escravos
    for (worker = 1; worker < n_tasks; worker++){
      MPI_Send(&index, 1, MPI_INT, worker, ARRAY_MSG, MPI_COMM_WORLD);
    }

    free(bag_of_tasks);
    t2 = MPI_Wtime(); 
    //printf("[Master] Duration [%f]\n", t2-t1);

    printf("Results:\n");
    // TODO: temporario
    for (i = 0; i < N_ARRAYS; i++){
      printf("[MASTER] result Vetor %d [", i);
      for(j = 0; j <= ARRAY_SIZE; j++){
        printf("%d ", results[i][j]);
      }
      printf("]\n");
    }
    
    free(results);
  } else {

    t1 = MPI_Wtime();
    // ================ SLAVE ===================
    int alive = 1;
    int worker_buffer[msg_size];
    worker_buffer[0] = FIRST_TASK;
    MPI_Send(&worker_buffer, msg_size, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD); // Ja pede uma tarefa de inicio
    do {
      MPI_Recv(&worker_buffer, msg_size, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD, &mpi_status);
      index = worker_buffer[0];
      if (index == POISON_PILL) {
        printf("[WORKER %d] Received POISON_PILL, ARGH!\n",task_id);
        alive = 0;
      } else {
        printf("[WORKER %d] Received vector %d!\n", task_id, index);
        // TODO: temporario
        printf("[WORKER %d] Vector Content: [", task_id);
        for(i = 0; i < msg_size; i++){
          printf("%d ", worker_buffer[i]);
        }
        printf("]\n");

        //remove_index(worker_buffer, msg_size);
        //qsort(worker_buffer, ARRAY_SIZE, sizeof(int), cmpfunc);// ... trabalha...
        //add_index(worker_buffer, index, msg_size);
        task_executed++;

        // TODO: temporario
        /*printf("[WORKER %d] Vector Content ordered: [", task_id);
        for(i = 0; i < msg_size; i++){
          printf("%d ", worker_buffer[i]);
        }
        printf("]\n");
*/
        MPI_Send(&worker_buffer, msg_size, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD);
      }
    } while (alive != 0);

    t2 = MPI_Wtime(); 
    //printf("[WORKER %d] Duration [%f] - Tasks [%d]\n", task_id, t2-t1, task_executed);

  }

  MPI_Finalize();

}