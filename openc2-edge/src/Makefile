PROG=	edge
SRCS=	config.c \
	edge.c \
	log.c \
	run.c \
	util.c
MAN=

CFLAGS+=	-I${.CURDIR}/../include -I/usr/local/include -g -O0
LDFLAGS+=	-L/usr/local/lib
LDADD+=		-lucl -lzmq

.if defined(PREFIX)
BINDIR?=	${PREFIX}/bin
.else
BINDIR?=	/usr/bin
.endif

.include <bsd.prog.mk>
