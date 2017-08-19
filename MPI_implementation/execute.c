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
	/* Create another local grid which will contain the values after the calculations */
	CreateGrid(&next_local_grid, block_dimension);
	//INIT
//	for (i = 0; i < block_dimension; i++) {
//		for (j = 0; j < block_dimension; j++) {
//			next_local_grid[i][j] = 0;
//		}
//	}

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

	/* Scatter the grid */
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

	/* Print grid */
	if (rank == 0) {
		PrintGrid(grid, dimension, rank, 1);
	}
	//todo to parakatw printf uparxei gia dieukolhnsh sto debugging.
	//isws na mhn xreiazetai sth teleutaia ekdosh
	/* Print local blocks(subgrids) */
	int p;
	for (p = 0; p < num_of_proc; p++) {
		if (rank == p) {
			PrintGrid(local_grid, block_dimension, rank, 0);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	int coords[2];
	MPI_Cart_coords(comm, rank, 2, coords);

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

	int top_buff[block_dimension], bot_buff[block_dimension],
			left_buff[block_dimension], right_buff[block_dimension];
	int top_left_value, top_right_value, bot_left_value, bot_right_value;
	// Send to all eight neighbors
	MPI_Isend(&local_grid[0][0], block_dimension, MPI_INT, top_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[block_dimension - 1][0], block_dimension, MPI_INT,
			bot_rank, 0,
			MPI_COMM_WORLD, &request);
	int temp_left_buff[block_dimension], temp_right_buff[block_dimension];
	// Left and right are special cases because we have to create a buffer since the values are in a column
	for (i = 0; i < block_dimension; i++) {
		temp_left_buff[i] = local_grid[i][0];
		temp_right_buff[i] = local_grid[i][block_dimension - 1];
	}
	MPI_Isend(temp_left_buff, block_dimension, MPI_INT, left_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(temp_right_buff, block_dimension, MPI_INT, right_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[0][0], 1, MPI_INT, top_left_rank, 0, MPI_COMM_WORLD,
			&request);
	MPI_Isend(&local_grid[0][block_dimension - 1], 1, MPI_INT, top_right_rank,
			0,
			MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[block_dimension - 1][0], 1, MPI_INT, bot_left_rank, 0,
	MPI_COMM_WORLD, &request);
	MPI_Isend(&local_grid[block_dimension - 1][block_dimension - 1], 1, MPI_INT,
			bot_right_rank, 0, MPI_COMM_WORLD, &request);

	// Calculate the middle cells while waiting to receive from neighbors
	CalculateInnerCells(block_dimension, local_grid, next_local_grid);

	if (rank == 0) {
		PrintGrid(next_local_grid, block_dimension, rank, 0);
	}
	/* Receive from every neighbor */
	MPI_Recv(&bot_buff, block_dimension, MPI_INT, bot_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&top_buff, block_dimension, MPI_INT, top_rank, 0, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&right_buff, block_dimension, MPI_INT, right_rank, 0,
	MPI_COMM_WORLD, &status);
	MPI_Recv(&left_buff, block_dimension, MPI_INT, left_rank, 0, MPI_COMM_WORLD,
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
		for (i = 0; i < block_dimension; i++) {
			if (bot_buff[i] == 1) {
				printf("*");
			} else {
				printf(".");
			}
		}
		printf("\n");
		printf("I am rank %d. Received from top (%d):", rank, top_rank);
		for (i = 0; i < block_dimension; i++) {
			if (top_buff[i] == 1) {
				printf("*");
			} else {
				printf(".");
			}
		}
		printf("\n");
		printf("I am rank %d. Received from right (%d):\n", rank, right_rank);
		for (i = 0; i < block_dimension; i++) {
			if (right_buff[i] == 1) {
				printf("*\n");
			} else {
				printf(".\n");
			}
		}
		printf("I am rank %d. Received from left (%d):\n", rank, left_rank);
		for (i = 0; i < block_dimension; i++) {
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
	CalculateEdgeCells(block_dimension, local_grid, next_local_grid, top_buff,
			right_buff, bot_buff, left_buff, top_left_value, top_right_value,
			bot_left_value, bot_right_value);
	

	//todo erase
	//print grids gia dieukolunhsh sto debugging
	for (p = 0; p < num_of_proc; p++) {
		if (rank == p) {
			PrintGrid(next_local_grid, block_dimension, rank, 0);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
//	CalculateEdgeCellsOpt(block_dimension, local_grid, next_local_grid, top_buff,
//				right_buff, bot_buff, left_buff, top_left_value, top_right_value,
//				bot_left_value, bot_right_value);
//	//todo erase
//	//print grids gia dieukolunhsh sto debugging
//	for (p = 0; p < num_of_proc; p++) {
//		if (rank == p) {
//			PrintGrid(next_local_grid, block_dimension, rank, 0);
//		}
//		MPI_Barrier(MPI_COMM_WORLD);
//	}

	/* Gather all processed blocks to process 0 */
	MPI_Gatherv(&(next_local_grid[0][0]), block_dimension * block_dimension,
	MPI_INT, ptr_to_grid, sendcounts, displs, block_type_1, 0,
	MPI_COMM_WORLD);

	/* Free local grids */
	FreeGrid(&local_grid);
	FreeGrid(&next_local_grid);
	MPI_Type_free(&block_type_1);
	MPI_Type_free(&block_type_2);

	if (rank == 0) {
		PrintGrid(grid, dimension, rank, 1);
	}
	/* Free grid */
	if (rank == 0) {
		FreeGrid(&grid);
	}

	return 0;
}

void CalculateInnerCells(int block_dimension, int **local_grid,
		int **next_local_grid) {
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

void CalculateEdgeCells(int block_dimension, int **local_grid,
		int **next_local_grid, int *top_buff, int *right_buff, int *bot_buff,
		int *left_buff, int top_left_value, int top_right_value,
		int bot_left_value, int bot_right_value) {
	int i, j;

	for (i = 0; i < block_dimension; i++) {
		for (j = 0; j < block_dimension; j++) {
			int alive_neighbors = 0;
			int modify_cell_value = 0;
			/* Iterate all columns in the first & last row */
			if (i == 0 || i == (block_dimension - 1)) {
				/* Top left cell
				 * (this is special case, to calculate next round we need top_left_value) */
				if (i == 0 && j == 0) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor (the value is borrowed by other process) */
					alive_neighbors += top_left_value;
					/* Top neighbor (the value is borrowed by other process) */
					alive_neighbors += top_buff[j];
					/* Top right neighbor (the value is borrowed by other process) */
					alive_neighbors += top_buff[j + 1];
					/* Right neighbor */
					alive_neighbors += local_grid[i][j + 1];
					/* Bot right neighbor */
					alive_neighbors += local_grid[i + 1][j + 1];
					/* Bot neighbor */
					alive_neighbors += local_grid[i + 1][j];
					/* Bot left neighbor (the value is borrowed by other process) */
					alive_neighbors += left_buff[i + 1];
					/* Left neighbor (the value is borrowed by other process) */
					alive_neighbors += left_buff[i];
					modify_cell_value = 1;
				}
				/* Top right cell
				 * (this is special case, to calculate next round we need top_right_value) */
				else if (i == 0 && j == (block_dimension - 1)) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor (the value is borrowed by other process) */
					alive_neighbors += top_buff[j - 1];
					/* Top neighbor (the value is borrowed by other process) */
					alive_neighbors += top_buff[j];
					/* Top right neighbor (the value is borrowed by other process) */
					alive_neighbors += top_right_value;
					/* Right neighbor (the value is borrowed by other process) */
					alive_neighbors += right_buff[i];
					/* Bot right neighbor (the value is borrowed by other process) */
					alive_neighbors += right_buff[i + 1];
					/* Bot neighbor */
					alive_neighbors += local_grid[i + 1][j];
					/* Bot left neighbor */
					alive_neighbors += local_grid[i + 1][j - 1];
					/* Left neighbor */
					alive_neighbors += local_grid[i][j - 1];
					modify_cell_value = 1;
				}
				/* Bot left cell
				 * (this is special case, to calculate next round we need bot_left_value) */
				else if (i == (block_dimension - 1) && j == 0) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor (the value is borrowed by other process) */
					alive_neighbors += left_buff[j - 1];
					/* Top neighbor */
					alive_neighbors += local_grid[i - 1][j];
					/* Top right neighbor */
					alive_neighbors += local_grid[i - 1][j + 1];
					/* Right neighbor */
					alive_neighbors += local_grid[i][j + 1];
					/* Bot right neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_buff[j + 1];
					/* Bot neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_buff[j];
					/* Bot left neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_left_value;
					/* Left neighbor (the value is borrowed by other process) */
					alive_neighbors += left_buff[i];
					modify_cell_value = 1;
				}
				/* Bot right cell
				 * (this is special case, to calculate next round we need bot_right_value) */
				else if (i == (block_dimension - 1)
						&& j == (block_dimension - 1)) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor */
					alive_neighbors += local_grid[i - 1][j - 1];
					/* Top neighbor */
					alive_neighbors += local_grid[i - 1][j];
					/* Top right neighbor (the value is borrowed by other process) */
					alive_neighbors += right_buff[i - 1];
					/* Right neighbor (the value is borrowed by other process) */
					alive_neighbors += right_buff[i];
					/* Bot right neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_right_value;
					/* Bot neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_buff[j];
					/* Bot left neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_buff[j - 1];
					/* Left neighbor */
					alive_neighbors += local_grid[i][j - 1];
					modify_cell_value = 1;
				}
				/* Top row without the "corner cells" */
				else if (i == 0 && j > 0 && j < (block_dimension - 1)) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor (the value is borrowed by other process) */
					alive_neighbors += top_buff[j - 1];
					/* Top neighbor (the value is borrowed by other process) */
					alive_neighbors += top_buff[j];
					/* Top right neighbor (the value is borrowed by other process) */
					alive_neighbors += top_buff[j + 1];
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
					modify_cell_value = 1;
				}
				/* Bot row without the "corner cells" */
				else if (i == (block_dimension - 1) && j > 0
						&& j < (block_dimension - 1)) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor */
					alive_neighbors += local_grid[i - 1][j - 1];
					/* Top neighbor */
					alive_neighbors += local_grid[i - 1][j];
					/* Top right neighbor */
					alive_neighbors += local_grid[i - 1][j + 1];
					/* Right neighbor */
					alive_neighbors += local_grid[i][j + 1];
					/* Bot right neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_buff[j + 1];
					/* Bot neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_buff[j];
					/* Bot left neighbor (the value is borrowed by other process) */
					alive_neighbors += bot_buff[j - 1];
					/* Left neighbor */
					alive_neighbors += local_grid[i][j - 1];
					modify_cell_value = 1;
				}
			}
			/* In the rows that are between the first & last row of the grid,
			 * choose the cells that are located in the first and the last column
			 * (with this way we ignore the inner cells of the grid) */
			else {
				/* First column */
				if (j == 0) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor (the value is borrowed by other process) */
					alive_neighbors += left_buff[i - 1];
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
					/* Bot left neighbor (the value is borrowed by other process) */
					alive_neighbors += left_buff[i + 1];
					/* Left neighbor (the value is borrowed by other process) */
					alive_neighbors += left_buff[i];
					modify_cell_value = 1;
				}
				/* Last column */
				else if (j == block_dimension - 1) {
					/* Calculate the value of the current cell according to its neighbors */
					/* Top left neighbor */
					alive_neighbors += local_grid[i - 1][j - 1];
					/* Top neighbor */
					alive_neighbors += local_grid[i - 1][j];
					/* Top right neighbor (the value is borrowed by other process) */
					alive_neighbors += right_buff[i - 1];
					/* Right neighbor (the value is borrowed by other process) */
					alive_neighbors += right_buff[i];
					/* Bot right neighbor (the value is borrowed by other process) */
					alive_neighbors += right_buff[i + 1];
					/* Bot neighbor */
					alive_neighbors += local_grid[i + 1][j];
					/* Bot left neighbor */
					alive_neighbors += local_grid[i + 1][j - 1];
					/* Left neighbor */
					alive_neighbors += local_grid[i][j - 1];
					modify_cell_value = 1;
				}
			}
			if (modify_cell_value == 1) {
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
}

void CalculateEdgeCellsOpt(int block_dimension, int **local_grid,
		int **next_local_grid, int *top_buff, int *right_buff, int *bot_buff,
		int *left_buff, int top_left_value, int top_right_value,
		int bot_left_value, int bot_right_value) {
	int i, j;
	
	for (i = 1; i < block_dimension - 1; i++) {
		/* Top row without the "corner cells" */
		int alive_neighbors = 0;
		/* Calculate the value of the current cell according to its neighbors */
		/* Top left neighbor (the value is borrowed by other process) */
		alive_neighbors += top_buff[i - 1];
		/* Top neighbor (the value is borrowed by other process) */
		alive_neighbors += top_buff[i];
		/* Top right neighbor (the value is borrowed by other process) */
		alive_neighbors += top_buff[i + 1];
		/* Right neighbor */
		alive_neighbors += local_grid[0][i + 1];
		/* Bot right neighbor */
		alive_neighbors += local_grid[1][i + 1];
		/* Bot neighbor */
		alive_neighbors += local_grid[1][i];
		/* Bot left neighbor */
		alive_neighbors += local_grid[1][i - 1];
		/* Left neighbor */
		alive_neighbors += local_grid[0][i - 1];
		
		/* If it is empty space */
		if (local_grid[0][i] == 0) {
			/* If there are exact 3 neighbors create a new cell */
			if (alive_neighbors == 3) {
				next_local_grid[0][i] = 1;
			}
		}
		/* If already lives a cell */
		else {
			/* Determine if the cell lives or dies in next round */
			/* Store the new value to the next_local_grid */
			/* DIE */
			if (alive_neighbors < 2 || alive_neighbors > 3) {
				next_local_grid[0][i] = 0;
			}
			/* LIVE */
			else {
				next_local_grid[0][i] = 1;
			}
		}
	}
		
		
//	for (i = 0; i < block_dimension; i++) {
//		for (j = 0; j < block_dimension; j++) {
//			int alive_neighbors = 0;
//			int modify_cell_value = 0;
//			/* Iterate all columns in the first & last row */
//			if (i == 0 || i == (block_dimension - 1)) {
//				/* Top left cell
//				 * (this is special case, to calculate next round we need top_left_value) */
//				if (i == 0 && j == 0) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_left_value;
//					/* Top neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_buff[j];
//					/* Top right neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_buff[j + 1];
//					/* Right neighbor */
//					alive_neighbors += local_grid[i][j + 1];
//					/* Bot right neighbor */
//					alive_neighbors += local_grid[i + 1][j + 1];
//					/* Bot neighbor */
//					alive_neighbors += local_grid[i + 1][j];
//					/* Bot left neighbor (the value is borrowed by other process) */
//					alive_neighbors += left_buff[i + 1];
//					/* Left neighbor (the value is borrowed by other process) */
//					alive_neighbors += left_buff[i];
//					modify_cell_value = 1;
//				}
//				/* Top right cell
//				 * (this is special case, to calculate next round we need top_right_value) */
//				else if (i == 0 && j == (block_dimension - 1)) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_buff[j - 1];
//					/* Top neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_buff[j];
//					/* Top right neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_right_value;
//					/* Right neighbor (the value is borrowed by other process) */
//					alive_neighbors += right_buff[i];
//					/* Bot right neighbor (the value is borrowed by other process) */
//					alive_neighbors += right_buff[i + 1];
//					/* Bot neighbor */
//					alive_neighbors += local_grid[i + 1][j];
//					/* Bot left neighbor */
//					alive_neighbors += local_grid[i + 1][j - 1];
//					/* Left neighbor */
//					alive_neighbors += local_grid[i][j - 1];
//					modify_cell_value = 1;
//				}
//				/* Bot left cell
//				 * (this is special case, to calculate next round we need bot_left_value) */
//				else if (i == (block_dimension - 1) && j == 0) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor (the value is borrowed by other process) */
//					alive_neighbors += left_buff[j - 1];
//					/* Top neighbor */
//					alive_neighbors += local_grid[i - 1][j];
//					/* Top right neighbor */
//					alive_neighbors += local_grid[i - 1][j + 1];
//					/* Right neighbor */
//					alive_neighbors += local_grid[i][j + 1];
//					/* Bot right neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_buff[j + 1];
//					/* Bot neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_buff[j];
//					/* Bot left neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_left_value;
//					/* Left neighbor (the value is borrowed by other process) */
//					alive_neighbors += left_buff[i];
//					modify_cell_value = 1;
//				}
//				/* Bot right cell
//				 * (this is special case, to calculate next round we need bot_right_value) */
//				else if (i == (block_dimension - 1)
//						&& j == (block_dimension - 1)) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor */
//					alive_neighbors += local_grid[i - 1][j - 1];
//					/* Top neighbor */
//					alive_neighbors += local_grid[i - 1][j];
//					/* Top right neighbor (the value is borrowed by other process) */
//					alive_neighbors += right_buff[i - 1];
//					/* Right neighbor (the value is borrowed by other process) */
//					alive_neighbors += right_buff[i];
//					/* Bot right neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_right_value;
//					/* Bot neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_buff[j];
//					/* Bot left neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_buff[j - 1];
//					/* Left neighbor */
//					alive_neighbors += local_grid[i][j - 1];
//					modify_cell_value = 1;
//				}
//				/* Top row without the "corner cells" */
//				else if (i == 0 && j > 0 && j < (block_dimension - 1)) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_buff[j - 1];
//					/* Top neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_buff[j];
//					/* Top right neighbor (the value is borrowed by other process) */
//					alive_neighbors += top_buff[j + 1];
//					/* Right neighbor */
//					alive_neighbors += local_grid[i][j + 1];
//					/* Bot right neighbor */
//					alive_neighbors += local_grid[i + 1][j + 1];
//					/* Bot neighbor */
//					alive_neighbors += local_grid[i + 1][j];
//					/* Bot left neighbor */
//					alive_neighbors += local_grid[i + 1][j - 1];
//					/* Left neighbor */
//					alive_neighbors += local_grid[i][j - 1];
//					modify_cell_value = 1;
//				}
//				/* Bot row without the "corner cells" */
//				else if (i == (block_dimension - 1) && j > 0
//						&& j < (block_dimension - 1)) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor */
//					alive_neighbors += local_grid[i - 1][j - 1];
//					/* Top neighbor */
//					alive_neighbors += local_grid[i - 1][j];
//					/* Top right neighbor */
//					alive_neighbors += local_grid[i - 1][j + 1];
//					/* Right neighbor */
//					alive_neighbors += local_grid[i][j + 1];
//					/* Bot right neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_buff[j + 1];
//					/* Bot neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_buff[j];
//					/* Bot left neighbor (the value is borrowed by other process) */
//					alive_neighbors += bot_buff[j - 1];
//					/* Left neighbor */
//					alive_neighbors += local_grid[i][j - 1];
//					modify_cell_value = 1;
//				}
//			}
//			/* In the rows that are between the first & last row of the grid,
//			 * choose the cells that are located in the first and the last column
//			 * (with this way we ignore the inner cells of the grid) */
//			else {
//				/* First column */
//				if (j == 0) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor (the value is borrowed by other process) */
//					alive_neighbors += left_buff[i - 1];
//					/* Top neighbor */
//					alive_neighbors += local_grid[i - 1][j];
//					/* Top right neighbor */
//					alive_neighbors += local_grid[i - 1][j + 1];
//					/* Right neighbor */
//					alive_neighbors += local_grid[i][j + 1];
//					/* Bot right neighbor */
//					alive_neighbors += local_grid[i + 1][j + 1];
//					/* Bot neighbor */
//					alive_neighbors += local_grid[i + 1][j];
//					/* Bot left neighbor (the value is borrowed by other process) */
//					alive_neighbors += left_buff[i + 1];
//					/* Left neighbor (the value is borrowed by other process) */
//					alive_neighbors += left_buff[i];
//					modify_cell_value = 1;
//				}
//				/* Last column */
//				else if (j == block_dimension - 1) {
//					/* Calculate the value of the current cell according to its neighbors */
//					/* Top left neighbor */
//					alive_neighbors += local_grid[i - 1][j - 1];
//					/* Top neighbor */
//					alive_neighbors += local_grid[i - 1][j];
//					/* Top right neighbor (the value is borrowed by other process) */
//					alive_neighbors += right_buff[i - 1];
//					/* Right neighbor (the value is borrowed by other process) */
//					alive_neighbors += right_buff[i];
//					/* Bot right neighbor (the value is borrowed by other process) */
//					alive_neighbors += right_buff[i + 1];
//					/* Bot neighbor */
//					alive_neighbors += local_grid[i + 1][j];
//					/* Bot left neighbor */
//					alive_neighbors += local_grid[i + 1][j - 1];
//					/* Left neighbor */
//					alive_neighbors += local_grid[i][j - 1];
//					modify_cell_value = 1;
//				}
//			}
//			if (modify_cell_value == 1) {
//				/* If it is empty space */
//				if (local_grid[i][j] == 0) {
//					/* If there are exact 3 neighbors create a new cell */
//					if (alive_neighbors == 3) {
//						next_local_grid[i][j] = 1;
//					}
//				}
//				/* If already lives a cell */
//				else {
//					/* Determine if the cell lives or dies in next round */
//					/* Store the new value to the next_local_grid */
//					/* DIE */
//					if (alive_neighbors < 2 || alive_neighbors > 3) {
//						next_local_grid[i][j] = 0;
//					}
//					/* LIVE */
//					else {
//						next_local_grid[i][j] = 1;
//					}
//				}
//			}
//		}
//	}
}
