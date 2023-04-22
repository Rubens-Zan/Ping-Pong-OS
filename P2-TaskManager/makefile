DEBUG_FLAGS = -g -DDEBUG
CFLAGS = -std=gnu99 -Wextra -Wall

.PHONY : all debug clean purge

all : pingpong-tasks1 pingpong-tasks2 pingpong-tasks3

pingpong-tasks1 : pingpong-tasks1.o ppos_core.o ppos_data.h ppos.h
	gcc $(CFLAGS) -o $@ $^

pingpong-tasks2 : pingpong-tasks2.o ppos_core.o ppos_data.h ppos.h
	gcc $(CFLAGS) -o $@ $^

pingpong-tasks3 : pingpong-tasks3.o ppos_core.o ppos_data.h ppos.h
	gcc $(CFLAGS) -o $@ $^

ppos_core.o : ppos_core.c
	gcc $(CFLAGS) -c $^
	
debug : CFLAGS += $(DEBUG_FLAGS)
debug : all

clean :
	$(RM) *.o

purge : clean
	$(RM) pingpong-tasks1 pingpong-tasks2 pingpong-tasks3