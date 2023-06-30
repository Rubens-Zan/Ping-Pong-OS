# GRR20206147 Rubens Zandomenighi Laszlo 

DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -Wall -lm -lrt

.PHONY : all debug clean purge

all : pingpong-disco1 pingpong-disco2

pingpong-disco1: ppos_core.o pingpong-disco1.o queue.o disk.o ppos_disk.o
	gcc -o $@ $^ $(CFLAGS) 

pingpong-disco2: ppos_core.o pingpong-disco2.o queue.o disk.o ppos_disk.o
	gcc -o $@ $^ $(CFLAGS) 

ppos_core.o : ppos_core.c
	gcc $(CFLAGS) -c $^

queue.o : queue.c
	gcc $(CFLAGS) -c $^

disk.o : disk.c
	gcc $(CFLAGS) -c $^

ppos_disk.o : ppos_disk.c
	gcc $(CFLAGS) -c $^
	
debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

clean :
	$(RM) *.o

purge : clean
	$(RM) pingpong-disco1 pingpong-disco2 *.log
