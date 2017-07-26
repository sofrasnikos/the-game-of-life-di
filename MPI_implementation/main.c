/*
 * main.c
 *
 *  Created on: Jul 10, 2017
 *      Author: vangelis
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "functions.h"

#define NUM_OF_ROWS 100
#define NUM_OF_COLS 100

int main(int argc, char *argv[]) {
	printf("hello!\n");

	int **grid;
	int rank, size, i, j;
	int rows = NUM_OF_ROWS;
	int columns = NUM_OF_COLS;
	int dir_stat = mkdir("../outputs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	//MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	grid = CreateGrid(rows, columns);

	/* Process with rank == 0 initializes the grid*/
	if (rank == 0) {
		srand(time(NULL));
		InitGrid(grid, rows, columns);
	}

	MPI_Bcast(&(**grid), rows * columns, MPI_INT, 0, MPI_COMM_WORLD);

	PrintGrid(grid, rows, columns, rank);
	//MPI_Barrier(MPI_COMM_WORLD);

	FreeGrid(grid, rows, columns);

	MPI_Finalize();
	return 0;
}
