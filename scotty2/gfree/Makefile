VERSION = 0.02
CC      = /usr/bin/arm-unknown-linux-gnu-gcc
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
INCDIR	= -I/usr/arm-unknown-linux-gnu/usr/include/ -I.
LIBDIR  = /usr/arm-unknown-linux-gnu/lib/
LDFLAGS = -L$(LIBDIR) -static -lm 
RM      = rm -f

TRG = gfree
OBJ = gopt.o gfree.o 

all: $(OBJ)
	$(CC) $(CFLAGS) $(INCDIR) -o $(TRG) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) gfree
	