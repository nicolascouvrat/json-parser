CC=gcc
AR=ar
TDIR=../test
BDIR=../build

%.o: %.c index.h
	mkdir -p $(BDIR)
	$(CC) -c $< -o $(BDIR)/$@

libindex.a: index.o
	$(AR) -rc $(BDIR)/$@ $(BDIR)/$^

test: $(TDIR)/test.c
	$(CC) $< -o $(TDIR)/$@
	./$(TDIR)/$@

test_debug: $(TDIR)/test.c
	$(CC) $^ -o $(TDIR)/$@ -D__DEBUG__=1
	./$(TDIR)/$@

clean: 
	rm -rf $(BDIR)

.PHONY: clean test