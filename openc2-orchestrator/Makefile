PROG=	orchestrator
SRCS=	orchestrator.c
MAN=

LDADD+=	-lzmq
LDFLAGS+=	-L/usr/local/lib
CFLAGS+=	-I/usr/local/include

.if defined(PREFIX)
BINDIR?=	${PREFIX}/bin
.else
BINDIR?=	/usr/bin
.endif

SUBDIR+=	etc

.include <bsd.prog.mk>
