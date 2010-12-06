VERSION = 0.02
CC      = /usr/bin/gcc
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
LDFLAGS = -lm 
RM      = rm -f

TRG = p7_s-off
OBJ = gopt.o soff_verify.o p7_s-off.o

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(TRG) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) p7_s-off
	