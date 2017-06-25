#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 10 //Tamanho do array
#define N_ARRAYS  10 // Quantidade de arrays
#define MASTER    0    // id do mestre
#define POISON_PILL -2
#define FIRST_TASK -1
#define ARRAY_MSG 2 // tipo de mensagem que transmite um array

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

  int n_tasks, task_id, array_per_thread, exit_code, i, j, k, index, worker, sent_arrays = 0, received_arrays = 0, task_executed = 0, sending = 1;
  int msg_size = (ARRAY_SIZE+1); // tamanho da mensagem trafegada entre os processos
  int index_pos = msg_size -1; // posição onde estará o índice do vetor
  double t1, t2;  // tempos para medição de duração de execuções
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
    // Aloca as matrizes, com a última posição reservada para o índice
    int (*bag_of_tasks)[msg_size] = malloc (N_ARRAYS * sizeof *bag_of_tasks);
    //buffer usado para transmissão de mensagem entre mestre e escravos
    int buffer[msg_size];
    // Define a última posição como identificador do array
    for (i=0; i < N_ARRAYS; i++){
      bag_of_tasks[i][index_pos] = i;
    }
    // populando nros invertidos
    for (i = 0; i < N_ARRAYS; i++){
      for(j = 0; j < (msg_size-1); j++){ //última posição reservada
        bag_of_tasks [i][j] = (ARRAY_SIZE-j)*(i+1); 
      }
    }

    /* Ao iniciar já envia para os workers as tarefas */
    for (worker = 1; worker < n_tasks; worker++){
      if (sent_arrays < N_ARRAYS){
        MPI_Send(&bag_of_tasks[sent_arrays], msg_size, MPI_INT, worker, ARRAY_MSG, MPI_COMM_WORLD);
        sent_arrays++;
      }      
    }

    // Delega enquanto tem arrays para receber
    while(sending != 0) {
      MPI_Recv(&buffer, msg_size, MPI_INT, MPI_ANY_SOURCE, ARRAY_MSG, MPI_COMM_WORLD, &mpi_status);
      int index = buffer[index_pos];
      memcpy(bag_of_tasks[index], buffer, msg_size * sizeof(int));
      received_arrays++;  // registra o recebimento de um array
      
      // Se o mestre já recebeu todos os vetores, então para o processo de envio
      if (received_arrays == N_ARRAYS){
        sending = 0;
      }else{
        // Verifica se já enviou todos
        if (sent_arrays < N_ARRAYS){
          MPI_Send(&bag_of_tasks[sent_arrays], msg_size, MPI_INT, mpi_status.MPI_SOURCE, ARRAY_MSG, MPI_COMM_WORLD);
          sent_arrays++;
        }
      }      
    }

    // encerrando os processos
    buffer[index_pos] = POISON_PILL; 
    for (worker = 1; worker < n_tasks; worker++){
      MPI_Send(&buffer, msg_size, MPI_INT, worker, ARRAY_MSG, MPI_COMM_WORLD);
    }

    t2 = MPI_Wtime(); 
    printf("[Master] Duration [%f]\n", t2-t1);
    free(bag_of_tasks);

  } else {

    // ================ SLAVE ===================
    int alive = 1;
    int (*worker_buffer) = malloc (msg_size * sizeof(int));
    do {
      MPI_Recv(worker_buffer, msg_size, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD, &mpi_status);
      index = worker_buffer[index_pos];
      if (index == POISON_PILL) {
        alive = 0;
      } else {
        bs(msg_size-1, &worker_buffer[0]);
        MPI_Send(worker_buffer, msg_size, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD);
      }
    } while (alive != 0);

    free(worker_buffer);
  }
  MPI_Finalize();
}
