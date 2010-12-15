VERSION = 0.01
CC      = /usr/bin/gcc
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
LDFLAGS = -lm 
RM      = rm -f

TRG = gfree_verify
OBJ = soff_verify.o gfree_verify.o

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(TRG) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) gfree_verify
	