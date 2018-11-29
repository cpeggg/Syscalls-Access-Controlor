CONFIG_MODULE_SIG=n
obj-m:=AuditModule.o
AuditModule-objs   :=sdthook.o syscalltable.o netlinkp.o 
KDIR   := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc auditdaemon.c -o auditdaemon
	gcc test.c -o syscall_test
	echo "SYSCALL TEST" > test
	-rm test_creat
clean:
	-$(RM) -rf .*.cmd *.mod.c *.o.* *.ko .tmp* auditdaemon log *.order *.symvers *.o test test_creat syscall_test peda*
adduser:
	useradd -u 1001 -g 1000 -d /home/test_cpegg -m test_cpegg
	groupadd -g 1001 cpegg2
	useradd -u 1002 -g 1001 -d /home/test_cpegg2 -m test_cpegg2
rmuser:
	userdel -r test_cpegg
	userdel -r test_cpegg2
	groupdel cpegg2
