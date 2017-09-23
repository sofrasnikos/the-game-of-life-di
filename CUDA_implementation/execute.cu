#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "execute.h"

void execute(int dimension, int loops, char *input_file, int prints_enabled) {
	int i, j, p;
	int nblocks;
	// char grid[dimension][dimension];
	char *grid, *gpu_grid_1, *gpu_grid_2;
	createGrid(&grid, dimension);
	initGrid(grid, dimension);
	grid[55] = 1;
	grid[45] = 1;
	grid[35] = 1;
	grid[56] = 1;
	grid[54] = 1;
	
	// if (input_file != NULL) {
	// 	readGrid(grid, input_file, dimension);
	// }

	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {

			printf("%d", grid[i * dimension + j]);
		}
		printf("\n");
	}

	printf("\nEND OF PRINT\n");
	fflush(stdout);

	//todo
	int dir_stat = mkdir("outputs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (dir_stat != 0 && errno != EEXIST) {
	printf("mkdir error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	cudaMalloc((void **) &gpu_grid_1, dimension * dimension * sizeof(char));
	cudaMalloc((void **) &gpu_grid_2, dimension * dimension * sizeof(char));
	cudaMemcpy(gpu_grid_1, grid, dimension * dimension * sizeof(char), cudaMemcpyHostToDevice);
	kernel<<<128, (dimension * dimension / 128) + 1>>>(gpu_grid_1, gpu_grid_2, dimension);
	cudaMemcpy(grid, gpu_grid_2, dimension * dimension * sizeof(char), cudaMemcpyDeviceToHost);
	
	for(i = 0; i < dimension; i++) {
		for(j = 0; j < dimension; j++) {
			printf("%d", grid[i * dimension + j]);
		}
		printf("\n");
	}

	printf("\nEND OF PRINT 2\n");
	fflush(stdout);
	// nblocks = ()
	freeGrid(&grid);
	printf("Exiting...\n");
}

__global__ void kernel(char *grid_1, char *grid_2, int dimension) {

	int ix = (blockIdx.x * blockDim.x + threadIdx.x) % (dimension * dimension);
	int iy = (blockIdx.y * blockDim.y + threadIdx.y) % (dimension * dimension);
	int idx	= (iy * dimension + ix) % (dimension * dimension);

	int i = idx / dimension;
	int j = idx % dimension;

	int top_offset = ((i + dimension - 1) % dimension) * dimension;
	int bot_offset = ((i + 1) % dimension) * dimension;
	int right_offset = (j + 1) % dimension;
	int left_offset = (j - 1 + dimension) % dimension;

	int top = top_offset + j;
	int top_right = top_offset + right_offset;
	int top_left = top_offset + left_offset;

	int bot = bot_offset + j;
	int bot_right = bot_offset + right_offset;
	int bot_left = bot_offset + left_offset;

	int right = i * dimension + right_offset;
	int left = i * dimension + left_offset;

	int alive_neighbors = 0;
	alive_neighbors += grid_1[top_left];
	alive_neighbors += grid_1[top];
	alive_neighbors += grid_1[top_right];
	alive_neighbors += grid_1[right];
	alive_neighbors += grid_1[bot_right];
	alive_neighbors += grid_1[bot];
	alive_neighbors += grid_1[bot_left];
	alive_neighbors += grid_1[left];
	
	// if (idx == 66){
	// 	for (i = 0; i < dimension; i++) {
	// 		for (j = 0; j < dimension; j++) {

	// 			printf("%d", grid_1[i * dimension + j]);
	// 		}
	// 		printf("\n");
	// 	}
	// }
	printf("idx = %d i = %d j = %d\n", idx, i ,j);

	if (i == 5 && j == 5){
		
		printf("alive alive_neighbors: %d\n", alive_neighbors);
		printf("top: %d\n", top);
		printf("top_right: %d\n", top_right);
		printf("top_left: %d\n", top_left);
	 	printf("bot: %d\n", bot);
	 	printf("bot_right: %d\n", bot_right);
	 	printf("bot_left: %d\n", bot_left);
	 	printf("right: %d\n", right);
	 	printf("left: %d\n", left);
	}
	// grid_2[i * dimension + j] = deadOrAlive(alive_neighbors, grid_1[i * dimension + j]);
	int pos = i * dimension + j;
	int status = grid_1[idx];

	// printf("status %d\n", grid_1[idx]);

	if (status == 0) {
		/* If there are exactly 3 neighbors create a new cell */
		if (alive_neighbors == 3) {
			/* CREATE NEW CELL */
			grid_2[idx] = 1;
		}
		/* Leave it empty */
		else {
			grid_2[idx] = 0;
		}
	}
	/* If a cell already lives */
	else {
		/* Determine if the cell lives or dies in next round */
		/* DIE */
		if (alive_neighbors < 2 || alive_neighbors > 3) {
			grid_2[idx] = 0;
		}
		/* LIVE */
		else {
			grid_2[idx] = 1;
		}
	}
}
