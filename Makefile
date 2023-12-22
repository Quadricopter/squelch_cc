SRCS    = squelch.c
NAME    = squelch_cc
OBJS    = $(SRCS:.c=.o)
CFLAGS  = -Wall
LDFLAGS = -lm
all: $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o *~ \#*

