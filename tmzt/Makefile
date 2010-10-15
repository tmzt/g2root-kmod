all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $@ 

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean


obj-$(tmzt) += foreign/mod18.o
obj-$(tmzt) += foreign/sysfind.o
obj-$(tmzt) += g2/hostoff3.o
obj-$(tmzt) += g2/blkops1.o
obj-m += g2/debugsmi1.o
obj-m += g2/unreg1.o
obj-m += g2/rereg1.o
#obj-m += g2/suspend1.o
obj-m += g2/resume1.o
obj-m += g2/bouncereg1.o
obj-m += g2/bouncepwrreg1.o
obj-m += g2/smidump1.o

