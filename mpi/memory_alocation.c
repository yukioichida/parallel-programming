
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int l,c,i,j;

    l = 5;
    c = 10;

    int (*M)[c] = malloc (l * sizeof *M);

   	for(i=0;i<l;i++)
        for(j=0;j<c;j++)
            M[i][j] = i*j;

	for(i=0;i<l;i++)
        {
        for(j=0;j<c;j++)
            printf("%2d ",M[i][j]);
        printf("\n");
        }

    free(M);
}