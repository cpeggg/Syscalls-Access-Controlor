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
#include <linux/slab.h>
#define MAX_LENGTH 256
#define BUFFER_INIT_SIZE 0x10000

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
asmlinkage void* (* orig_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
asmlinkage int (* orig_mprotect)(void *addr, size_t len, int prot);
asmlinkage int (* orig_execve)(const char *filename, char *const argv[], char *const envp[]);
asmlinkage int (* orig_creat)(const char *pathname, mode_t mode);
asmlinkage int (* orig_openat)(int dirfd, const char *pathname, int flags, mode_t mode);
asmlinkage int (* orig_remap_file_pages)(void *addr, size_t size, int prot, size_t pgoff, int flags);



asmlinkage ssize_t hook_read(int fd, void *buf, size_t count){
    ssize_t ret;
    void *bufferSDTHook=kzalloc(count,GFP_ATOMIC);
    ret = orig_read(fd, buf, count);
    if (ret>=0)
        copy_from_user(bufferSDTHook, buf, count); // to avoid copy \0 to userspace when ret < count
    // To Apply analysis
    // ...
    kfree(bufferSDTHook);
    return ret;
}
asmlinkage ssize_t hook_write(int fd, const void *buf, size_t count){
    void *bufferSDTHook=kzalloc(count,GFP_ATOMIC);
    copy_from_user(bufferSDTHook, buf, count);
    // Apply content analysis
    // ...
    kfree(bufferSDTHook);
    return orig_write(fd, buf, count);
}
asmlinkage long hook_open(const char *pathname, int flags, mode_t mode)
{
	long ret;
    void *bufferSDTHook=kzalloc(MAX_LENGTH,GFP_ATOMIC);
    if( pathname == NULL ) return -1;
    
    // To secure from crash (SMAP protect)
    copy_from_user(bufferSDTHook,pathname,MAX_LENGTH);
	ret = orig_open(pathname, flags, mode);
  	AuditOpen(bufferSDTHook,flags,ret);
    kfree(bufferSDTHook);
  	return ret; 
}
asmlinkage void* hook_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset){
    // Apply var analysis
    return orig_mmap(addr, length, prot, flags, fd, offset);
}
asmlinkage long hook_mprotect(void *addr, size_t len, int prot){
    // Apply var analysis
    return orig_mprotect(addr, len, prot);
}
asmlinkage long hook_execve(const char *filename, char *const argv[], char *const envp[]){
    void *bufferSDTHook=kzalloc(MAX_LENGTH,GFP_ATOMIC);
    copy_from_user(bufferSDTHook,filename,MAX_LENGTH);
    kfree(bufferSDTHook);
    return orig_execve(filename, argv, envp);
}
asmlinkage long hook_creat(const char *pathname, mode_t mode){
    void *bufferSDTHook=kzalloc(MAX_LENGTH,GFP_ATOMIC);
    copy_from_user(bufferSDTHook,pathname,MAX_LENGTH);
    kfree(bufferSDTHook);
    return orig_creat(pathname, mode);
}
asmlinkage long hook_openat(int dirfd, const char *pathname, int flags, mode_t mode){
    void *bufferSDTHook=kzalloc(MAX_LENGTH,GFP_ATOMIC);
    copy_from_user(bufferSDTHook,pathname,MAX_LENGTH);
    kfree(bufferSDTHook);
    return orig_openat(dirfd, pathname, flags, mode);
}
asmlinkage long hook_remap_file_pages(void *addr, size_t size, int prot, size_t pgoff, int flags){
    return orig_remap_file_pages(addr, size, prot, pgoff, flags);
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
    //Hook Sys Call Mmap 
    orig_mmap = (void*)sys_call_table[__NR_mmap];
    sys_call_table[__NR_mmap] = (unsigned long)&hook_mmap;
    //Hook Sys Call Mprotect
    orig_mprotect = (void*)sys_call_table[__NR_mprotect];
    sys_call_table[__NR_mprotect] = (unsigned long)&hook_mprotect;
    //Hook Sys Call Execve
    orig_execve = (void*)sys_call_table[__NR_execve];
    sys_call_table[__NR_execve] = (unsigned long)&hook_execve;
    //Hook Sys Call Creat
    orig_creat = (void*)sys_call_table[__NR_creat];
    sys_call_table[__NR_creat] = (unsigned long)&hook_creat;
    //Hook Sys Call Openat
    orig_openat = (void*)sys_call_table[__NR_openat];
    sys_call_table[__NR_openat] = (unsigned long)&hook_openat;
    //Hook Sys Call Remap_file_pages
    orig_remap_file_pages = (void*)sys_call_table[__NR_remap_file_pages];
    sys_call_table[__NR_remap_file_pages] = (unsigned long)&hook_remap_file_pages;

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
    sys_call_table[__NR_mmap] = (unsigned long)orig_mmap;
    sys_call_table[__NR_mprotect] = (unsigned long)orig_mprotect;
    sys_call_table[__NR_execve] = (unsigned long)orig_execve;
    sys_call_table[__NR_creat] = (unsigned long)orig_creat;
    sys_call_table[__NR_openat] = (unsigned long)orig_openat;
    sys_call_table[__NR_remap_file_pages] = (unsigned long)orig_remap_file_pages;

    setback_cr0(orig_cr0);
 	netlink_release();  	
    printk(KERN_INFO "Module exit.\n");
}
module_init(audit_init);
module_exit(audit_exit);
