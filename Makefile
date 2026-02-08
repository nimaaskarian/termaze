include config.mk

SRC=main.c termaze.c util.c
OBJ=${SRC:.c=.o}
BIN=termaze

all: ${BIN}

debug: CFLAGS=${INCS} -g -D_DEBUG -O0
debug: LDFLAGS=${LIBS} -g -D_DEBUG -O0
debug: ${BIN}

$(BIN): ${OBJ}
	${CC} ${OBJ} -o $@ ${LDFLAGS}

install: all
	cp -f ${BIN} ${DESTDIR}${PREFIX}/bin/${BIN}

.c.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm -f *.o ${BIN} || true
