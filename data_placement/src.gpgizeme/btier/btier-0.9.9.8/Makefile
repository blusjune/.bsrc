obj-m += btier.o
btier-objs :=
btier-objs += btier_common.o
btier-objs += btier_main.o 

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
CC := gcc -O2 -march=core2

default:
	$(MAKE) -Wall -C $(KDIR) SUBDIRS=$(PWD) M=$(PWD) modules
	$(CC) -D_FILE_OFFSET_BITS=64 btier_setup.c -o btier_setup 
	$(CC) tools/writetest.c -o tools/writetest
clean:
	rm -f *.c~ *.o *.ko btier.mod.c modules.order Module.symvers .*.cmd 
	rm -rf .tmp_versions
	rm -f tools/writetest
	rm -f btier.ko.unsigned
	rm -f btier_setup
	rm -f Makefile.xen
pretty: 
	indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 *.c
	rm -f *.c~
install:
	install -D -m 755 btier.ko $(DEST)/lib/modules/`uname -r`/kernel/drivers/block/btier.ko
	depmod -a
	install -D -m 755 btier_setup $(DEST)/sbin/btier_setup
	install -D -m 755 rc/btier $(DEST)/etc/init.d/btier
	install -D -m 600 rc/bttab $(DEST)/etc/bttab
	install -D -m 600 btier_setup.1 $(DEST)/usr/share/man/man1/btier_setup.1
uninstall:
	rm /lib/modules/`uname -r`/kernel/drivers/block/btier.ko
	rm /sbin/btier_setup
	depmod -a	
