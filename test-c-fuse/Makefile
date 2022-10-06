C = gcc
SRC = fusehash.c hashmap.c
OBJS = $(subst .c,.o,$(SRCS))
CFLAGSDEBUG = -DDEBUG -g -O0 -D_FILE_OFFSET_BITS=64 -lm
CFLAGS = -Ofast -D_FILE_OFFSET_BITS=64 -lm
RM = rm -f
OUTFILE = "fusehash"

build: $(SRC)
	$(C) $(SRC) $(CFLAGS) -o $(OUTFILE) `pkg-config fuse --cflags --libs`

clean:
	$(RM) $(OBJS)
