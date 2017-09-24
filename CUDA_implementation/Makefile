######################################################################
OBJS 	= main.o execute.o functions.o
SOURCE	= main.cu execute.cu functions.cu
HEADER  = execute.h functions.h definitions.h
OUT  	= game_of_life
NVCC	= nvcc
FLAGS = -g
# -g option enables debugging mode

all: $(OBJS)
	$(NVCC) $(FLAGS) $(OBJS) -o $(OUT) 

main.o : main.cu
	$(NVCC) $(FLAGS) -c -o main.o main.cu

execute.o : execute.cu
	$(NVCC) $(FLAGS) -c -o execute.o execute.cu

functions.o : functions.cu
	$(NVCC) $(FLAGS) -c -o functions.o functions.cu

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
#########################################################################