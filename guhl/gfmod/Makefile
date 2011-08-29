    ifneq ($(KERNELRELEASE),)
    obj-m	:= gfmod.o

    else
    KDIR	:= /lib/modules/2.6.35.9-g3052235/build
#    KDIR       := /lib/modules/2.6.35.13-cyanogenmod+/build
#    KDIR       := /lib/modules/2.6.35.1399999999+/build
    PWD		:= $(shell pwd)
    default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
    endif

