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
	char grid[dimension][dimension];
	char /***grid,*/ *gpu_grid;
	// createGrid(&grid, dimension);
	// initGrid(grid, dimension);
	// if (input_file != NULL) {
	// 	readGrid(grid, input_file, dimension);
	// }

	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {
			grid[i][j] = 1;
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
	cudaMalloc((void **) &gpu_grid, dimension * dimension * sizeof(char));
	cudaMemcpy(gpu_grid , grid, dimension * dimension * sizeof(char), cudaMemcpyHostToDevice);
	kernel<<<128, (dimension * dimension / 128) + 1>>>(gpu_grid, dimension);
	cudaMemcpy(grid, gpu_grid, dimension * dimension * sizeof(char), cudaMemcpyDeviceToHost);
	
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

__global__ void kernel(char *grid, int dimension) {
	// printf("EXECUTING KERNEL\n");
	int ix = blockIdx.x * blockDim.x + threadIdx.x;
	int	iy = blockIdx.y*blockDim.y + threadIdx.y;
	int idx	= (iy * dimension + ix) % (dimension * dimension);
	printf("idx = %d\n", idx);
	grid[idx] += 1;
	// grid[0] = 2;
	// printf("KERNEL: "/*%d\n", grid[0][0]*/);
  	// for (idx = iy * dimension + ix; idx < dimension * dimension; idx += blockDim.x * gridDim.x) {
	// if(grid[idx] == 2){
	// 	printf("changed to 3\n");
	// 	grid[idx] = 3;
	// }
	// if(grid[idx] == 1){
	// 	printf("changed to 2\n");
	// 	grid[idx] = 2;
	// }
	// }
}