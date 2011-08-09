    ifneq ($(KERNELRELEASE),)
    obj-m	:= gkmem.o

    else
    KDIR	:= /lib/modules/2.6.35.9-g3052235/build
    PWD		:= $(shell pwd)

    default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
    endif

