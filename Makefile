CONFIG_MODULE_SIG=n
obj-m:=AuditModule.o
AuditModule-objs   :=sdthook.o syscalltable.o netlinkp.o AccessControl.o 
KDIR   := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc auditdaemon.c -o auditdaemon
	gcc test.c -o syscall_test
	echo "USER's SECRET" > test
	-rm test_creat
clean:
	-$(RM) -rf .*.cmd *.mod.c *.o.* *.ko .tmp* auditdaemon log *.order *.symvers *.o test test_creat syscall_test peda*
adduser:
	useradd -u 1001 -g 1000 -d /home/test_cpegg -m test_cpegg
	useradd -u 1003 -g 1000 -d /home/test_cpegg3 -m test_cpegg3
	groupadd -g 1001 cpegg2
	useradd -u 1002 -g 1001 -d /home/test_cpegg2 -m test_cpegg2
	useradd -u 1004 -g 1001 -d /home/test_cpegg4 -m test_cpegg4
rmuser:
	userdel -r test_cpegg
	userdel -r test_cpegg2
	userdel -r test_cpegg3
	userdel -r test_cpegg4
	groupdel cpegg2
