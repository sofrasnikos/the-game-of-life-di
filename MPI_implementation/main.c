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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "functions.h"

#define DIMENSION_SIZE 200

int main(int argc, char *argv[]) {
	srand(time(NULL));
	int i, j;
	int **grid, **local_grid;
	int rank, num_of_proc;
	int dimension = DIMENSION_SIZE;
	SplitAttributes attributes = ProcessNumber(dimension);
	int block_dimension = attributes.length_of_sides;

	/* The below values are used to split equally the grid
	 * into blocks which will be assigned to processes */
	int proc_grid_dimension = dimension / attributes.length_of_sides;
	int periods[2], dim_size[2];
	periods[0] = 1;
	periods[1] = 1;
	dim_size[0] = proc_grid_dimension;
	dim_size[1] = proc_grid_dimension;

	MPI_Comm comm;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_proc);

	if (num_of_proc != attributes.number_of_processes) {
		if (rank == 0) {
			printf(
					"the given grid is %d x %d\nthe optimal number of processes is %d\n",
					dimension, dimension, attributes.number_of_processes);
		}
		MPI_Finalize();
		exit(0);
	}

	MPI_Cart_create(MPI_COMM_WORLD, 2, dim_size, periods, 1, &comm);

	/* Process with rank == 0 creates & initializes the grid*/
	if (rank == 0) {
		CreateGrid(&grid, dimension);
		InitGrid(grid, dimension);
		int dir_stat = mkdir("../outputs",
		S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		printf("side = %d\n", block_dimension);
	}

	//MPI_Bcast(&(grid[0][0]), rows * columns, MPI_INT, 0, MPI_COMM_WORLD);

	/* Create local array */
	CreateGrid(&local_grid, block_dimension);

	/* Create MPI_Datatypes*/
	MPI_Datatype block_type_1, block_type_2;
	MPI_Type_vector(block_dimension, block_dimension, dimension, MPI_INT,
			&block_type_2);
	MPI_Type_create_resized(block_type_2, 0, sizeof(int), &block_type_1);
	MPI_Type_commit(&block_type_1);

	int sendcounts[proc_grid_dimension * proc_grid_dimension];
	int displs[proc_grid_dimension * proc_grid_dimension];

	int *ptr_to_grid = NULL;
	if (rank == 0) {
		ptr_to_grid = &(grid[0][0]);
	}

	/* Scatter the grid*/
	for (i = 0; i < proc_grid_dimension; i++) {
		for (j = 0; j < proc_grid_dimension; j++) {
			displs[i * proc_grid_dimension + j] = i * dimension
					* block_dimension + j * block_dimension;
			sendcounts[i * proc_grid_dimension + j] = 1;
		}
	}
	MPI_Scatterv(ptr_to_grid, sendcounts, displs, block_type_1,
			&(local_grid[0][0]), block_dimension * block_dimension, MPI_INT, 0,
			MPI_COMM_WORLD);

	/* Print grid*/
	if (rank == 0) {
		PrintGrid(grid, dimension, rank, 1);
	}
	/* Print local blocks(subgrids)*/
	int p;
	for (p = 0; p < num_of_proc; p++) {
		if (rank == p) {
			PrintGrid(local_grid, block_dimension, rank, 0);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	int coords[2];
	MPI_Cart_coords(comm, rank, 2, coords);
	//printf("%d, %d %d\n", rank, coords[0], coords[1]);

	/* Determine the neighbouring procceses*/
	int top[2], bot[2], left[2], right[2], top_left[2], top_right[2],
			bot_left[2], bot_right[2];
	top[0] = coords[0] - 1;
	top[1] = coords[1];
	bot[0] = coords[0] + 1;
	bot[1] = coords[1];
	left[0] = coords[0];
	left[1] = coords[1] - 1;
	right[0] = coords[0];
	right[1] = coords[1] + 1;
	top_left[0] = coords[0] - 1;
	top_left[1] = coords[1] - 1;
	top_right[0] = coords[0] - 1;
	top_right[1] = coords[1] + 1;
	bot_left[0] = coords[0] + 1;
	bot_left[1] = coords[1] - 1;
	bot_right[0] = coords[0] + 1;
	bot_right[1] = coords[1] + 1;

	int top_rank, bot_rank, left_rank, right_rank, top_left_rank,
			top_right_rank, bot_left_rank, bot_right_rank;
	MPI_Cart_rank(comm, top, &top_rank);
	MPI_Cart_rank(comm, bot, &bot_rank);
	MPI_Cart_rank(comm, left, &left_rank);
	MPI_Cart_rank(comm, right, &right_rank);
	MPI_Cart_rank(comm, top_left, &top_left_rank);
	MPI_Cart_rank(comm, top_right, &top_right_rank);
	MPI_Cart_rank(comm, bot_left, &bot_left_rank);
	MPI_Cart_rank(comm, bot_right, &bot_right_rank);
	if (rank == 0) {
		printf("%d %d %d %d %d %d %d %d\n", top_rank, bot_rank, left_rank,
				right_rank, top_left_rank, top_right_rank, bot_left_rank,
				bot_right_rank);
	}

	MPI_Status status;
	MPI_Request request;
	int buff[40];
	MPI_Isend(&local_grid[0][0], 40, MPI_INT, top_rank, 0, MPI_COMM_WORLD,
			&request);
	MPI_Recv(&buff, 40, MPI_INT, bot_rank, 0, MPI_COMM_WORLD, &status);
	if (rank == 6) {
		printf("I am rank %d. My buff from bot (%d) == ", rank, bot_rank);
		for (i = 0; i < 40; i++) {
			if (buff[i] == 1) {
				printf("*");
			} else {
				printf(".");
			}
		}
		printf("\n");
	}

	/* Gather all processed blocks to process 0 */
	MPI_Gatherv(&(local_grid[0][0]), block_dimension * block_dimension, MPI_INT,
			ptr_to_grid, sendcounts, displs, block_type_1, 0, MPI_COMM_WORLD);

	/* Free local grids*/
	FreeGrid(&local_grid);
	MPI_Type_free(&block_type_1);
	MPI_Type_free(&block_type_2);

	if (rank == 0) {
		PrintGrid(grid, dimension, rank, 1);
	}
	/* Free grid*/
	if (rank == 0) {
		FreeGrid(&grid);
	}
	MPI_Finalize();
	return 0;
}
