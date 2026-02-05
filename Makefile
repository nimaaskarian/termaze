include config.mk

SRC=main.c
OBJ=${SRC:.c=.o}
BIN=thecodinggame

all: ${BIN}

$(BIN): ${OBJ}
	${CC} ${OBJ} -o $@ ${LDFLAGS}

.c.o:
	${CC} -c ${CFLAGS} $<

clean:
	rm *.o || true
	rm thecodinggame
