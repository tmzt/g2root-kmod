all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $@ 

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

obj-m += gpgenoff1.o
obj-m += gpio88onoff1.o
obj-m += gpio88mode1.o
