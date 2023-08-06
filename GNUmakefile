PROJECT    =c-trec
VERSION    =1.0.0
DESTDIR    =
PREFIX     =/usr/local
AR         =ar
CC         =gcc
CFLAGS     =-Wall -g
CPPFLAGS   =
LIBS       =-lutil
PROGRAMS   =./trec
CFLAGS_ALL =$(LDFLAGS) $(CFLAGS) $(CPPFLAGS)
## -- targets
all: $(PROGRAMS)
install: $(PROGRAMS)
	install -d                $(DESTDIR)$(PREFIX)/bin
	install -m755 $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin
clean:
	rm -f $(PROGRAMS)
## -- programs
./trec: main.c trec.c trec.h
	$(CC) -o $@ main.c trec.c $(CFLAGS_ALL) $(LIBS)
## -- BLOCK:license --
install: install-license
install-license: 
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/$(PROJECT)
	cp LICENSE README.md $(DESTDIR)$(PREFIX)/share/doc/$(PROJECT)
update: update-license
update-license:
	ssnip README.md
## -- BLOCK:license --
## -- BLOCK:man --
## -- BLOCK:man --
