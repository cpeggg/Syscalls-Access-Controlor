CONFIG_MODULE_SIG=n
obj-m:=AuditModule.o
AuditModule-objs   :=sdthook.o syscalltable.o netlinkp.o 
KDIR   := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc auditdaemon.c -o auditdaemon
clean:
	$(RM) -rf .*.cmd *.mod.c *.o *.ko .tmp* auditdaemon log

