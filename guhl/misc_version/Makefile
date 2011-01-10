VERSION = 0.01
CC      = /usr/bin/arm-linux-gnueabi-gcc
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
INCDIR	= -I/usr/arm-linux-gnueabi/usr/include/ -I.
LIBDIR  = /usr/arm-linux-gnueabi/lib/
LDFLAGS = -L$(LIBDIR) -static
RM      = rm -f

TRG = misc_version
OBJ = gopt.o misc_version.o 

all: $(OBJ)
	$(CC) $(CFLAGS) $(INCDIR) -o $(TRG) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) misc_version
