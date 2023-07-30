CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ccani: $(OBJS)
		$(CC) -o ccani $(OBJS) $(LDFLAGS)

$(OBJS): ccani.h

test: ccani
		./test.sh

clean:
		rm -f ccani *.o *~ tmp*

.PHONY: test clean
