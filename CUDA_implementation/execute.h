#ifndef EXECUTE_H_
#define EXECUTE_H_

#include "functions.h"

void execute(int dimension, int loops, char *input_file, int prints_enabled);
__global__ void kernel(char *grid, int dimension);

#endif /* EXECUTE_H_ */