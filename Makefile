

all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $@ 

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

#obj-m += tmzt/g2/suspend1.o
obj-m += tmzt/g2/resume1.o
obj-m += tmzt/g2/unreg2.o
obj-m += tmzt/g2/rereg1.o
obj-m += tmzt/g2/bouncereg1.o
obj-m += tmzt/g2/bouncepwrreg1.o
obj-m += tmzt/g2/smidump1.o
