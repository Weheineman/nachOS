/// Test program to sort a large number of integers.
///
/// Intention is to stress virtual memory system.
///
/// Ideally, we could read the unsorted array off of the file system,
/// and store the result back to the file system!


#include "syscall.h"


#define DIM  1024

/// Size of physical memory; with code, we will run out of space!
static int A[DIM];

unsigned itoa(int n, char result[]){
	unsigned index = 0;
	unsigned digits = 1;
	unsigned pot = 10;
	if(n < 0){
		n = -1 * n;
		result[index] = '-';
		index ++;
	}

	for(; pot <= n; pot = pot * 10, digits ++);
	
	pot = pot / 10;
	
	while(pot > 0){
		result[index] = '0' + (n / pot);
		index ++;
		n = n % pot;
		pot = pot / 10;
	}
	
	result[index] = 0;
	return index;
}


int
main(void)
{
    int i, j, tmp;

    // First initialize the array, in reverse sorted order.
    for (i = 0; i < DIM; i++)
        A[i] = DIM - i - 1;

    // Then sort!
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM - i - 1; j++)
            if (A[j] > A[j + 1]) {  // Out of order -> need to swap!
                tmp = A[j];
                A[j] = A[j + 1];
                A[j + 1] = tmp;
            }

    // And then we're done -- should be 0!
    char result[20];
    unsigned length = itoa(A[0], result);
    
    Write("Finished!\n", 11, CONSOLE_OUTPUT);
    Write("Exit status: ", 14, CONSOLE_OUTPUT);
    Write(result, length, CONSOLE_OUTPUT);
    Write("\n", 2, CONSOLE_OUTPUT);
    Exit(A[0]);
}
