CC=gcc
AR=ar
TDIR=../test

%.o: %.c index.h
	$(CC) -c $< -o $@

libindex.a: index.o
	$(AR) -rc $@ $^

test: $(TDIR)/test.c
	$(CC) $< -o $(TDIR)/$@.test
	./$(TDIR)/$@.test

test_debug: $(TDIR)/test.c
	$(CC) $^ -o $(TDIR)/$@.test -D__DEBUG__=1
	./$(TDIR)/$@.test

clean: 
	rm -f *.o *.a

.PHONY: clean test
