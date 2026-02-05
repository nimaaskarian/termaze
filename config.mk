PREFIX = /usr/local
NCURSESINC = `pkg-config --cflags ncurses`
NCURSESLIBS = `pkg-config --libs ncurses`

INC=${NCURSESINC}
LIBS=${NCURSESLIBS}

CFLAGS=-pedantic -Wall -Wno-unused-function -Wno-deprecated-declarations -O3 -march=native ${INCS}
LDFLAGS=${LIBS}
