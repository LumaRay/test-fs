C=gcc
RM=rm -f
CFLAGS=-O3 -mavx2 -g -funroll-all-loops -Wall
LDLIBS=
SRCS=main.c
OBJS=$(subst .c,.o,$(SRCS))
OUTFILE="main"

main: $(OBJS)
	$(C) $(LDFLAGS) -o $(OUTFILE) $(OBJS) $(LDLIBS) 

main.o: 
	$(C) $(CFLAGS) -c main.c

clean:
	$(RM) $(OBJS)


