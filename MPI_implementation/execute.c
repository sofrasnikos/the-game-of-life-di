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

int execute(int rank, int num_of_proc, int dimension, SplitAttributes attributes) {

	int i, j, p;
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
			printf("the given grid is %d x %d\nthe optimal number of processes is %d\n", dimension, dimension, attributes.number_of_processes);
		}
		MPI_Finalize();
		exit(0);
	}

	MPI_Comm comm;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim_size, periods, 1, &comm);

	/* Process with rank == 0 creates & initializes the grid*/
	if (rank == 0) {
		createGrid(&grid, dimension);
		initGrid(grid, dimension);
		readGrid(grid, "../inputs/peos2.txt", dimension);
		int dir_stat = mkdir("../outputs",
		S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (dir_stat != 0 && errno != EEXIST) {
			printf("mkdir error %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("side = %d\n", block_dimension);
	}

	/* Create a zero array to compare if a local grid has only 0 */
	int zero_block[block_dimension * block_dimension];
	memset(zero_block, 0, sizeof zero_block);

	/* Create local array */
	createGrid(&local_grid, block_dimension);
	/* Create another local grid which will contain the values after the calculations */
	createGrid(&next_local_grid, block_dimension);
	//INIT
//	for (i = 0; i < block_dimension; i++) {
//		for (j = 0; j < block_dimension; j++) {
//			next_local_grid[i][j] = 0;
//		}
//	}

	/* Create MPI_Datatypes*/
	MPI_Datatype block_type_1, block_type_2;
	MPI_Type_vector(block_dimension, block_dimension, dimension, MPI_INT, &block_type_2);
	MPI_Type_create_resized(block_type_2, 0, sizeof(int), &block_type_1);
	MPI_Type_commit(&block_type_1);

	int sendcounts[proc_grid_dimension * proc_grid_dimension];
	int displs[proc_grid_dimension * proc_grid_dimension];

	int *ptr_to_grid = NULL;
	if (rank == 0) {
		ptr_to_grid = &(grid[0][0]);
	}

	/* Scatter the grid */
	for (i = 0; i < proc_grid_dimension; i++) {
		for (j = 0; j < proc_grid_dimension; j++) {
			displs[i * proc_grid_dimension + j] = i * dimension * block_dimension + j * block_dimension;
			sendcounts[i * proc_grid_dimension + j] = 1;
		}
	}
	MPI_Scatterv(ptr_to_grid, sendcounts, displs, block_type_1, &(local_grid[0][0]), block_dimension * block_dimension, MPI_INT, 0,
	MPI_COMM_WORLD);

	/* Print grid */
	if (rank == 0) {
		printGrid(grid, dimension, rank, 1);
	}
	//todo to parakatw printf uparxei gia dieukolhnsh sto debugging.
	//isws na mhn xreiazetai sth teleutaia ekdosh
	/* Print local blocks(subgrids) */
//	int p;
//	for (p = 0; p < num_of_proc; p++) {
//		if (rank == p) {
//			printGrid(local_grid, block_dimension, rank, 0);
//		}
//		MPI_Barrier(MPI_COMM_WORLD);
//	}
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
	MPI_Type_vector(block_dimension, 1, block_dimension, MPI_INT, &for_columns);
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

	int top_buff[block_dimension], bot_buff[block_dimension], left_buff[block_dimension], right_buff[block_dimension];
	int top_left_value, top_right_value, bot_left_value, bot_right_value;
	int generation = 0;
	continue_next_gen = 1;
	while (continue_next_gen == 1 && generation < 4) {
		if (rank == 0) {
			printf("Generation: %d\n", generation);
		}
		generation++;
		// Send to all eight neighbors
		MPI_Isend(&local_grid[0][0], block_dimension, MPI_INT, top_rank, 0,
		MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[block_dimension - 1][0], block_dimension, MPI_INT, bot_rank, 0, MPI_COMM_WORLD, &request);

		// Left and right are special cases because items in columns are not contiguous
		MPI_Isend(&local_grid[0][0], 1, for_columns, left_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[0][block_dimension - 1], 1, for_columns, right_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[0][0], 1, MPI_INT, top_left_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[0][block_dimension - 1], 1, MPI_INT, top_right_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[block_dimension - 1][0], 1, MPI_INT, bot_left_rank, 0,
		MPI_COMM_WORLD, &request);
		MPI_Isend(&local_grid[block_dimension - 1][block_dimension - 1], 1, MPI_INT, bot_right_rank, 0, MPI_COMM_WORLD, &request);

		// Calculate the middle cells while waiting to receive from neighbors
		calculateInnerCells(block_dimension, local_grid, next_local_grid);

		/* Receive from every neighbor */
		MPI_Recv(&bot_buff, block_dimension, MPI_INT, bot_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&top_buff, block_dimension, MPI_INT, top_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&right_buff, block_dimension, MPI_INT, right_rank, 0,
		MPI_COMM_WORLD, &status);
		MPI_Recv(&left_buff, block_dimension, MPI_INT, left_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&bot_right_value, 1, MPI_INT, bot_right_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&bot_left_value, 1, MPI_INT, bot_left_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&top_right_value, 1, MPI_INT, top_right_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&top_left_value, 1, MPI_INT, top_left_rank, 0, MPI_COMM_WORLD, &status);
		calculateEdgeCells(block_dimension, local_grid, next_local_grid, top_buff, right_buff, bot_buff, left_buff, top_left_value, top_right_value,
				bot_left_value, bot_right_value);
		//todo erase
		//print grids gia dieukolunhsh sto debugging

		for (p = 0; p < num_of_proc; p++) {
			if (rank == p) {
				printGrid(next_local_grid, block_dimension, rank, 0);
			}
//			MPI_Barrier(MPI_COMM_WORLD);
		}
		int different = 0;
		int zero_check = memcmp(zero_block, *next_local_grid, block_dimension * block_dimension * sizeof(int));
		if (zero_check == 0) {
			printf("asdfsdfasdf\n");
		}
		// If at least one cell is not zero
		if (zero_check != 0) {
			different = memcmp(*local_grid, *next_local_grid, block_dimension * block_dimension * sizeof(int));
			zero_check = 1;
		}
//		// If it is different and not zero
//		if (different != 0 && zero_check != 0) {
//			different = 1;
////			printf("rank %d: different\n", rank);
//		} else {
//			different = 0;
//			//todo send alert
////			printf("rank %d: same\n", rank);
//		}
		int keep_looping;
		// If it is different and not zero
		if (different != 0 && zero_check != 0) {
			keep_looping = 1;
		} else{
			keep_looping = 0;
		}
		MPI_Gather(&keep_looping, 1, MPI_INT, dif_array, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (rank == 0) {
			continue_next_gen = 0;
			for (p = 0; p < num_of_proc; p++) {
//				printf("%d ", dif_array[p]);
				if (dif_array[p] == 1) {
					printf("process %d has differences\n", p);
					continue_next_gen = 1;
					break;
				}

			}
//			printf("\n");
			printf("continue_next_gen: %d\n", continue_next_gen);
		}
		MPI_Bcast(&continue_next_gen, 1, MPI_INT, 0, MPI_COMM_WORLD);
		printf("rank %d: continue_next_gen: %d\n", rank, continue_next_gen);

		MPI_Gatherv(&(next_local_grid[0][0]), block_dimension * block_dimension, MPI_INT, ptr_to_grid, sendcounts, displs, block_type_1, 0, MPI_COMM_WORLD);

		printGrid2(local_grid, block_dimension, rank, 0);
		int** temp;
		temp = &(*next_local_grid);
		next_local_grid = &(*local_grid);
		local_grid = temp;


		for (i = 0; i < block_dimension; ++i) {
			for (j = 0; j < block_dimension; ++j) {
				next_local_grid[i][j] = 0;
			}
		}

		//	printGrid(next_local_grid, dimension, rank, 1);
		if (rank == 0) {
			printGrid(grid, dimension, rank, 1);
		}
	}

//	if (continue_next_gen == 0) {
//		mpi send();
//	} else {
//
//	}

//	} else {
//		MPI_Isend(&different, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
//	}

//	MPI_Recv(&top_left_value, 1, MPI_INT, top_left_rank, 0, MPI_COMM_WORLD, &status);

	/* Gather all processed blocks to process 0 */
//	MPI_Gatherv(&(next_local_grid[0][0]), block_dimension * block_dimension, MPI_INT, ptr_to_grid, sendcounts, displs, block_type_1, 0, MPI_COMM_WORLD);
	/* Free local grids */
	freeGrid(&local_grid);
	freeGrid(&next_local_grid);
	MPI_Type_free(&block_type_1);
	MPI_Type_free(&block_type_2);
	MPI_Type_free(&for_columns);

//	if (rank == 0) {
//		printGrid(grid, dimension, rank, 1);
//	}
	/* Free grid */
	if (rank == 0) {
		freeGrid(&grid);
	}

	return 0;
}

void calculateInnerCells(int block_dimension, int **local_grid, int **next_local_grid) {
	int i, j;
	/* To calculate the middle cells we have to ignore the first row & column and the last row & column */
	/* Ignore the first row (i == 0) and the last row (i == block_dimension - 1) */
	int alive_neighbors;
	for (i = 1; i < block_dimension - 1; i++) {
		/* Ignore the first column (j == 0) and the last column (j == block_dimension - 1) */
		for (j = 1; j < block_dimension - 1; j++) {
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

			/* If it is empty space */
			if (local_grid[i][j] == 0) {
				/* If there are exact 3 neighbors create a new cell */
				if (alive_neighbors == 3) {
					next_local_grid[i][j] = 1;
				}
			}
			/* If already lives a cell */
			else {
				/* Determine if the cell lives or dies in next round */
				/* Store the new value to the next_local_grid */
				/* DIE */
				if (alive_neighbors < 2 || alive_neighbors > 3) {
					next_local_grid[i][j] = 0;
				}
				/* LIVE */
				else {
					next_local_grid[i][j] = 1;
				}
			}
		}
	}
}

void calculateEdgeCells(int block_dimension, int **local_grid,
		int **next_local_grid, int *top_buff, int *right_buff, int *bot_buff,
		int *left_buff, int top_left_value, int top_right_value,
		int bot_left_value, int bot_right_value) {
	int k;
	int alive_neighbors = 0;
	for (k = 1; k < block_dimension - 1; k++) {
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

		/* If it is empty space */
		if (local_grid[0][k] == 0) {
			/* If there are exact 3 neighbors create a new cell */
			if (alive_neighbors == 3) {
				next_local_grid[0][k] = 1;
			}
		}
		/* If already lives a cell */
		else {
			/* Determine if the cell lives or dies in next round */
			/* Store the new value to the next_local_grid */
			/* DIE */
			if (alive_neighbors < 2 || alive_neighbors > 3) {
				next_local_grid[0][k] = 0;
			}
			/* LIVE */
			else {
				next_local_grid[0][k] = 1;
			}
		}
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
		alive_neighbors += local_grid[block_dimension - 1][k + 1];
		/* Bot right neighbor */
		alive_neighbors += local_grid[block_dimension - 2][k + 1];
		/* Bot neighbor */
		alive_neighbors += local_grid[block_dimension - 2][k];
		/* Bot left neighbor */
		alive_neighbors += local_grid[block_dimension - 2][k - 1];
		/* Left neighbor */
		alive_neighbors += local_grid[block_dimension - 1][k - 1];

		/* If it is empty space */
		if (local_grid[block_dimension - 1][k] == 0) {
			/* If there are exact 3 neighbors create a new cell */
			if (alive_neighbors == 3) {
				next_local_grid[block_dimension - 1][k] = 1;
			}
		}
		/* If already lives a cell */
		else {
			/* Determine if the cell lives or dies in next round */
			/* Store the new value to the next_local_grid */
			/* DIE */
			if (alive_neighbors < 2 || alive_neighbors > 3) {
				next_local_grid[block_dimension - 1][k] = 0;
			}
			/* LIVE */
			else {
				next_local_grid[block_dimension - 1][k] = 1;
			}
		}
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

		/* If it is empty space */
		if (local_grid[k][0] == 0) {
			/* If there are exact 3 neighbors create a new cell */
			if (alive_neighbors == 3) {
				next_local_grid[k][0] = 1;
			}
		}
		/* If already lives a cell */
		else {
			/* Determine if the cell lives or dies in next round */
			/* Store the new value to the next_local_grid */
			/* DIE */
			if (alive_neighbors < 2 || alive_neighbors > 3) {
				next_local_grid[k][0] = 0;
			}
			/* LIVE */
			else {
				next_local_grid[k][0] = 1;
			}
		}
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
		alive_neighbors += local_grid[k + 1][block_dimension - 1];
		/* Bot right neighbor */
		alive_neighbors += local_grid[k + 1][block_dimension - 2];
		/* Bot neighbor */
		alive_neighbors += local_grid[k][block_dimension - 2];
		/* Bot left neighbor */
		alive_neighbors += local_grid[k - 1][block_dimension - 2];
		/* Left neighbor */
		alive_neighbors += local_grid[k - 1][block_dimension - 1];

		/* If it is empty space */
		if (local_grid[k][block_dimension - 1] == 0) {
			/* If there are exact 3 neighbors create a new cell */
			if (alive_neighbors == 3) {
				next_local_grid[k][block_dimension - 1] = 1;
			}
		}
		/* If already lives a cell */
		else {
			/* Determine if the cell lives or dies in next round */
			/* Store the new value to the next_local_grid */
			/* DIE */
			if (alive_neighbors < 2 || alive_neighbors > 3) {
				next_local_grid[k][block_dimension - 1] = 0;
			}
			/* LIVE */
			else {
				next_local_grid[k][block_dimension - 1] = 1;
			}
		}
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

	/* If it is empty space */
	if (local_grid[0][0] == 0) {
		/* If there are exact 3 neighbors create a new cell */
		if (alive_neighbors == 3) {
			next_local_grid[0][0] = 1;
		}
	}
	/* If already lives a cell */
	else {
		/* Determine if the cell lives or dies in next round */
		/* Store the new value to the next_local_grid */
		/* DIE */
		if (alive_neighbors < 2 || alive_neighbors > 3) {
			next_local_grid[0][0] = 0;
		}
		/* LIVE */
		else {
			next_local_grid[0][0] = 1;
		}
	}

	/* TOP RIGHT CELL */
	alive_neighbors = 0;
	/* Calculate the value of the current cell according to its neighbors */
	/* Top left neighbor (the value is borrowed by other process) */
	alive_neighbors += top_buff[block_dimension - 2];
	/* Top neighbor (the value is borrowed by other process) */
	alive_neighbors += top_buff[block_dimension - 1];
	/* Top right neighbor (the value is borrowed by other process) */
	alive_neighbors += top_right_value;
	/* Right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[0];
	/* Bot right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[1];
	/* Bot neighbor */
	alive_neighbors += local_grid[1][block_dimension - 1];
	/* Bot left neighbor */
	alive_neighbors += local_grid[1][block_dimension - 2];
	/* Left neighbor */
	alive_neighbors += local_grid[0][block_dimension - 2]; /* edw exei allagh */
	/* If it is empty space */
	if (local_grid[0][block_dimension - 1] == 0) {
		/* If there are exact 3 neighbors create a new cell */
		if (alive_neighbors == 3) {
			next_local_grid[0][block_dimension - 1] = 1;
		}
	}
	/* If already lives a cell */
	else {
		/* Determine if the cell lives or dies in next round */
		/* Store the new value to the next_local_grid */
		/* DIE */
		if (alive_neighbors < 2 || alive_neighbors > 3) {
			next_local_grid[0][block_dimension - 1] = 0;
		}
		/* LIVE */
		else {
			next_local_grid[0][block_dimension - 1] = 1;
		}
	}
	/* BOTTOM RIGHT CELL */
	alive_neighbors = 0;
	/* Bot right cell
	 * (this is special case, to calculate next round we need bot_right_value) */
	/* Calculate the value of the current cell according to its neighbors */
	/* Top left neighbor */
	alive_neighbors += local_grid[block_dimension - 2][block_dimension - 2];
	/* Top neighbor */
	alive_neighbors += local_grid[block_dimension - 2][block_dimension - 1];
	/* Top right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[block_dimension - 2];
	/* Right neighbor (the value is borrowed by other process) */
	alive_neighbors += right_buff[block_dimension - 1];
	/* Bot right neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_right_value;
	/* Bot neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[block_dimension - 1];
	/* Bot left neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[block_dimension - 2];
	/* Left neighbor */
	alive_neighbors += local_grid[block_dimension - 1][block_dimension - 2];
	/* If it is empty space */
	if (local_grid[block_dimension - 1][block_dimension - 1] == 0) {
		/* If there are exact 3 neighbors create a new cell */
		if (alive_neighbors == 3) {
			next_local_grid[block_dimension - 1][block_dimension - 1] = 1;
		}
	}
	/* If already lives a cell */
	else {
		/* Determine if the cell lives or dies in next round */
		/* Store the new value to the next_local_grid */
		/* DIE */
		if (alive_neighbors < 2 || alive_neighbors > 3) {
			next_local_grid[block_dimension - 1][block_dimension - 1] = 0;
		}
		/* LIVE */
		else {
			next_local_grid[block_dimension - 1][block_dimension - 1] = 1;
		}
	}
	/* BOTTOM LEFT CELL */
	alive_neighbors = 0;
	/* Calculate the value of the current cell according to its neighbors */
	/* Top left neighbor (the value is borrowed by other process) */
	alive_neighbors += left_buff[block_dimension - 2]; /* edw exei allagh */
	/* Top neighbor */
	alive_neighbors += local_grid[block_dimension - 2][0];
	/* Top right neighbor */
	alive_neighbors += local_grid[block_dimension - 2][1];
	/* Right neighbor */
	alive_neighbors += local_grid[block_dimension - 1][1];
	/* Bot right neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[1];
	/* Bot neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_buff[0];
	/* Bot left neighbor (the value is borrowed by other process) */
	alive_neighbors += bot_left_value;
	/* Left neighbor (the value is borrowed by other process) */
	alive_neighbors += left_buff[block_dimension - 1];
	/* If it is empty space */
	if (local_grid[block_dimension - 1][0] == 0) {
		/* If there are exact 3 neighbors create a new cell */
		if (alive_neighbors == 3) {
			next_local_grid[block_dimension - 1][0] = 1;
		}
	}
	/* If already lives a cell */
	else {
		/* Determine if the cell lives or dies in next round */
		/* Store the new value to the next_local_grid */
		/* DIE */
		if (alive_neighbors < 2 || alive_neighbors > 3) {
			next_local_grid[block_dimension - 1][0] = 0;
		}
		/* LIVE */
		else {
			next_local_grid[block_dimension - 1][0] = 1;
		}
	}
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
