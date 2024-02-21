PROJECT    =c-trec
VERSION    =1.0.0
DESTDIR    =
PREFIX     =/usr/local
CC         =gcc -pedantic-errors -std=c99 -Wall
PROGRAMS   =./trec$(EXE)
## -- targets
all: $(PROGRAMS)
install: $(PROGRAMS)
	@install -d $(DESTDIR)$(PREFIX)/bin
	install -m755 $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin
clean:
	rm -f $(PROGRAMS)
## -- programs
./trec: main.c trec.c trec.h
	$(CC) -o $@ main.c trec.c $(CFLAGS)
## -- BLOCK:license --
install: install-license
install-license: 
	@mkdir -p $(DESTDIR)$(PREFIX)/share/doc/$(PROJECT)
	cp LICENSE  $(DESTDIR)$(PREFIX)/share/doc/$(PROJECT)
## -- BLOCK:license --
