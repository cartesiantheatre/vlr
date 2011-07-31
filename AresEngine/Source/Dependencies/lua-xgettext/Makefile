
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

OPTS = -O3 #-fglasgow-exts -funbox-strict-fields
PACKAGES = -package parsec

SRC = lua-xgettext.hs
PROG = lua-xgettext

all:
	ghc $(PROF) $(OPTS) --make $(PACKAGES) $(SRC) -o $(PROG)

DIRS=. Kuu

install:
	install -c -m 755 $(PROG) $(BINDIR)
	
clean:
	for i in $(DIRS); do rm -f $$i/*.o $$i/*.hi; done

