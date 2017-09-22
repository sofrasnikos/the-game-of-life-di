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
	char **grid, **gpu_grid;
	createGrid(&grid, dimension);
	initGrid(grid, dimension);
	if (input_file != NULL) {
		readGrid(grid, input_file, dimension);
	}

	for(i = 0; i < dimension; i++) {
		for(j = 0; j < dimension; j++) {
			printf("%d ", grid[i][j]);
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
	cudaMalloc(&gpu_grid, dimension * dimension * sizeof(char));
	cudaMemcpy(grid, gpu_grid, dimension * dimension * sizeof(char), cudaMemcpyHostToDevice);
	kernel<<<128, dimension * dimension / 128>>>(grid, dimension);
	cudaMemcpy(gpu_grid, grid, dimension * dimension * sizeof(char), cudaMemcpyDeviceToHost);
	
	for(i = 0; i < dimension; i++) {
		for(j = 0; j < dimension; j++) {
			printf("%d ", grid[i][j]);
		}
		printf("\n");
	}
	printf("\nEND OF PRINT\n");
	fflush(stdout);
	// nblocks = ()

}

__global__ void kernel(char **grid, int dimension) {
	printf("EXECUTING KERNEL\n");
	int ix = blockIdx.x * blockDim.x + threadIdx.x;
	int	iy = blockIdx.y*blockDim.y + threadIdx.y;
	int idx	= iy * dimension + ix;
	printf("KERNEL: %d\n", grid[ix][iy]);
	if(grid[ix][iy] == 1){
		grid[ix][iy] = 2;
	}
}