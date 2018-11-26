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

asmlinkage ssize_t (* orig_read)(int fd, void *buf, size_t count);
asmlinkage ssize_t (* orig_write)(int fd, const void *buf, size_t count);
asmlinkage int (* orig_open)(const char *pathname, int flags, mode_t mode);
asmlinkage int (* orig_close)(int fd);
asmlinkage off_t (* orig_lseek)(int fd, off_t offset, int whence);
asmlinkage int (* orig_execve)(const char *filename, char *const argv[], char *const envp[]);
asmlinkage int (* orig_creat)(const char *pathname, mode_t mode);
asmlinkage int (* orig_openat)(int dirfd, const char *pathname, int flags, mode_t mode);

asmlinkage ssize_t hook_read(int fd, void *buf, size_t count){
    printk(KERN_DEBUG "hook_read");
    return orig_read(fd, buf, count);
}
asmlinkage ssize_t hook_write(int fd, const void *buf, size_t count){
    printk(KERN_DEBUG"hook_write");
    return orig_write(fd, buf, count);
}
asmlinkage long hook_open(const char *pathname, int flags, mode_t mode)
{
	long ret;
    char buf[MAX_LENGTH]={0};
    printk(KERN_DEBUG"hook_open");
    if( pathname == NULL ) return -1;
    // To secure from crash (SMAP protect)
    copy_from_user(buf,pathname,MAX_LENGTH);
	ret = orig_open(pathname, flags, mode);
  	AuditOpen(buf,flags,ret);
  	return ret; 
}
asmlinkage long hook_close(int fd){
    printk(KERN_DEBUG"hook_close");
    return orig_close(fd);
}
asmlinkage off_t hook_lseek(int fd, off_t offset, int whence){
    printk(KERN_DEBUG"hook_lseek");
    return orig_lseek(fd, offset, whence);
}
asmlinkage long hook_execve(const char *filename, char *const argv[], char *const envp[]){
    printk(KERN_DEBUG"hook_execve");
    return orig_execve(filename, argv, envp);
}
asmlinkage long hook_creat(const char *pathname, mode_t mode){
    printk(KERN_DEBUG"hook_creat");
    return orig_creat(pathname, mode);
}
asmlinkage long hook_openat(int dirfd, const char *pathname, int flags, mode_t mode){
    printk(KERN_DEBUG"hook_openat");
    return orig_openat(dirfd, pathname, flags, mode);
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

	sys_call_table = get_sys_call_table();
    
    //Hook Sys Call Open
	orig_open = (void*)sys_call_table[__NR_open];
    sys_call_table[__NR_open] = (unsigned long)&hook_open;
    //Hook Sys Call Read
    orig_read = (void*)sys_call_table[__NR_read];
    sys_call_table[__NR_read] = (unsigned long)&hook_read;
    //Hook Sys Call Write
    orig_write = (void*)sys_call_table[__NR_write];
    sys_call_table[__NR_write] = (unsigned long)&hook_write;
    //Hook Sys Call Close
    orig_close = (void*)sys_call_table[__NR_close];
    sys_call_table[__NR_close] = (unsigned long)&hook_close;
    //Hook Sys Call Lseek
    orig_lseek = (void*)sys_call_table[__NR_lseek];
    sys_call_table[__NR_lseek] = (unsigned long)&hook_lseek;
    //Hook Sys Call Execve
    orig_execve = (void*)sys_call_table[__NR_execve];
    sys_call_table[__NR_execve] = (unsigned long)&hook_execve;
    //Hook Sys Call Creat
    orig_creat = (void*)sys_call_table[__NR_creat];
    sys_call_table[__NR_creat] = (unsigned long)&hook_creat;
    //Hook Sys Call Openat
    orig_openat = (void*)sys_call_table[__NR_openat];
    sys_call_table[__NR_openat] = (unsigned long)&hook_openat;

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
	sys_call_table[__NR_read] = (unsigned long)orig_read;
    sys_call_table[__NR_write] = (unsigned long)orig_write;
    sys_call_table[__NR_close] = (unsigned long)orig_close;
    sys_call_table[__NR_lseek] = (unsigned long)orig_lseek;
    sys_call_table[__NR_execve] = (unsigned long)orig_execve;
    sys_call_table[__NR_creat] = (unsigned long)orig_creat;
    sys_call_table[__NR_openat] = (unsigned long)orig_openat;

    setback_cr0(orig_cr0);
 	netlink_release();  	
    printk(KERN_INFO "Module exit.\n");
}
module_init(audit_init);
module_exit(audit_exit);
