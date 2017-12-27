CC=gcc
AR=ar
TDIR=../test
BDIR=../build

%.o: %.c index.h DynamicArray.h
	mkdir -p $(BDIR)
	$(CC) -c $< -o $(BDIR)/$@

all: libindex.a libdynamicindex.a

libindex.a: index.o
	$(AR) -rc $(BDIR)/$@ $(BDIR)/$^

libdynamicindex.a: DynamicArray.o
	$(AR) -rc $(BDIR)/$@ $(BDIR)/$^

test: test_dyn_array test_main

test_dyn_array: $(TDIR)/DynamicArrayTest.c
	$(CC) $< -o $(TDIR)/$@
	./$(TDIR)/$@

test_main: $(TDIR)/test.c
	$(CC) $< -o $(TDIR)/$@ $(BDIR)/libdynamicindex.a
	./$(TDIR)/$@

test_debug: $(TDIR)/test.c
	$(CC) $^ -o $(TDIR)/$@ -D__DEBUG__=1
	./$(TDIR)/$@

clean: 
	rm -rf $(BDIR)

.PHONY: clean test
