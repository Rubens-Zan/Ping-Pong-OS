DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -std=gnu99 -Wextra -Wall

.PHONY : all debug clean purge

all : pingpong-scheduler

pingpong-scheduler : pingpong-scheduler.o ppos_core.o queue.o ppos_data.h ppos.h
	gcc $(CFLAGS) -o $@ $^

ppos_core.o : ppos_core.c
	gcc $(CFLAGS) -c $^

queue.o : queue.c
	gcc $(CFLAGS) -c $^
	
debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

clean :
	$(RM) *.o

purge : clean
	$(RM) pingpong-scheduler