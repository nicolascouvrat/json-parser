CC=gcc
AR=ar
TDIR=../test
BDIR=../build

#PREFIX is an environment variable, give it default value if not set
ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

%.o: %.c jspr.h
	mkdir -p $(BDIR)
	$(CC) -c $< -o $(BDIR)/$@

libjspr.a: jspr.o
	$(AR) -rc $(BDIR)/$@ $(BDIR)/$^

install: libjspr.a
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(BDIR)/libjspr.a $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 jspr.h $(DESTDIR)$(PREFIX)/include/

uninstall:
	rm $(DESTDIR)$(PREFIX)/lib/libjspr.a
	rm $(DESTDIR)$(PREFIX)/include/jspr.h

test: $(TDIR)/test.c
	$(CC) $< -o $(TDIR)/$@
	./$(TDIR)/$@

test_debug: $(TDIR)/test.c
	$(CC) $^ -o $(TDIR)/$@ -D__DEBUG__=1
	./$(TDIR)/$@

clean: 
	rm -rf $(BDIR)

.PHONY: clean test
