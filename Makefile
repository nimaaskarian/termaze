include config.mk

SRC=main.c termaze.c util.c
OBJ=${SRC:.c=.o}
BIN=termaze

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} $<

debug: CFLAGS=${INCS} -g3 -D_DEBUG -O0
debug: LDFLAGS=${LIBS} -g3 -D_DEBUG -O0
debug: ${BIN}

$(BIN): ${OBJ}
	${CC} ${OBJ} -o $@ ${LDFLAGS}

install: all
	cp -f ${BIN} ${DESTDIR}${PREFIX}/bin/${BIN}

clean:
	rm -f *.o ${BIN} || true
