DESTDIR    =
PREFIX     =/usr/local
AR         =ar
CC         =gcc
CFLAGS     =-Wall -g
CPPFLAGS   =
LIBS       =-lutil
PROGRAMS   =./trec
CFLAGS_ALL =$(LDFLAGS) $(CFLAGS) $(CPPFLAGS)

all: $(PROGRAMS)
install: $(PROGRAMS)
	install -d                $(DESTDIR)$(PREFIX)/bin
	install -m755 $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin
clean:
	rm -f $(PROGRAMS)
./trec: main.c trec.c trec.h
	$(CC) -o $@ main.c trec.c $(CFLAGS_ALL) $(LIBS)

## -- license --
ifneq ($(PREFIX),)
install: install-license
install-license: LICENSE
	@echo 'I share/doc/c-trec/LICENSE'
	@mkdir -p $(DESTDIR)$(PREFIX)/share/doc/c-trec
	@cp LICENSE $(DESTDIR)$(PREFIX)/share/doc/c-trec
endif
## -- license --
