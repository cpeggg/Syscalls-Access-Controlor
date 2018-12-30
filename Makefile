CONFIG_MODULE_SIG=n
obj-m:=AuditModule.o
AuditModule-objs   :=sdthook.o syscalltable.o netlinkp.o AccessControl.o parseconf.o
KDIR   := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc auditdaemon.c -o auditdaemon
	gcc execTest.c -o execTest
	##gcc ./user_interface.c -o user_interface
	chmod 111 execTest
	echo "USERS_SECRET" > /tmp/ioTest
	chmod 666 /tmp/ioTest
	mv -f execTest /tmp/
clean:
	-$(RM) -rf .*.cmd *.mod.c *.o.* *.ko .tmp* 
	-$(RM) auditdaemon log *.order *.symvers *.o test test_creat syscall_test peda* user_interface
	-$(RM) /tmp/ioTest /tmp/execTest /tmp/creatTest
