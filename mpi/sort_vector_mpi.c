#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
   char hostname[20];
   
   FILE *host;

    if((host = fopen("/etc/hostname","r")) == NULL)
    {
        printf("\nNao consigo abrir o arquivo ! ");
        exit(1);
    }

   fscanf (host, "%s", hostname);

   int myrank, //who am i
       numprocs; //how many process
   
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
   MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
   printf("Hostname: \"%s\" Rank: \"%d\" Size: \"%d\"\n",hostname,myrank,numprocs);
   MPI_Finalize();
   return 0;
}
