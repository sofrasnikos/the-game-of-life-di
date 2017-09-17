#ifndef EXECUTE_H_
#define EXECUTE_H_

#include "functions.h"

int execute(int rank, int num_of_proc, int num_of_threads, int dimension, int sub_grid_dimension, int loops, char *input_file, int prints_enabled);

void calculateInnerCells(int block_dimension, int **local_grid, int **next_local_grid, int num_of_threads);

void calculateEdgeCells(int block_dimension, int **local_grid, int **next_local_grid, int *top_buff, int *right_buff, int *bot_buff, int *left_buff,
		int top_left_value, int top_right_value, int bot_left_value, int bot_right_value, int num_of_threads);

int deadOrAlive(int alive_neighbors, int status);

#endif /* EXECUTE_H_ */
