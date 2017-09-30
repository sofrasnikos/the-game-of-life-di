#!/bin/sh

clear 
echo "Execution 1"
echo "Processors: 4, Threads: 2, Dimension: 100x100, Loops: 100"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 2 -d 100 -l 100;
echo
echo "Execution 2"
echo "Processors: 4, Threads: 2, Dimension: 100x100, Loops: 1000"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 2 -d 100 -l 1000;
echo
echo "Execution 3"
echo "Processors: 4, Threads: 4, Dimension: 100x100, Loops: 100"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 4 -d 100 -l 100;
echo
echo "Execution 4"
echo "Processors: 4, Threads: 4, Dimension: 100x100, Loops: 1000"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 4 -d 100 -l 1000;
echo
echo "Execution 5"
echo "Processors: 4, Threads: 2, Dimension: 1000x1000, Loops: 100"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 2 -d 1000 -l 100;
echo
echo "Execution 6"
echo "Processors: 4, Threads: 2, Dimension: 1000x1000, Loops: 1000"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 2 -d 1000 -l 1000;
echo
echo "Execution 7"
echo "Processors: 4, Threads: 4, Dimension: 1000x1000, Loops: 100"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 4 -d 1000 -l 100;
echo
echo "Execution 8"
echo "Processors: 4, Threads: 4, Dimension: 1000x1000, Loops: 1000"
mpiexec -n 4 -f machines -bind-to socket ../../MPI_OpenMP_implementation/game_of_life -t 4 -d 1000 -l 1000;


time elapsed: 3.215046
