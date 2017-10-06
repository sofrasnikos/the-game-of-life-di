#!/bin/sh
THREADS=2
FLAG=core

clear 
echo "Execution 1"
echo "Processors: 1, Dimension: 30x30, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 1 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 30 -l 100;
echo
echo "Execution 2"
echo "Processors: 1, Dimension: 300x300, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 1 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 300 -l 100;
echo
echo "Execution 3"
echo "Processors: 1, Dimension: 3000x3000, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 1 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 3000 -l 100;
echo
########################################################################################################################################
echo "Execution 4"
echo "Processors: 4, Dimension: 30x30, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 4 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 30 -l 100;
echo
echo "Execution 5"
echo "Processors: 4, Dimension: 300x300, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 4 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 300 -l 100;
echo
echo "Execution 6"
echo "Processors: 4, Dimension: 3000x3000, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 4 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 3000 -l 100;
echo
########################################################################################################################################
echo "Execution 7"
echo "Processors: 9, Dimension: 30x30, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 9 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 30 -l 100;
echo
echo "Execution 8"
echo "Processors: 9, Dimension: 300x300, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 9 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 300 -l 100;
echo
echo "Execution 9"
echo "Processors: 9, Dimension: 3000x3000, Loops: 100"
mpiexec -f ../../MPI_OpenMP_implementation/machines -n 9 -bind-to $FLAG ../../MPI_OpenMP_implementation/game_of_life -t $THREADS -d 3000 -l 100;
echo
