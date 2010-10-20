all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $@ 

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

pmmod1-y += pm1.o idle1.o cache1.o
obj-m += pmmod1.o

pmmod2-y += pm2.o idle1.o cache1.o
obj-m += pmmod2.o

pmmod3-y += pm3.o idle1.o cache1.o
obj-m += pmmod3.o

pmmod4-y += pm4.o idle1.o cache1.o
obj-m += pmmod4.o

