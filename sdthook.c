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
#define MAX_LENGTH 256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cpegg");
MODULE_DESCRIPTION("A simple example Linux module leart from Robert W. Oliver II");
MODULE_VERSION("0.01");

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
unsigned long * sys_call_table=NULL;
asmlinkage int (* orig_open)(const char *pathname, int flags, mode_t mode);

asmlinkage int hacked_open(const char *pathname, int flags, mode_t mode)
{
	int ret;
    char buf[MAX_LENGTH]={0};
    //printk(KERN_INFO"[+] open() hooked.");	
    if( pathname == NULL ) return -1;
    //printk("Open Intercepted : %lx\n", (unsigned long)pathname);
    // To secure from crash (SMAP protect)
    copy_from_user(buf,pathname,MAX_LENGTH);
    //printk("pathname : %s\n",buf);
	ret = orig_open(pathname, flags, mode);
    if (ret<-1) ret=-1;
  	AuditOpen(buf,flags,ret);
  	return ret; 
}
  

static int __init audit_init(void)
{
#ifdef _X86_
	unsigned int orig_cr0;
#else
    unsigned long orig_cr0;
#endif
    printk("Audit Module Loading...\n");
    orig_cr0 = clear_and_return_cr0();
    //printk("Info: orig_cr0 %lx\n",(unsigned long)orig_cr0);
	sys_call_table = get_sys_call_table();
	//printk("Info: sys_call_table found at %lx\n",(unsigned long)sys_call_table) ;
	
    //Hook Sys Call Open
	orig_open   = (void*)sys_call_table[__NR_open];
    //printk("Info: orig_open at %lx\n",(unsigned long) orig_open);
    //printk("Info: hacked_open at %lx\n",(unsigned long) &hacked_open);
    
    // It seems that change the 16bit of CR0 on x86_64 platform won't work.
    // No no, CR0 can work. But I still don't understand why define sys_call_table as unsigned long* and it will work?
    sys_call_table[__NR_open] = (unsigned long)&hacked_open;
	setback_cr0(orig_cr0);
	
    //Initialize Netlink
	netlink_init();
    printk("Audit Module loaded.\n");
	return 0;
}


static void __exit audit_exit(void)
{
#ifdef _X86_
	unsigned int orig_cr0;
#else
    unsigned long orig_cr0;
#endif
    orig_cr0 = clear_and_return_cr0();
	sys_call_table[__NR_open] = (unsigned long)orig_open;
	setback_cr0(orig_cr0);
 	netlink_release();  	
    printk(KERN_INFO "Module exit.\n");
}
module_init(audit_init);
module_exit(audit_exit);
