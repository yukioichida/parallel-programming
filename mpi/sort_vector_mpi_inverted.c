#include "mpi.h"
#include <stdio.h>

#define ARRAYSIZE 100000 //Tamanho do array
#define N_ARRAYS  1000 // Quantidade de arrays
#define MASTER    0    // id do mestre
#define POISON_PILL 1

int main(int argc,char **argv){

    int n_tasks, n_workers, task_id, exit_code, i, j, dest, task_type;
    MPI_Status mpi_status;

    exit_code = 1;

    exit_code = MPI_Init(&argc,&argv);
    exit_code|= MPI_Comm_size(MPI_COMM_WORLD,&n_tasks);
    exit_code|= MPI_Comm_rank(MPI_COMM_WORLD,&task_id);

    if (exit_code != MPI_SUCCESS) {
        printf ("Error initializing MPI and obtaining task ID information\n");
    } else {
        //printf ("MPI task ID = %d\n", task_id);
    }

    n_workers = n_tasks -1; // quantidade de slaves

    //printf("%d tasks, I am task %d\n", n_tasks, task_id);

    if (task_id == MASTER){        
        /* APENAS PARA TESTE - alocação estática */
        /* Inicializa o saco de tarefas */
        int bag_of_tasks[3][2]; // 3 tarefas com 2 números 
        for (i = 0; i < 3; i++){
            for(j=0; j<2; j++){
                bag_of_tasks [i][j] = j+i;
            }
        }

        /* lembrando que 0 é o mestre - este modelo é o mestre que envia as tarefas, inverter a lógica*/
        task_type = 0;
        i = 0;
        while(i < 3) {
            for (dest = 1; dest <= n_workers; dest++){
                if (i < 3){
                    printf("[MASTER] - Sending vector %d to slave %d\n", i, dest);
                    MPI_Send(&task_type, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
                    MPI_Send(&bag_of_tasks[i], 2, MPI_INT, dest, 1, MPI_COMM_WORLD);
                    i++; // o prox slave pega a tarefa posterior
                }
            }
        }

        /* Mandando POISON_PILL para os escravos pararem */
        task_type = POISON_PILL;
        for (dest = 1; dest <= n_workers; dest++){
            printf("[MASTER] - Sending poison pill to worker %d.\n", dest);
            MPI_Send(&task_type, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
        }

        // recebe os vetores, 
        // usando o recebimento como garantia de término da tarefa, enviar mais tarefas
    } else{
        int alive = 1;
        int task_type = 0;
        int vector[2];

        while(alive != 0){
            int source = MASTER;
            MPI_Recv(&task_type, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &mpi_status);
            if (task_type == POISON_PILL){
                printf("[Worker %d] - Receiving poison pill, ARGH!\n", task_id);
                alive = 0;
            }else{
                MPI_Recv(&vector, 2, MPI_INT, source, 1, MPI_COMM_WORLD, &mpi_status);
                printf("[Worker %d] - Received vector: [", task_id);
                for (i = 0; i < 2; i++){
                    printf("%d ", vector[i]);
                }
                printf("]\n");
            }
        }
    }


    MPI_Finalize();

}
