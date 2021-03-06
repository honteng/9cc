
CLFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	cc -g -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS)

$(OBJS): 9cc.h

test: 9cc
	./9cc -test
	./test.sh

clean:
	rm -f 9cc *.o tmp*

