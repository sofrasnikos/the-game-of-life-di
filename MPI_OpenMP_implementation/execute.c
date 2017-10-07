#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "execute.h"

int execute(int rank, int num_of_proc, int num_of_threads, int dimension, int sub_grid_dimension, int loops, char *input_file, int prints_enabled) {
	int i, j, p;
	char **grid, **local_grid, **next_local_grid;
	/* The below values are used to split equally the grid
	 * into blocks which will be assigned to processes */
	int proc_grid_dimension = dimension / sub_grid_dimension;
	int periods[2], dim_size[2];
	periods[0] = 1;
	periods[1] = 1;
	dim_size[0] = proc_grid_dimension;
	dim_size[1] = proc_grid_dimension;

	MPI_Comm comm;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim_size, periods, 1, &comm);

	/* Process with rank == 0 creates & initializes the grid*/
	if (rank == 0) {
		createGrid(&grid, dimension);
		initGrid(grid, dimension);
		if (input_file != NULL) {
			readGrid(grid, input_file, dimension);
		}
		if (prints_enabled == 1) {
			int dir_stat = mkdir("outputs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (dir_stat != 0 && errno != EEXIST) {
				printf("mkdir error %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			printf("block size: %d\n", sub_grid_dimension);
		}
	}

	/* Create a zero array to compare if a local grid has only 0 */
	char *zero_block = malloc(sizeof(char) * sub_grid_dimension * sub_grid_dimension);
	if (zero_block == NULL) {
		free(zero_block);
		printf("malloc error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(zero_block, 0,
			sizeof(char) * sub_grid_dimension * sub_grid_dimension);

	/* Create local array */
	createGrid(&local_grid, sub_grid_dimension);
	/* Create another local grid which will contain the values after the calculations */
	createGrid(&next_local_grid, sub_grid_dimension);
	//INIT
	/* Create MPI_Datatypes*/
	MPI_Datatype block_type_1, block_type_2;
	MPI_Type_vector(sub_grid_dimension, sub_grid_dimension, dimension, MPI_CHAR, &block_type_2);
	MPI_Type_create_resized(block_type_2, 0, sizeof(char), &block_type_1);
	MPI_Type_commit(&block_type_1);

	int sendcounts[proc_grid_dimension * proc_grid_dimension];
	int displs[proc_grid_dimension * proc_grid_dimension];

	char *ptr_to_grid = NULL;
	if (rank == 0) {
		ptr_to_grid = &(grid[0][0]);
	}

	/* Scatter the grid */
	for (i = 0; i < proc_grid_dimension; i++) {
		for (j = 0; j < proc_grid_dimension; j++) {
			displs[i * proc_grid_dimension + j] = i * dimension * sub_grid_dimension + j * sub_grid_dimension;
			sendcounts[i * proc_grid_dimension + j] = 1;
		}
	}
	MPI_Scatterv(ptr_to_grid, sendcounts, displs, block_type_1, &(local_grid[0][0]), sub_grid_dimension * sub_grid_dimension, MPI_CHAR, 0,
	MPI_COMM_WORLD);

	/* Print grid if -p flag is given */
	if (rank == 0 && prints_enabled == 1) {
		printGrid(grid, dimension, rank, 1);
	}

	int coords[2];
	MPI_Cart_coords(comm, rank, 2, coords);

	/* Determine the neighboring processes*/
	int top[2], bot[2], left[2], right[2], top_left[2], top_right[2], bot_left[2], bot_right[2];
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

	int top_rank, bot_rank, left_rank, right_rank, top_left_rank, top_right_rank, bot_left_rank, bot_right_rank;
	MPI_Cart_rank(comm, top, &top_rank);
	MPI_Cart_rank(comm, bot, &bot_rank);
	MPI_Cart_rank(comm, left, &left_rank);
	MPI_Cart_rank(comm, right, &right_rank);
	MPI_Cart_rank(comm, top_left, &top_left_rank);
	MPI_Cart_rank(comm, top_right, &top_right_rank);
	MPI_Cart_rank(comm, bot_left, &bot_left_rank);
	MPI_Cart_rank(comm, bot_right, &bot_right_rank);
	MPI_Datatype for_columns;
	MPI_Type_vector(sub_grid_dimension, 1, sub_grid_dimension, MPI_CHAR, &for_columns);
	MPI_Type_commit(&for_columns);
	MPI_Status status;
	MPI_Request request;
	int continue_next_gen;
	int *dif_array = NULL;
	if (rank == 0) {
		dif_array = malloc(sizeof(int) * num_of_proc);
		if (dif_array == NULL) {
			free(dif_array);
			printf("malloc error %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		for (p = 0; p < num_of_proc; p++) {
			dif_array[p] = 3;
		}
	}

	char top_buff[sub_grid_dimension], bot_buff[sub_grid_dimension], left_buff[sub_grid_dimension], right_buff[sub_grid_dimension];
	char top_left_value, top_right_value, bot_left_value, bot_right_value;
	int generation = 1;
	continue_next_gen = 1;
	while (continue_next_gen == 1 && generation <= loops) {
		if (rank == 0 && prints_enabled == 1) {
			printf("Generation: %d\n", generation);
		}
		// Send to all eight neighbors
		MPI_Isend(&local_grid[0][0], sub_grid_dimension, MPI_CHAR, top_rank, 0,
		MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[sub_grid_dimension - 1][0], sub_grid_dimension, MPI_CHAR, bot_rank, 0, MPI_COMM_WORLD, &request);

		// Left and right are special cases because items in columns are not contiguous
		MPI_Isend(&local_grid[0][0], 1, for_columns, left_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[0][sub_grid_dimension - 1], 1, for_columns, right_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[0][0], 1, MPI_CHAR, top_left_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[0][sub_grid_dimension - 1], 1, MPI_CHAR, top_right_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[sub_grid_dimension - 1][0], 1, MPI_CHAR, bot_left_rank, 0,
		MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[sub_grid_dimension - 1][sub_grid_dimension - 1], 1, MPI_CHAR, bot_right_rank, 0, MPI_COMM_WORLD, &request);

		// Calculate the middle cells while waiting to receive from neighbors
		calculateInnerCells(sub_grid_dimension, local_grid, next_local_grid, num_of_threads);

		/* Receive from every neighbor */
		MPI_Recv(&bot_buff, sub_grid_dimension, MPI_CHAR, bot_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&top_buff, sub_grid_dimension, MPI_CHAR, top_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&right_buff, sub_grid_dimension, MPI_CHAR, right_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&left_buff, sub_grid_dimension, MPI_CHAR, left_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&bot_right_value, 1, MPI_CHAR, bot_right_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&bot_left_value, 1, MPI_CHAR, bot_left_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&top_right_value, 1, MPI_CHAR, top_right_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&top_left_value, 1, MPI_CHAR, top_left_rank, 0, MPI_COMM_WORLD, &status);

		calculateEdgeCells(sub_grid_dimension, local_grid, next_local_grid, top_buff, right_buff, bot_buff, left_buff, top_left_value, top_right_value,
				bot_left_value, bot_right_value, num_of_threads);

		int different = 0;
		// Check if grid is next_local_grid by checking if it is the same as a grid full of zeros
		int zero_check = memcmp(zero_block, *next_local_grid, sub_grid_dimension * sub_grid_dimension * sizeof(char));
		// If at least one cell is not zero
		if (zero_check != 0) {
			different = memcmp(*local_grid, *next_local_grid, sub_grid_dimension * sub_grid_dimension * sizeof(char));
			zero_check = 1;
		}
		int keep_looping;
		// If it is different and not zero
		if (different != 0 && zero_check != 0) {
			keep_looping = 1;
		} else {
			keep_looping = 0;
		}
		MPI_Gather(&keep_looping, 1, MPI_INT, dif_array, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (rank == 0) {
			continue_next_gen = 0;
			for (p = 0; p < num_of_proc; p++) {
				if (dif_array[p] == 1) {
					continue_next_gen = 1;
					break;
				}
			}
		}
		MPI_Bcast(&continue_next_gen, 1, MPI_INT, 0, MPI_COMM_WORLD);

		/* Gather grid if -p flag is given */
		if(prints_enabled == 1){
			MPI_Gatherv(&(next_local_grid[0][0]), sub_grid_dimension * sub_grid_dimension, MPI_CHAR, ptr_to_grid, sendcounts, displs, block_type_1, 0, MPI_COMM_WORLD);
		}

		char** temp;
		temp = &(*next_local_grid);
		next_local_grid = &(*local_grid);
		local_grid = temp;

		/* Print grid if -p flag is given */
		if (rank == 0 && prints_enabled == 1) {
			printGrid(grid, dimension, rank, 1);
		}
		generation++;
	}

	/* Free local grids */
	freeGrid(&local_grid);
	freeGrid(&next_local_grid);
	MPI_Type_free(&block_type_1);
	MPI_Type_free(&block_type_2);
	MPI_Type_free(&for_columns);
	free(zero_block);

	/* Free grid */
	if (rank == 0) {
		freeGrid(&grid);
	}

	return 0;
}

void calculateInnerCells(int sub_grid_dimension, char **local_grid, char **next_local_grid, int num_of_threads) {
	int i, j;
	int me;
	/* To calculate the middle cells we have to ignore the first row & column and the last row & column */
	/* Ignore the first row (i == 0) and the last row (i == sub_grid_dimension - 1) */
	int alive_neighbors;
	int pid = getpid();
	omp_set_num_threads(num_of_threads);
	#pragma omp parallel for shared (sub_grid_dimension, next_local_grid, local_grid) private(i, j, me) reduction(+:alive_neighbors) collapse(2)
	for (i = 1; i < sub_grid_dimension - 1; i++) {
		// printf("Iteration %d is assigned to thread %d of %d. pid master: %d\n", i, tid, total, pid);
		/* Ignore the first column (j == 0) and the last column (j == sub_grid_dimension - 1) */
		for (j = 1; j < sub_grid_dimension - 1; j++) {
			alive_neighbors = 0;
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
			me = local_grid[i][j];
			next_local_grid[i][j] = deadOrAlive(alive_neighbors, me);
		}
	}
}

void calculateEdgeCells(int sub_grid_dimension, char **local_grid, char **next_local_grid, char *top_buff, char *right_buff, char *bot_buff, char *left_buff,
		char top_left_value, char top_right_value, char bot_left_value, char bot_right_value, int num_of_threads) {
	int k;
	int alive_neighbors = 0;
	int me;
	int pid = getpid();
	omp_set_num_threads(num_of_threads);
	#pragma omp parallel for shared (sub_grid_dimension, next_local_grid, local_grid, top_buff, bot_buff, left_buff, right_buff) private(k, me) reduction(+:alive_neighbors)
	for (k = 1; k < sub_grid_dimension - 1; k++) {
		// printf("Iteration %d is assigned to thread %d of %d. pid master: %d\n", k, tid, total, pid);
		/* TOP ROW */
		alive_neighbors = 0;
		/* Calculate the value of the current cell according to its neighbors */
		/* Top left neighbor (the value is borrowed by other process) */
		alive_neighbors += top_buff[k - 1];
		/* Top neighbor (the value is borrowed by other process) */
		alive_neighbors += top_buff[k];
		/* Top right neighbor (the value is borrowed by other process) */
		alive_neighbors += top_buff[k + 1];
		/* Right neighbor */
		alive_neighbors += local_grid[0][k + 1];
		/* Bot right neighbor */
		alive_neighbors += local_grid[1][k + 1];
		/* Bot neighbor */
		alive_neighbors += local_grid[1][k];
		/* Bot left neighbor */
		alive_neighbors += local_grid[1][k - 1];
		/* Left neighbor */
		alive_neighbors += local_grid[0][k - 1];
		me = local_grid[0][k];
		next_local_grid[0][k] = deadOrAlive(alive_neighbors, me);

		/* BOT ROW */
		alive_neighbors = 0;
		/* Calculate the value of the current cell according to its neighbors */
		/* Top left neighbor (the value is borrowed by other process) */
		alive_neighbors += bot_buff[k - 1];
		/* Top neighbor (the value is borrowed by other process) */
		alive_neighbors += bot_buff[k];
		/* Top right neighbor (the value is borrowed by other process) */
		alive_neighbors += bot_buff[k + 1];
		/* Right neighbor */
		alive_neighbors += local_grid[sub_grid_dimension - 1][k + 1];
		/* Bot right neighbor */
		alive_neighbors += local_grid[sub_grid_dimension - 2][k + 1];
		/* Bot neighbor */
		alive_neighbors += local_grid[sub_grid_dimension - 2][k];
		/* Bot left neighbor */
		alive_neighbors += local_grid[sub_grid_dimension - 2][k - 1];
		/* Left neighbor */
		alive_neighbors += local_grid[sub_grid_dimension - 1][k - 1];
		me = local_grid[sub_grid_dimension - 1][k];
		next_local_grid[sub_grid_dimension - 1][k] = deadOrAlive(alive_neighbors, me);

		/* LEFT COLUMN */
		alive_neighbors = 0;
		/* Calculate the value of the current cell according to its neighbors */
		/* Top left neighbor (the value is borrowed by other process) */
		alive_neighbors += left_buff[k - 1];
		/* Top neighbor (the value is borrowed by other process) */
		alive_neighbors += left_buff[k];
		/* Top right neighbor (the value is borrowed by other process) */
		alive_neighbors += left_buff[k + 1];
		/* Right neighbor */
		alive_neighbors += local_grid[k + 1][0];
		/* Bot right neighbor */
		alive_neighbors += local_grid[k + 1][1];
		/* Bot neighbor */
		alive_neighbors += local_grid[k][1];
		/* Bot left neighbor */
		alive_neighbors += local_grid[k - 1][1];
		/* Left neighbor */
		alive_neighbors += local_grid[k - 1][0];
		me = local_grid[k][0];
		next_local_grid[k][0] = deadOrAlive(alive_neighbors, me);

		/* RIGHT COLUMN */
		alive_neighbors = 0;
		/* Calculate the value of the current cell according to its neighbors */
		/* Top left neighbor (the value is borrowed by other process) */
		alive_neighbors += right_buff[k - 1];
		/* Top neighbor (the value is borrowed by other process) */
		alive_neighbors += right_buff[k];
		/* Top right neighbor (the value is borrowed by other process) */
		alive_neighbors += right_buff[k + 1];
		/* Right neighbor */
		alive_neighbors += local_grid[k + 1][sub_grid_dimension - 1];
		/* Bot right neighbor */
		alive_neighbors += local_grid[k + 1][sub_grid_dimension - 2];
		/* Bot neighbor */
		alive_neighbors += local_grid[k][sub_grid_dimension - 2];
		/* Bot left neighbor */
		alive_neighbors += local_grid[k - 1][sub_grid_dimension - 2];
		/* Left neighbor */
		alive_neighbors += local_grid[k - 1][sub_grid_dimension - 1];
		me = local_grid[k][sub_grid_dimension - 1];
		next_local_grid[k][sub_grid_dimension - 1] = deadOrAlive(alive_neighbors, me);
	}
	/* TOP LEFT CELL */
	alive_neighbors = 0;
	/* Calculate the value of the current cell according to its neighbors */
	/* Top left neighbor (the value is borrowed by other process) */
	alive_neighbors += top_left_value;
	/* Top neighbor (the value is borrowed by other process) */
	alive_neighbors += top_buff[0];
	/* Top right neighbor (the value is borrowed by other process) */
	alive_neighbors += top_buff[1];
	/* Right neighbor */
	alive_neighbors += local_grid[0][1];
	/* Bot right neighbor */
	alive_neighbors += local_grid[1][1];
	/* Bot neighbor */
	alive_neighbors += local_grid[1][0];
	/* Bot left neighbor (the value is borrowed by other process) */
	alive_neighbors += left_buff[1];
	/* Left neighbor (the value is borrowed by other process) */
	alive_neighbors += left_buff[0];
	me = local_grid[0][0];
	next_local_grid[0][0] = deadOrAlive(alive_neighbors, me);

	/* TOP RIGHT CELL */
	alive_neighbors = 0;
	/* Calculate the value of the current cell according to its neighbors */
	/* Top left neighbor (the value is borrowed by other process) */
	alive_neighbors += top_buff[sub_grid_dimension - 2];
	/* Top neighbor (the value is borrowed by other process) */
	alive_neighbors += top_buff[sub_grid_dimension - 1];
	/* Top right neighbor (the value is borrowed by other process) */
	alive_neighbors += top_right_value;
	/* Right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[0];
	/* Bot right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[1];
	/* Bot neighbor */
	alive_neighbors += local_grid[1][sub_grid_dimension - 1];
	/* Bot left neighbor */
	alive_neighbors += local_grid[1][sub_grid_dimension - 2];
	/* Left neighbor */
	alive_neighbors += local_grid[0][sub_grid_dimension - 2];
	me = local_grid[0][sub_grid_dimension - 1];
	next_local_grid[0][sub_grid_dimension - 1] = deadOrAlive(alive_neighbors, me);

	/* BOTTOM RIGHT CELL */
	alive_neighbors = 0;
	/* Bot right cell
	 * (this is special case, to calculate next round we need bot_right_value) */
	/* Calculate the value of the current cell according to its neighbors */
	/* Top left neighbor */
	alive_neighbors += local_grid[sub_grid_dimension - 2][sub_grid_dimension - 2];
	/* Top neighbor */
	alive_neighbors += local_grid[sub_grid_dimension - 2][sub_grid_dimension - 1];
	/* Top right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[sub_grid_dimension - 2];
	/* Right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[sub_grid_dimension - 1];
	/* Bot right neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_right_value;
	/* Bot neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[sub_grid_dimension - 1];
	/* Bot left neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[sub_grid_dimension - 2];
	/* Left neighbor */
	alive_neighbors += local_grid[sub_grid_dimension - 1][sub_grid_dimension - 2];
	me = local_grid[sub_grid_dimension - 1][sub_grid_dimension - 1];
	next_local_grid[sub_grid_dimension - 1][sub_grid_dimension - 1] = deadOrAlive(alive_neighbors, me);

	/* BOTTOM LEFT CELL */
	alive_neighbors = 0;
	/* Calculate the value of the current cell according to its neighbors */
	/* Top left neighbor (the value is borrowed by other process) */
	alive_neighbors += left_buff[sub_grid_dimension - 2];
	/* Top neighbor */
	alive_neighbors += local_grid[sub_grid_dimension - 2][0];
	/* Top right neighbor */
	alive_neighbors += local_grid[sub_grid_dimension - 2][1];
	/* Right neighbor */
	alive_neighbors += local_grid[sub_grid_dimension - 1][1];
	/* Bot right neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[1];
	/* Bot neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[0];
	/* Bot left neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_left_value;
	/* Left neighbor (the value is borrowed by other process) */
	alive_neighbors += left_buff[sub_grid_dimension - 1];
	/* If it is empty space */
	me = local_grid[sub_grid_dimension - 1][0];
	next_local_grid[sub_grid_dimension - 1][0] = deadOrAlive(alive_neighbors, me);
}

int deadOrAlive(int alive_neighbors, int status) {
	/* If it is empty space */
	if (status == 0) {
		/* If there are exactly 3 neighbors create a new cell */
		if (alive_neighbors == 3) {
			/* CREATE NEW CELL */
			return 1;
		}
		/* Leave it empty */
		else {
			return 0;
		}
	}
	/* If a cell already lives */
	else {
		/* Determine if the cell lives or dies in next round */
		/* DIE */
		if (alive_neighbors < 2 || alive_neighbors > 3) {
			return 0;
		}
		/* LIVE */
		else {
			return 1;
		}
	}
}
