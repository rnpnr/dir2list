# dir2list - generates a list of files shuffled by directory
.POSIX:

include config.mk

SRC = dir2list.c
OBJ = $(SRC:.c=.o)

all: dir2list

config.h:
	cp config.def.h config.h

.c.o:
	$(CC) $(CFLAGS) -c $<

$(OBJ): config.h

dir2list: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f *.o dir2list

options:
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f dir2list $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/dir2list

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/dir2list

.PHONY: all options clean install uninstall
