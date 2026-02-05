include config.mk

SRC=main.c termaze.c
OBJ=${SRC:.c=.o}
BIN=termaze

all: ${BIN}

$(BIN): ${OBJ}
	${CC} ${OBJ} -o $@ ${LDFLAGS}

.c.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm *.o || true
	rm thecodinggame
