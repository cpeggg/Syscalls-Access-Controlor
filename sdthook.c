#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/unistd.h>

void netlink_release(void);
void netlink_init(void);
int AuditOpen(const char *pathname, int flags, int ret);

void *get_sys_call_table(void);
#ifdef _X86_
unsigned int clear_and_return_cr0(void);
void setback_cr0(unsigned int val);
#else
unsigned long clear_and_return_cr0(void);
void setback_cr0(unsigned long val);
#endif
void **sys_call_table;
asmlinkage long (* orig_open)(const char *pathname, int flags, mode_t mode);

asmlinkage long hacked_open(const char *pathname, int flags, mode_t mode)
{
	long ret;
  	if( pathname == NULL ) return -1;
    printk("Open Intercepted : %s \n", pathname);
	ret = orig_open(pathname, flags, mode);
  	AuditOpen(pathname,flags,ret);
  	return ret; 
}
  

static int __init audit_init(void)
{
#ifdef _X86_
	unsigned int orig_cr0 = clear_and_return_cr0();
#else
    unsigned long orig_cr0 = clear_and_return_cr0();
#endif
	sys_call_table = get_sys_call_table();
	printk("Info: sys_call_table found at %lx\n",(unsigned long)sys_call_table) ;
	//Hook Sys Call Open
	orig_open   = sys_call_table[__NR_open];
	sys_call_table[__NR_open]   = hacked_open;
	setback_cr0(orig_cr0);
	//Initialize Netlink
	netlink_init();
	return 0;
}


static void __exit audit_exit(void)
{
#ifdef _X86_
	unsigned int orig_cr0 = clear_and_return_cr0();
#else
    unsigned long orig_cr0 = clear_and_return_cr0();
#endif
	sys_call_table[__NR_open] = orig_open;
	setback_cr0(orig_cr0);
 	netlink_release();  	
}
MODULE_LICENSE("Dual BSD/GPL");
module_init(audit_init);
module_exit(audit_exit);
