OBJS = main.o flooder.o
CFLAGS = -O2
CC = gcc

all: flooder

flooder: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o flooder

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f flooder
	rm -f *~

