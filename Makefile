CONFIG_MODULE_SIG=n
obj-m:=AuditModule.o
AuditModule-objs   :=sdthook.o syscalltable.o netlinkp.o AccessControl.o parseconf.o
KDIR   := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc auditdaemon.c -o auditdaemon
	gcc -Wno-nonnull test.c -o syscall_test
	gcc execTest.c -o execTest
	chmod 111 execTest
	echo "USER_SECRET" > /tmp/ioTest
	chmod 666 /tmp/ioTest
	mv -f execTest /tmp/

clean:
	-$(RM) -rf .*.cmd *.mod.c *.o.* *.ko .tmp* 
	-$(RM) auditdaemon log *.order *.symvers *.o test test_creat syscall_test peda*
	-$(RM) /tmp/ioTest /tmp/execTest /tmp/creatTest
adduser:
	useradd -u 1001 -g 1000 -d /home/test_cpegg -m test_cpegg
	useradd -u 1003 -g 1000 -d /home/test_cpegg3 -m test_cpegg3
	groupadd -g 1001 cpegg2
	useradd -u 1002 -g 1001 -d /home/test_cpegg2 -m test_cpegg2
	useradd -u 1004 -g 1001 -d /home/test_cpegg4 -m test_cpegg4
	# test conf
	useradd -u 1005 -g 1000 -d /home/test_cpegg5 -m test_cpegg5
	groupadd -g 1002 cpegg3
	useradd -u 1006 -g 1002 -d /home/test_cpegg6 -m test_cpegg6
rmuser:
	-userdel -r test_cpegg
	-userdel -r test_cpegg2
	-userdel -r test_cpegg3
	-userdel -r test_cpegg4
	-groupdel cpegg2
	# test conf
	-userdel -r test_cpegg5
	-userdel -r test_cpegg6
	-groupdel cpegg3
