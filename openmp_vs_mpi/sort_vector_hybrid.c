#include "mpi.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 10 //Tamanho do array
#define N_ARRAYS  10 // Quantidade de arrays
#define WORKER_ARRAYS 10 // quantidade de arrays que o worker passa a receber
#define MASTER    0    // id do mestre
#define THREADS 4
#define POISON_PILL -2
#define FIRST_TASK -1
#define ARRAY_MSG 2 // tipo de mensagem que transmite um array

/* Função de que é usado pelo qsort */
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}

/* Método onde é o escravo que requisita a tarefa */
int main(int argc,char **argv){

  int n_tasks, task_id, array_per_thread, exit_code, i, j, k, index, worker, sent_arrays = 0, received_arrays = 0, task_executed = 0, sending = 1;
  int msg_size = (ARRAY_SIZE+1); // tamanho da mensagem trafegada entre os processos
  int index_pos = msg_size -1; // posição onde estará o índice do vetor
  double t1, t2;  // tempos para medição de duração de execuções
  MPI_Status mpi_status;

  /* tamanho do buffer trafegado entre worker e master */
  int buffer_size = msg_size * WORKER_ARRAYS;

  exit_code = MPI_Init(&argc,&argv);
  exit_code|= MPI_Comm_size(MPI_COMM_WORLD,&n_tasks);
  exit_code|= MPI_Comm_rank(MPI_COMM_WORLD,&task_id);

  if (exit_code != MPI_SUCCESS) {
    printf ("Error initializing MPI and obtaining task ID information\n");
    return 1;
  }



  if (task_id == MASTER){
    printf("[Master] Start process\n");
    int slave_recv[n_tasks]; // registrando os workers que realizaram a carga
    for(i = 0; i < n_tasks; i++){
        slave_recv[i] = 0;
    }
    // ====================== MESTRE ============================
    t1 = MPI_Wtime();
    // Aloca as matrizes, com a última posição reservada para o índice
    int (*bag_of_tasks)[msg_size] = malloc (N_ARRAYS * sizeof *bag_of_tasks);
    //buffer usado para transmissão de mensagem entre mestre e escravos
    int buffer[buffer_size];
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
      int (*send_buffer) = malloc (buffer_size * sizeof(int));
      k = 0;
      for(i=0; i < WORKER_ARRAYS; i++){
        for(j=0; j < msg_size; j++){
          send_buffer[k++] = bag_of_tasks[sent_arrays][j];
        }
        sent_arrays++;
      }
      MPI_Send(&buffer_size, buffer_size, MPI_INT, worker, ARRAY_MSG, MPI_COMM_WORLD);
      free(send_buffer);
      sent_arrays++;
    }

    printf("[MASTER] Start receiving arrays...\n");
    // Delega enquanto tem arrays para receber
    while(sending != 0) {
      // Probe for an incoming message from process zero
      MPI_Recv(&buffer, buffer_size, MPI_INT, MPI_ANY_SOURCE, ARRAY_MSG, MPI_COMM_WORLD, &mpi_status);
      /* Recebe as respostas */
      for (i=0; i < WORKER_ARRAYS; i++){
        k = 0;
        int receive_buffer[msg_size];
        for (j = 0; j < msg_size; j++){
          receive_buffer[j] = buffer[k++];
        }
        int index = receive_buffer[index_pos];
        memcpy(bag_of_tasks[index], receive_buffer, msg_size * sizeof(int));
        received_arrays++;  // registra o recebimento de um array
      }
      // Se o mestre já recebeu todos os vetores, então para o processo de envio
      if (received_arrays == N_ARRAYS){
        sending = 0;
      }else{
        // Verifica se já enviou todos
        if (sent_arrays < N_ARRAYS){
          int (*send_buffer) = malloc (buffer_size * sizeof(int));
          k = 0;
          for(i=0; i < WORKER_ARRAYS; i++){
            for(j=0; j < msg_size; j++){
              send_buffer[k++] = bag_of_tasks[sent_arrays][j];
            }
            sent_arrays++;
          }
          MPI_Send(&buffer_size, buffer_size, MPI_INT, mpi_status.MPI_SOURCE, ARRAY_MSG, MPI_COMM_WORLD);
          free(send_buffer);
        }
      }

      
    }
    printf("[MASTER] ending process\n");
    // enviando POISON PILL para matar os escravos
    buffer[index_pos] = POISON_PILL; 
    for (worker = 1; worker < n_tasks; worker++){
      MPI_Send(&buffer, buffer_size, MPI_INT, worker, ARRAY_MSG, MPI_COMM_WORLD);
    }

    printf("====================\n");
    for (i = 0; i < N_ARRAYS; i++){
      printf("ORDERED - Vector number %d[", i);
      for(j = 0; j < ARRAY_SIZE; j++){
        printf("%d ", bag_of_tasks[i][j]);
      }
      printf("]\n");
    }

    t2 = MPI_Wtime(); 
    printf("[Master] Duration [%f]\n", t2-t1);
    free(bag_of_tasks);
  

  } else {


    // ================ SLAVE ===================
    t1 = MPI_Wtime();
    int alive = 1;
    //int worker_buffer[msg_size];
    int (*worker_buffer) = malloc (buffer_size * sizeof(int));
    do {
      MPI_Recv(worker_buffer, buffer_size, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD, &mpi_status);
      index = worker_buffer[index_pos];
      if (index == POISON_PILL) {
        printf("[Worker %d] Ending process\n", task_id);
        alive = 0;
      } else {
        // lembrando que a última posição é o índice, ou seja, não deve ser usado no algoritmo de ordenação
        i = 0;
        while (i < buffer_size){
          qsort(&worker_buffer[i], (msg_size-1), sizeof(int), cmpfunc);// ... trabalha...        
          i += msg_size;
        }

        task_executed++;
        MPI_Send(worker_buffer, buffer_size, MPI_INT, MASTER, ARRAY_MSG, MPI_COMM_WORLD);
      }
    } while (alive != 0);

    t2 = MPI_Wtime(); 
  }
  MPI_Finalize();
}
