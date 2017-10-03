#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "execute.h"

/* This variable is used to check if there should be next generation */
__device__ int diff = 0;

void execute(int dimension, int loops, char *input_file, int prints_enabled) {

	int local_diff;
	char *grid, *gpu_grid_1, *gpu_grid_2;

	struct timeval time_1, time_2;
	gettimeofday(&time_1, 0);

	createGrid(&grid, dimension);
	if (input_file != NULL) {
		readGrid(grid, input_file, dimension);
	} else {
		initGrid(grid, dimension);
	}
	
	if (prints_enabled == 1) {
		int dir_stat = mkdir("outputs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (dir_stat != 0 && errno != EEXIST) {
		printf("mkdir error %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		printGrid(grid, dimension);
	}

	cudaMalloc((void **) &gpu_grid_1, dimension * dimension * sizeof(char));
	cudaMalloc((void **) &gpu_grid_2, dimension * dimension * sizeof(char));
	cudaMemcpy(gpu_grid_1, grid, dimension * dimension * sizeof(char), cudaMemcpyHostToDevice);

	/* Kernel invocation */
	dim3 dimBlock(16, 16);
	dim3 dimGrid;
	dimGrid.x = (dimension + dimBlock.x - 1) / dimBlock.x;
	dimGrid.y = (dimension + dimBlock.y - 1) / dimBlock.y;

	int generation = 1;
	while (generation <= loops) {
		if (prints_enabled == 1) {
			printf("Generation: %d\n", generation);
		}
		local_diff = 0;
		cudaMemcpyToSymbol(diff, &local_diff,sizeof(int), 0, cudaMemcpyHostToDevice);

		kernel<<<dimGrid, dimBlock>>>(gpu_grid_1, gpu_grid_2, dimension);

		if (cudaGetLastError() != cudaSuccess) {
			printf("kernel launch failed\n");
		}
		cudaThreadSynchronize();
		
		//todo 8elei optimize auto
		if (prints_enabled == 1) {
			cudaMemcpy(grid, gpu_grid_2, dimension * dimension * sizeof(char), cudaMemcpyDeviceToHost);
			printGrid(grid, dimension);
		}

		cudaMemcpyFromSymbol(&local_diff, diff, sizeof(int), 0, cudaMemcpyDeviceToHost);

		// printf("local_diff %d\n", local_diff);

		/* If there are no differences between two generations
		 * OR if the next generation is 0 */
		if(local_diff == 0) {
			break;
		}

		char *temp = gpu_grid_1;
		gpu_grid_1 = gpu_grid_2;
		gpu_grid_2 = temp;

		generation++;
	}

	gettimeofday(&time_2, 0);
	double time = (1000000.0 * (time_2.tv_sec - time_1.tv_sec) + time_2.tv_usec - time_1.tv_usec) / 1000000;
	printf("time elapsed: %lf\n", time);

	cudaFree(gpu_grid_1);
	cudaFree(gpu_grid_2);
	freeGrid(&grid);
}

__global__ void kernel(char *grid_1, char *grid_2, int dimension) {

	/* The variables below are used to iterate the grid */
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

	/* We don't care about race conditions, we only check if it is different than 0 */
	if (grid_1[idx] != grid_2[idx]) {
		if(grid_2[idx] != 0){
			diff += 1;
		}
	}
}
