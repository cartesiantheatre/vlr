# Makefile for luagettext.
# $Id: Makefile,v 1.17 2007/10/30 21:55:20 cpressey Exp $

#BASE?=../../..
#include ${BASE}/Config

LIB=	lgettext.so
OBJS=	lua_gettext.o

CFLAGS+=	${WARNS} -fPIC -D __unused ""
CFLAGS+=	$(shell pkg-config --cflags lua5.1)
CFLAGS+=	${NLS_CFLAGS}
CFLAGS+=	${EXTRA_CFLAGS}

LDFLAGS+=	-lm $(shell pkg-config --libs lua5.1)
LDFLAGS+=	${NLS_LDFLAGS}

all:
	@echo "Nothing to build (not making NLS.)"

nls: ${LIB}

${LIB}: ${OBJS}
	${CC} ${OBJS} -shared ${LDFLAGS} -o lgettext.so

strip: ${LIB}
	${STRIP} ${LIB}

clean:
	touch phony.o phony.so
	rm -f *.o *.so

distribution:
	@echo "Nothing to distribute (not distributing NLS.)"

distribution_nls: strip
	mkdir -p ${DESTDIR}/lib/lua/5.1
	cp ${LIB} ${DESTDIR}/lib/lua/5.1

