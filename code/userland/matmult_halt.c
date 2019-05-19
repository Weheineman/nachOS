/// Test program to do matrix multiplication on large arrays.
///
/// Intended to stress virtual memory system.
///
/// Ideally, we could read the matrices off of the file system, and store the
/// result back to the file system!


#include "syscall.h"


/// Sum total of the arrays does not fit in physical memory.
#define DIM  20

static int A[DIM][DIM];
static int B[DIM][DIM];
static int C[DIM][DIM];

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
    int i, j, k;

    // First initialize the matrices.
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }

    // Then multiply them together.
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++)
            for (k = 0; k < DIM; k++)
                C[i][j] += A[i][k] * B[k][j];

    // And then we are done.
    char result[20];
    unsigned length = itoa(C[DIM - 1][DIM - 1], result);
    
    Write("Finished!\n", 11, CONSOLE_OUTPUT);
    Write("Result: ", 9, CONSOLE_OUTPUT);
    Write(result, length, CONSOLE_OUTPUT);
    Write("\n", 2, CONSOLE_OUTPUT);
    Halt();
}
