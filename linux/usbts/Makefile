CFLAGS_usbts.o = -DCONFIG_TOUCHSCREEN_USB_FINGERVU436 -DCONFIG_TOUCHSCREEN_USB_USBDISPLAY32
ifndef KERNELRELEASE

PWD := $(shell pwd)
all:
	$(MAKE) -C /lib/modules/`uname -r`/build SUBDIRS=$(PWD) modules
clean:
	rm -f *.o *.ko *.mod.* .*.cmd Module.symvers Module.markers modules.order
	rm -rf .tmp_versions

install:
	$(MAKE) -C /lib/modules/`uname -r`/build SUBDIRS=$(PWD) modules_install

else
     obj-m := usbts.o
endif

