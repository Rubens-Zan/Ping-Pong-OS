# GRR20206147 Rubens Zandomenighi Laszlo 

DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -Wall

.PHONY : all debug clean purge

all : pingpong-preempcao pingpong-preempcao-stress

pingpong-preempcao-stress: ppos_core.o pingpong-preempcao-stress.o queue.o
	gcc -o $@ $^

pingpong-preempcao: ppos_core.o pingpong-preempcao.o queue.o
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
	$(RM) pingpong-preempcao pingpong-preempcao-stress
