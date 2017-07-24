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

	/* Allocate grid*/
	grid = malloc(sizeof(int *) * rows);
	if (grid == NULL) {
		printf("malloc error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < rows; i++) {
		grid[i] = malloc(sizeof(int) * columns);
		if (grid[i] == NULL) {
			printf("malloc error %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Process with rank =0 initializes the grid*/
	if (rank == 0) {
		srand(time(NULL));
		/* Allocate grid*/

		int r;
		/* Initialize the grid*/
		for (i = 0; i < rows; i++) {
			for (j = 0; j < columns; j++) {
				/* Range from 1 to 100*/
				r = rand() % 100 + 1;
				/* 30% possibility to create a cell
				 * 70% possibility to create empty space*/
				if (r < 30) {
					grid[i][j] = 1;
				} else {
					grid[i][j] = 0;
				}
			}
		}
	}

	MPI_Bcast(&grid[0][0], rows * columns, MPI_INT, 0, MPI_COMM_WORLD);
	PrintGrid(grid, rows, columns, rank);

	MPI_Finalize();
	/* Free grid*/
	return 0;
}
