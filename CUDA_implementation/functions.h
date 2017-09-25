#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "definitions.h"

int createGrid(char **grid, int dimension);
void freeGrid(char **grid);
void initGrid(char *grid, int dimension);
void readGrid(char *grid, char* filename, int dimension);
void printGrid(char *grid, int dimension);

#endif /* FUNCTIONS_H_ */
