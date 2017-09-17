#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "definitions.h"

int createGrid(int ***grid, int dimension);
void freeGrid(int ***grid);
void initGrid(int **grid, int dimension);
void readGrid(int **grid, char* filename, int dimension);
void printGrid(int **grid, int dimension, int rank, int glob_grid);
int calculateSubgridSize(int dimension, int number_of_processes);

#endif /* FUNCTIONS_H_ */
