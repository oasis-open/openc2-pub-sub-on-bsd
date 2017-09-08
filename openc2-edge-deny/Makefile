SHLIB=	edgeactiondeny
SHLIB_MAJOR=	0
SRCS=	deny.c
MAN=

CFLAGS+=	-I/usr/local/include -I${.CURDIR}/../openc2-edge/include -D_EDGE_PLUGIN
LDFLAGS+=	-L/usr/local/lib
LDADD+=		-lucl

.if defined(PREFIX)
LIBDIR?=	${PREFIX}/local/lib
.endif

.include <bsd.lib.mk>
