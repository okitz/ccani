CFLAGS=-std=c11 -g -static

ccani: ccani.c

play: play.c

test: ccani
	./test.sh

clean:
	rm -f ccani *.o *~ tmp*

.PHONY: test clean
