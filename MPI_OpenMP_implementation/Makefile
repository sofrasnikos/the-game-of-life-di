######################################################################
OBJS 	= main.o execute.o functions.o
SOURCE	= main.c execute.c functions.c
HEADER  = execute.h functions.h definitions.h
OUT  	= game_of_life
CC	= mpicc -fopenmp
FLAGS   = -g -lm
# -g option enables debugging mode 
# -lm flag enables "math.h" library

all: $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $(OUT)

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
#########################################################################