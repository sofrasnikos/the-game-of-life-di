/*
 * functions.c
 *
 *  Created on: Jul 23, 2017
 *      Author: vangelis
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <errno.h>

#include "functions.h"

void PrintGrid(int **grid, int rows, int columns, int rank) {
	char *filename = malloc(sizeof(char) * 256);
	snprintf(filename, 256, "../outputs/process_%d", rank);
	FILE *fd = fopen(filename, "w+");
	if (!fd) {
		printf("fopen failed\n");
		exit(EXIT_FAILURE);
	}
	int i, j;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			if (grid[i][j] == 1) {
				//printf("*");
				fprintf(fd, "*");
			} else {
				//printf(".");
				fprintf(fd, ".");
			}
		}
		fprintf(fd, "\n");
	}
	fclose(fd);
	free(filename);
}
