all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $@ 

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean


obj-$(tmzt) += mod18.o
obj-$(tmzt) += sysfind.o
obj-$(tmzt) += hostoff3.o
obj-$(tmzt) += blkops1.o
obj-m += debugsmi1.o
obj-m += unreg1.o
obj-m += rereg1.o
#obj-m += suspend1.o
obj-m += resume1.o
obj-m += bouncereg1.o
obj-m += bouncepwrreg1.o
obj-m += smidump1.o

