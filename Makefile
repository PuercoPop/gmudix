CC = clang
SRCS=$(wildcard src/*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
INCS= -I/usr/include -I./include
LDLIBS= -lc
CFLAGS = -std=c99 -Wall -Werror -pedantic -g -D_XOPEN_SOURCE ${INCS}

gmudix: $(OBJS)
	$(CC) $(CFLAGS) $^ $(INCS) $(LIBS) -o $@
