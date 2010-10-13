

all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $@ 

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

#obj-m += tmzt/suspend1.o
obj-m += tmzt/resume1.o
obj-m += tmzt/unreg2.o
obj-m += tmzt/rereg1.o
obj-m += tmzt/bouncereg1.o
