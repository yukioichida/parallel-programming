#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE  1000000


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


/* Método onde é o escravo que requisita a tarefa */
int main(int argc,char **argv){

  int i; // variáveis de índice de laços de repetição
  int array[ARRAY_SIZE];
  clock_t begin = clock();

  /* popula o vetor totalmente invertido */
  for (i = 0; i < ARRAY_SIZE; i++) array[i] = ARRAY_SIZE-i;

  bs(ARRAY_SIZE, array);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("[Sequential] Duration [%f] with size %d\n", time_spent, ARRAY_SIZE);
}