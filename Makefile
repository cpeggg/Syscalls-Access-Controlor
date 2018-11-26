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
	$(RM) -rf .*.cmd *.mod.c *.o.* *.ko .tmp* auditdaemon log *.order *.symvers *.o test test_creat syscall_test

