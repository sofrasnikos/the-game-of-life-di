#!/bin/sh
# This script sets and exports OMP_NUM_THREADS environment variable

echo "Make sure that you executed the script with 'source' command."
echo "Otherwise the script will fail."
echo "Example: 'source ./set_threads.sh'"
echo

echo Specify the number of threads
read number_of_threads

#Check if input is not integer
if [ "$number_of_threads" -eq "$number_of_threads" ] 2>/dev/null
then
	#If it is then set/export env. var.
	echo "Exporting OMP_NUM_THREADS=$number_of_threads"
    set OMP_NUM_THREADS="$number_of_threads"
    export OMP_NUM_THREADS="$number_of_threads"
    echo "Export completed"
else
	#else quit
    echo "Only integer values are accepted"
fi