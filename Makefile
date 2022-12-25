CC = clang
SRCS=$(wildcard src/*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
INCS = -I/usr/include -I./include $(shell pkg-config glib-2.0 --cflags)
LDLIBS= -lc $(shell pkg-config glib-2.0 --libs)
CFLAGS = -std=c99 -Wall -Werror -pedantic -g -D_XOPEN_SOURCE ${INCS}

gmudix: $(OBJS)
	$(CC) $(CFLAGS) $^ $(INCS) $(LIBS) -o $@
