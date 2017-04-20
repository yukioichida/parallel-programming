#include "mpi.h"
#include <stdio.h>

#define ARRAYSIZE 60000
#define  MASTER   0         /* taskid of first process */

int main(int argc,char **argv)
{
   int  ntasks,           /* total number of MPI tasks in partitiion */
         nworkers,         /* number of worker tasks */
        taskid,          /* task identifier */
        rc,               /* return error code */
        dest,             /* destination task id to send message */
         index,            /* index into the array */
        i,                /* loop variable */
         arraymsg = 1,     /* setting a message type */
       indexmsg = 2,     /* setting a message type */
        source,           /* origin task id of message */
         chunksize;        /* for partitioning the array */

float data[ARRAYSIZE],     /* the intial array */
      result[ARRAYSIZE];    /* for holding results of array operations */
MPI_Status status;


    rc = MPI_Init(&argc,&argv);
    rc|= MPI_Comm_size(MPI_COMM_WORLD,&ntasks);
    rc|= MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    if (rc != MPI_SUCCESS)
       printf ("Error initializing MPI and obtaining task ID information\n");
    else
       printf ("MPI task ID = %d\n", taskid);
    printf("%d tasks, I am task %d\n", ntasks, taskid);

    nworkers = ntasks-1;
    printf("\nDefining chunksize with works %d \n",nworkers);
    chunksize = (ARRAYSIZE / nworkers);
    printf("\nChunksize defined\n");

/**************************** master task ************************************/
    if (taskid == MASTER)
    {  printf("MASTER: number of worker tasks will be= %d\n",nworkers);

       for(i=0; i<ARRAYSIZE; i++) 
          data[i] =  0.0;
       index = 0;

      /* Send each worker task its portion of the array */
      for (dest=1; dest<= nworkers; dest++)
      {   printf("Sending to worker task %d\n",dest);

          //MPI_Send(buffer,count,type,dest,tag,comm)
          MPI_Send(&index, 1, MPI_INT, dest, indexmsg, MPI_COMM_WORLD);
          MPI_Send(&data[index], chunksize, MPI_FLOAT, dest, arraymsg, MPI_COMM_WORLD);
          index = index + chunksize;
      }

      // Now wait to receive back the results from each worker task and print 
      // a few sample values 
      for (i=1; i<= nworkers; i++)
      {   source = i;
          MPI_Recv(&index, 1, MPI_INT, source, indexmsg, MPI_COMM_WORLD, &status);
          MPI_Recv(&result[index], chunksize, MPI_FLOAT, source, arraymsg, MPI_COMM_WORLD, &status);
          printf("---------------------------------------------------\n");
          printf("MASTER: Sample results from worker task = %d\n",source);
          printf("   result[%d]=%f\n", index, result[index]);
          printf("   result[%d]=%f\n", index+100, result[index+100]);
          printf("   result[%d]=%f\n\n", index+1000, result[index+1000]);
          fflush(stdout);
      }
      printf("MASTER: All Done! \n");
   }

/**************************** worker task ************************************/

   if (taskid > MASTER)
   {   // Receive my portion of array from the master task 
       source = MASTER;

       //MPI_Recv(buffer,count,type,source,tag,comm,status) 
       MPI_Recv(&index, 1, MPI_INT, source, indexmsg, MPI_COMM_WORLD, &status);
       MPI_Recv(&result[index], chunksize, MPI_FLOAT, source, arraymsg, MPI_COMM_WORLD, &status);
 
       // Do a simple value assignment to each of my array elements 
       for(i=index; i < index + chunksize; i++)
          result[i] = i + 1;

       // Send my results back to the master task 
       dest = MASTER;
       MPI_Send(&index, 1, MPI_INT, dest, indexmsg, MPI_COMM_WORLD);
       MPI_Send(&result[index], chunksize, MPI_FLOAT, MASTER, arraymsg, MPI_COMM_WORLD);
   }
   MPI_Finalize();
}
