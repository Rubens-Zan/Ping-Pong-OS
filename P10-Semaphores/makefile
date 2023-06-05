# GRR20206147 Rubens Zandomenighi Laszlo 

DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -Wall

.PHONY : all debug clean purge

all : pingpong-semaphore pingpong-racecond

pingpong-semaphore: ppos_core.o pingpong-semaphore.o queue.o
	gcc -o $@ $^

pingpong-racecond: ppos_core.o pingpong-racecond.o queue.o
	gcc -o $@ $^

ppos_core.o : ppos_core.c
	gcc $(CFLAGS) -c $^

queue.o : queue.c
	gcc $(CFLAGS) -c $^
	
debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

clean :
	$(RM) *.o

purge : clean
	$(RM) pingpong-semaphore pingpong-racecond
