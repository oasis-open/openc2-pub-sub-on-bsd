PROG=	openc2broadcast
SRCS=	openc2broadcast.c
MAN=

LDADD+=	-lzmq
LDFLAGS+=	-L/usr/local/lib
CFLAGS+=	-I/usr/local/include

.if defined(PREFIX)
BINDIR?=	${PREFIX}/bin
.else
BINDIR?=	/usr/bin
.endif

.include <bsd.prog.mk>
