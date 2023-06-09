# GRR20206147 Rubens Zandomenighi Laszlo 

DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -Wall -lm

.PHONY : all debug clean purge

all : pingpong-mqueue

pingpong-mqueue: ppos_core.o pingpong-mqueue.o queue.o
	gcc -o $@ $^ $(CFLAGS) 

ppos_core.o : ppos_core.c
	gcc $(CFLAGS) -c $^

queue.o : queue.c
	gcc $(CFLAGS) -c $^
	
debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

clean :
	$(RM) *.o

purge : clean
	$(RM) pingpong-mqueue
