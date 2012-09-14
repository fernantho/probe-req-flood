OBJS = main.o flooder.o
CFLAGS = -O2
CC = gcc
LIBS = -L/usr/local/lib -lnl

all: flooder

flooder: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o flooder

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f flooder
	rm -f *~
