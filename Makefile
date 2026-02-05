include config.mk

SRC=main.c termaze.c
OBJ=${SRC:.c=.o}
BIN=termaze

all: ${BIN}

$(BIN): ${OBJ}
	${CC} ${OBJ} -o $@ ${LDFLAGS}

install: all
	cp -f ${BIN} ${DESTDIR}${PREFIX}/bin/${BIN}

.c.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm *.o ${BIN} || true
