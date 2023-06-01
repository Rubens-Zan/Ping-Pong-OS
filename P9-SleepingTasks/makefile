# GRR20206147 Rubens Zandomenighi Laszlo 

DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -Wall

.PHONY : all debug clean purge

all : pingpong-sleep

pingpong-sleep: ppos_core.o pingpong-sleep.o queue.o
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
	$(RM) pingpong-sleep 
