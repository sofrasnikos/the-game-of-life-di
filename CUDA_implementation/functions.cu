#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <math.h>

#include "functions.h"

int createGrid(char **grid, int dimension) {
	int i;
	 /* Allocate 1d array and use it like a 2d */
	(*grid) = (char *) malloc(dimension * dimension * sizeof(char));
	if (grid == NULL) {
		printf("malloc error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return 0;
}

void freeGrid(char **grid) {
	free(*grid);
}

void initGrid(char *grid, int dimension) {
	int i, j, r;
	/* Initialize the grid*/
	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {
			/* Range from 1 to 100*/
			r = rand() % 100 + 1;
			/* 30% possibility to create a cell
			 * 70% possibility to create empty space*/
			if (r < 10) {
				grid[i * dimension + j] = 1;
			} else {
				grid[i * dimension + j] = 0;
			}
		}
	}
}

void readGrid(char *grid, char *filename, int dimension) {
	FILE *f;
	char line[MAXROW + 1];
	printf("Opening %s...\n", filename);
	f = fopen(filename, "r");
	if (!f) {
		printf("fopen failed\n");
		exit(EXIT_FAILURE);
	}
	int i = 0;
	int j;
	while (1) {
		if (fgets(line, MAXROW + 1, f) == NULL) {
			break;
		}
		int len = strlen(line) - 1;
		if (len != dimension) {
			printf("Found line size different than dimension given\n");
			exit(EXIT_FAILURE);
		}
		for (j = 0; j < len; j++) {
			if (line[j] == '*' || line[j] == '1') {
				grid[i * dimension + j] = 1;
			} else if (line[j] == '.' || line[j] == '0') {
				grid[i * dimension + j] = 0;
			} else {
				printf(
						"Wrong characters found in %s. Accepted characters are: 01.*\n",
						filename);
				exit(EXIT_FAILURE);
			}
		}
		i++;
	}
}

void printGrid(char *grid, int dimension) {
	int i, j;
	char *filename = (char *) malloc(sizeof(char) * 256);
	struct stat buffer;
	int exist;
	i = 0;
	do {
		snprintf(filename, 256, "outputs/grid_(%d)", i);
		exist = stat(filename, &buffer);
		if (exist == 0) {
			i++;
		}
	} while (exist == 0);

	FILE *fd = fopen(filename, "w+");
	if (!fd) {
		printf("fopen failed\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {
			if (grid[i * dimension + j] == 1) {
				fprintf(fd, "*");
			} else {
				fprintf(fd, ".");
			}
		}
		fprintf(fd, "\n");
	}
	fclose(fd);
	free(filename);
}
