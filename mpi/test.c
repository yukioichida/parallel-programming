#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void remove_index(int *array, int array_length){
   int i;
   for(i = 0; i < array_length - 1; i++) array[i] = array[i + 1];
}

void add_index(int *array, int index, int array_length){
   int i;
   for(i = array_length-1; i >= 0 - 1; i--) array[i] = array[i - 1];
   array[0] = index;
}

int main()
{
    int i;
    int array[6] = {1,2,3,4,5,6};
    int *firstHalf = array;
    int *secondHalf = array + 1;

    for (i=0;i<6;i++){
        printf("%d\n", secondHalf[i]);
    }

    return 0;
}