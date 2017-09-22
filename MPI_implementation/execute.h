#ifndef EXECUTE_H_
#define EXECUTE_H_

#include "functions.h"

int execute(int rank, int num_of_proc, int dimension, int sub_grid_dimension, int loops, char *input_file, int prints_enabled);

void calculateInnerCells(int block_dimension, char **local_grid, char **next_local_grid);

void calculateEdgeCells(int block_dimension, char **local_grid, char **next_local_grid, char *top_buff, char *right_buff, char *bot_buff, char *left_buff,
		char top_left_value, char top_right_value, char bot_left_value, char bot_right_value);

int deadOrAlive(int alive_neighbors, int status);

#endif /* EXECUTE_H_ */
