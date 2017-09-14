######################################################################
OBJS 	= main.o execute.o functions.o
SOURCE	= main.c execute.c functions.c
HEADER  = execute.h functions.h
OUT  	= game_of_life
CC	= mpicc
FLAGS   = -g -lm -fopenmp
# -g option enables debugging mode 
# -lm flag enables "math.h" library

all: $(OBJS)
	$(CC) $(SOURCE) $(FLAGS) -o $(OUT)

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
#########################################################################