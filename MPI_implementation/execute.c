/*
 * execute.c
 *
 *  Created on: Aug 8, 2017
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

#include "execute.h"

int Execute(int rank, int num_of_proc, int dimension,
		SplitAttributes attributes) {

	int i, j;
	int **grid, **local_grid, **next_local_grid;
	int block_dimension = attributes.length_of_sides;

	/* The below values are used to split equally the grid
	 * into blocks which will be assigned to processes */
	int proc_grid_dimension = dimension / attributes.length_of_sides;
	int periods[2], dim_size[2];
	periods[0] = 1;
	periods[1] = 1;
	dim_size[0] = proc_grid_dimension;
	dim_size[1] = proc_grid_dimension;

	if (num_of_proc != attributes.number_of_processes) {
		if (rank == 0) {
			printf(
					"the given grid is %d x %d\nthe optimal number of processes is %d\n",
					dimension, dimension, attributes.number_of_processes);
		}
		MPI_Finalize();
		exit(0);
	}

	MPI_Comm comm;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim_size, periods, 1, &comm);

	/* Process with rank == 0 creates & initializes the grid*/
	if (rank == 0) {
		CreateGrid(&grid, dimension);
		InitGrid(grid, dimension);
		int dir_stat = mkdir("../outputs",
		S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (dir_stat != 0 && errno != EEXIST) {
			printf("mkdir error %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("side = %d\n", block_dimension);
	}

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

	/* Determine the neighboring processes*/
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
	//todo isws na mhn uparxei logos na einai 2 metavlhtes me thn idia timh
	// to exw proswrina etsi na mhn trwei seg fault se alles diastaseis grid
	int line_size = block_dimension;
	int top_buff[line_size], bot_buff[line_size], left_buff[line_size],
			right_buff[line_size];
	int top_left_value, top_right_value, bot_left_value, bot_right_value;
	// Send to all eight neighbors
	MPI_Isend(&local_grid[0][0], line_size, MPI_INT, top_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[line_size - 1][0], line_size, MPI_INT, bot_rank, 0,
	MPI_COMM_WORLD, &request);
	int temp_left_buff[line_size], temp_right_buff[line_size];
	// Left and right are special cases because we have to create a buffer since the values are in a column
	for (i = 0; i < line_size; i++) {
		temp_left_buff[i] = local_grid[i][0];
		temp_right_buff[i] = local_grid[i][line_size - 1];
	}
	MPI_Isend(temp_left_buff, line_size, MPI_INT, left_rank, 0, MPI_COMM_WORLD,
			&request);
	MPI_Isend(temp_right_buff, line_size, MPI_INT, right_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[0][0], 1, MPI_INT, top_left_rank, 0, MPI_COMM_WORLD,
			&request);
	MPI_Isend(&local_grid[0][line_size - 1], 1, MPI_INT, top_right_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[line_size - 1][0], 1, MPI_INT, bot_left_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[line_size - 1][line_size - 1], 1, MPI_INT,
			bot_right_rank, 0, MPI_COMM_WORLD, &request);

	// Calculate the middle cells while waiting to receive from neighbors
	/* Create a new local grid which will contain the values after the calculations */
	CreateGrid(&next_local_grid, block_dimension);

	/* To calculate the middle cells we have to ignore the first row & column and the last row & column */
	/* Ignore the first row (i == 0) and the last row (i == block_dimension - 1) */
	int alive_neighbors;
	for (i = 1; i < block_dimension - 1; i++) {
		/* Ignore the first column (j == 0) and the last column (j == block_dimension - 1) */
		alive_neighbors = 0;
		for (j = 1; j < block_dimension - 1; j++) {
			/* Calculate the value of the current cell according to its neighbors */
			/* Top left neighbor */
			alive_neighbors += local_grid[i - 1][j - 1];
			/* Top neighbor */
			alive_neighbors += local_grid[i - 1][j];
			/* Top right neighbor */
			alive_neighbors += local_grid[i - 1][j + 1];
			/* Right neighbor */
			alive_neighbors += local_grid[i][j + 1];
			/* Bot right neighbor */
			alive_neighbors += local_grid[i + 1][j + 1];
			/* Bot neighbor */
			alive_neighbors += local_grid[i + 1][j];
			/* Bot left neighbor */
			alive_neighbors += local_grid[i + 1][j - 1];
			/* Left neighbor */
			alive_neighbors += local_grid[i][j - 1];

			/* Determine if the cell lives or dies in next round */
			/* Store the new value to the next_local_grid */
			/* DIE */
			if(alive_neighbors < 2 || alive_neighbors > 3){
				next_local_grid[i][j] = 0;
			}
			/* LIVE */
			else{
				next_local_grid[i][j] = 1;
			}
		}
	}

// Receive from every neighbor
	MPI_Recv(&bot_buff, line_size, MPI_INT, bot_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&top_buff, line_size, MPI_INT, top_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&right_buff, line_size, MPI_INT, right_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&left_buff, line_size, MPI_INT, left_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&bot_right_value, 1, MPI_INT, bot_right_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&bot_left_value, 1, MPI_INT, bot_left_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&top_right_value, 1, MPI_INT, top_right_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&top_left_value, 1, MPI_INT, top_left_rank, 0, MPI_COMM_WORLD,
			&status);
	if (rank == 6) { // TODO na fugei olo to if molis tsekareis oti doulevei swsta
		printf("I am rank %d. Received from bot (%d):", rank, bot_rank);
		for (i = 0; i < line_size; i++) {
			if (bot_buff[i] == 1) {
				printf("*");
			} else {
				printf(".");
			}
		}
		printf("\n");
		printf("I am rank %d. Received from top (%d):", rank, top_rank);
		for (i = 0; i < line_size; i++) {
			if (top_buff[i] == 1) {
				printf("*");
			} else {
				printf(".");
			}
		}
		printf("\n");
		printf("I am rank %d. Received from right (%d):\n", rank, right_rank);
		for (i = 0; i < line_size; i++) {
			if (right_buff[i] == 1) {
				printf("*\n");
			} else {
				printf(".\n");
			}
		}
		printf("I am rank %d. Received from left (%d):\n", rank, left_rank);
		for (i = 0; i < line_size; i++) {
			if (left_buff[i] == 1) {
				printf("*\n");
			} else {
				printf(".\n");
			}
		}
		printf("I am rank %d. Received from bot right(%d):", rank,
				bot_right_rank);
		if (bot_right_value == 1) {
			printf("*\n");
		} else {
			printf(".\n");
		}
		printf("I am rank %d. Received from bot left(%d):", rank,
				bot_left_rank);
		if (bot_left_value == 1) {
			printf("*\n");
		} else {
			printf(".\n");
		}
		printf("I am rank %d. Received from top right(%d):", rank,
				top_right_rank);
		if (top_right_value == 1) {
			printf("*\n");
		} else {
			printf(".\n");
		}
		printf("I am rank %d. Received from top left(%d):", rank,
				top_left_rank);
		if (top_left_value == 1) {
			printf("*\n");
		} else {
			printf(".\n");
		}
	}
// Calculate edge cells

	/* Gather all processed blocks to process 0 */
	MPI_Gatherv(&(local_grid[0][0]), block_dimension * block_dimension, MPI_INT,
			ptr_to_grid, sendcounts, displs, block_type_1, 0, MPI_COMM_WORLD);

	/* Free local grids*/
	FreeGrid(&local_grid);
	FreeGrid(&next_local_grid);
	MPI_Type_free(&block_type_1);
	MPI_Type_free(&block_type_2);

	if (rank == 0) {
		PrintGrid(grid, dimension, rank, 1);
	}
	/* Free grid*/
	if (rank == 0) {
		FreeGrid(&grid);
	}

	return 0;
}
