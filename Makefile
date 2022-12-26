CC = clang
SRCS=$(wildcard src/*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
INCS = -I/usr/include -I./include $(shell pkg-config glib-2.0 gtk+-2.0 --cflags)
LDLIBS= -lc $(shell pkg-config glib-2.0 gtk+-2.0 --libs)
# TODO(javier): Renable -Werror
CFLAGS = -std=c99 -Wall -pedantic -g -D_XOPEN_SOURCE -D_POSIX_C_SOURCE=200809L ${INCS}

gmudix: $(OBJS)
	$(CC) $(CFLAGS) $^ $(INCS) $(LIBS) -o $@
