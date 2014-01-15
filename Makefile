CC := gcc
TARGET := a.out
SRCS := \
	gl_helpers.c    \
	draw_frame.c    \
	intro.c         \
	scene.c         \
	hud.c           \
	ui.c            \
	game.c          \
	level_design.c  \
	formation.c	\
	world.c         \
	player.c        \
	enemy.c         \
	main.c

OBJS := ${SRCS:.c=.o}
DEPS := ${SRCS:.c=.dep}
CFLAGS := -std=c99 -pedantic -Wall `sdl-config --cflags`
DEBUG := -g3 -O0
LDFLAGS :=
LIBS := -lGLEW -lSDL -lGL -lGLU -lSDL_ttf -lm -DGL_GLEXT_PROTOTYPES  \
        `sdl-config --libs`            \
        `pkg-config SDL_ttf --libs`    \
        `pkg-config SDL_image --libs`  \
        `pkg-config SDL_gfx --libs`    \
	-lSDL_mixer
#-lX11 -lXi -lXmu

.PHONY: all release debug clean help

all: release

release: ${TARGET}

debug: CFLAGS := ${CFLAGS} ${DEBUG}
debug: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

${OBJS}: %.o: %.c %.dep
	${CC} ${CFLAGS} -o $@ -c $<

${DEPS}: %.dep: %.c Makefile
	${CC} ${CFLAGS} -MM $< > $@

clean:
	rm -f *.o *.dep ${TARGET}

help:
	echo "e.g.: make LIBS=-lm -lusb ... oder gleich eintragen"
