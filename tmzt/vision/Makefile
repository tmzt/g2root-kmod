all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $@ 

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

obj-m += gpgenoff1.o
obj-m += gpio88onoff1.o
obj-m += gpio88mode1.o
obj-m += gpio88mode2.o
obj-m += gpio88mode3.o
obj-m += bouncemode1.o
obj-m += gpio88mode3-2.o
obj-m += gpio88mode3-3.o
obj-m += gpio88mode3-4.o
obj-m += gpio88mode3-5.o
obj-m += gpio88mode4.o
obj-m += gpio88mode5.o
obj-m += claimmode3.o
obj-m += claimmode4.o
