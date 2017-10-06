#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "execute.h"

int main(int argc, char *argv[]) {

	srand(time(NULL));
	
	int i;
	int dimension = -1, loops = -1;
	int prints_enabled = 0;
	char *input_file = NULL;
	int error = 0;

	
	// Parse arguments
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			i++;
			if (argv[i] != NULL) {
				dimension = atoi(argv[i]);
				if (dimension < 5) {
					error = 1;
					break;
				}
			} else {
				error = 2;
				break;
			}
			continue;
		}
		else if (!strcmp(argv[i], "-f")) {
			i++;
			if (argv[i] != NULL) {
				input_file = argv[i];
			} else {
				error = 3;
				break;
			}
			continue;
		} else if (!strcmp(argv[i], "-l")) {
			i++;
			if (argv[i] != NULL) {
				loops = atoi(argv[i]);
				if (loops <= 0) {
					error = 4;
					break;
				}
			} else {
				error = 5;
				break;
			}
			continue;
		} else if (!strcmp(argv[i], "-p")) {
			prints_enabled = 1;
			continue;
		} else {
			error = 6;
			break;
		}
	}

	// Default values if user does not specify
	if (dimension == -1) {
		dimension = DEFAULT_DIMENSION_SIZE;
	}
	if (loops == -1) {
		loops = DEFAULT_NUMBER_OF_LOOPS;
	}

	// Print error messages if you are root process
	if (error != 0) {
		switch (error) {
		case 1:
			printf("Error: wrong dimension size\n");
			break;
		case 2:
			printf("Error: you did not specify dimension size\n");
			break;
		case 3:
			printf("Error: you did not specify input file\n");
			break;
		case 4:
			printf("Error: wrong number of loops\n");
			break;
		case 5:
			printf("Error: you did not specify number of loops\n");
			break;
		case 6:
			printf("Error: unknown argument: %s\n", argv[i]);
			break;
		}
		printf(
				"\nUSAGE: ./game_of_life -d <dimension_of_grid> -f <input_file> -l <number_of_loops> -p\n");
		printf("\nFLAGS\n");
		printf("-d <dimension_of_grid> : This flag sets up the size of the grid. If it is not set up, the program will use %d as deafault value.\n", DEFAULT_DIMENSION_SIZE);
		printf("-f <input_file> : This flag forces the program to read an initial state of grid from file, and use it as the first generation.\n");
		printf("-l <number_of_loops> : This flag determines the generations that will be completed.\n");
		printf("-p : (OPTIONAL FLAG) This flag forces the program to print the generations to output files. ATTENTION! This flag causes major slowdown to the execution!\n");
	} 
	if (error != 0) {
		exit(0);
	}

	if(prints_enabled == 1) {
		printf("dimension size: %d\n", dimension);
		printf("number of loops: %d\n", loops);
	}

	execute(dimension, loops, input_file, prints_enabled);
	
	printf("Terminated successfully\n");
	exit(0);
}
