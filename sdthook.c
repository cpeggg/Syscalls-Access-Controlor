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
#define EACDENIED -

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cpegg");
MODULE_DESCRIPTION("A simple example Linux module leart from Robert W. Oliver II");
MODULE_VERSION("0.01");

void netlink_release(void);
void netlink_init(void);
extern int AuditOpen(const char *pathname, int flags, int ret, int ACret);
extern int AuditExecve(const char *filename, char *const argv[],char *const envp[], int ret, int ACret);
extern int AuditRead(const char* content, int fd, size_t count, ssize_t ret, int ACret); 
extern int AuditWrite(const char* content, int fd, size_t count, ssize_t ret, int ACret);
extern int AuditCreat(const char* pathname, mode_t mode, ssize_t ret, int ACret);

extern int ACRead(int fd, void* buf, size_t count);
extern int ACOpen(const char*filename, int flags, mode_t mode);
extern int ACExecve(const char* filename, char *const argv[], char* const envp[]);
extern int ACWrite(int fd, const void* buf, size_t count);
extern int ACCreat(const char * pathname, mode_t mode);

extern int parsemain(const char* path);

void *get_sys_call_table(void);
#ifdef _X86_
unsigned int clear_and_return_cr0(void);
void setback_cr0(unsigned int val);
#else
unsigned long clear_and_return_cr0(void);
void setback_cr0(unsigned long val);
#endif
unsigned long * sys_call_table=NULL;

struct access_control_list{
    long syscall_num;
    long grant;//you can make use of every bit
    char* substring;
};
struct user_access_control{
    unsigned int uid;
    struct access_control_list access_list;
};
struct group_access_control{
    unsigned int gid;
    struct access_control_list access_list;
};
struct user_access_control* user_access_control=NULL;
struct group_access_control* group_access_control=NULL;

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
    int ACret=0;
    void *bufferSDTHook=kzalloc(count,GFP_ATOMIC);

    ret = orig_read(fd, buf, count);
    if (ret>=0)
        copy_from_user(bufferSDTHook, buf, ret); // to avoid copy \0 to userspace when ret < count
    ACret = ACRead(fd, bufferSDTHook, count);
    if (ACret)
        ret = -1;
    AuditRead(bufferSDTHook,fd,count,ret, ACret);
    kfree(bufferSDTHook);
    return ret;
}
asmlinkage ssize_t hook_write(int fd, const void *buf, size_t count){
    void *bufferSDTHook=kzalloc(count,GFP_ATOMIC);
    ssize_t ret;
    int ACret=0;
    copy_from_user(bufferSDTHook, buf, count);
    ACret = ACWrite(fd, bufferSDTHook, count);
    if (ACret)
        ret = -1;
    else
        ret = orig_write(fd, buf, count);
    // Apply content analysis
    AuditWrite(bufferSDTHook,fd,count,ret, ACret);
    kfree(bufferSDTHook);
    return ret;
}
asmlinkage long hook_open(const char *pathname, int flags, mode_t mode)
{
	long ret;
    void *bufferSDTHook=kzalloc(MAX_LENGTH,GFP_ATOMIC);
    int ACret=0;
    if( pathname == NULL ) return -1; 
    // To secure from crash (SMAP protect)
    copy_from_user(bufferSDTHook,pathname,MAX_LENGTH);
	// Time for access control
    ACret = ACOpen(bufferSDTHook, flags, mode);
    if (ACret)
        ret = -1;
    else 
        ret = orig_open(pathname, flags, mode);
  	AuditOpen(bufferSDTHook,flags,ret, ACret);
    kfree(bufferSDTHook);
  	return ret; 
}
asmlinkage long hook_execve(const char *filename, char *const argv[], char *const envp[]){
    long ret;
    int ACret=0;
    void *bufferSDTHook=kzalloc(MAX_LENGTH,GFP_ATOMIC);

    if( filename == NULL ) return -1; 
    // To secure from crash (SMAP protect)
    copy_from_user(bufferSDTHook,filename,MAX_LENGTH);
	// Time for access control
    ACret = ACExecve(bufferSDTHook, argv, envp);
    if (ACret)
        ret = -1;
    else
        ret = orig_execve(filename, argv, envp);
    AuditExecve(bufferSDTHook, argv, envp, ret, ACret);
    kfree(bufferSDTHook);
    return ret;
}
asmlinkage long hook_creat(const char *pathname, mode_t mode){
    long ret;
    int ACret=0;
    void *bufferSDTHook=kzalloc(MAX_LENGTH,GFP_ATOMIC);
    if (pathname == NULL) return -1;
    copy_from_user(bufferSDTHook,pathname,MAX_LENGTH);
    ACret = ACCreat(bufferSDTHook, mode);
    if (ACret)
        ret = -1;
    else
        ret = orig_creat(pathname, mode);
    AuditCreat(bufferSDTHook, mode, ret, ACret);
    kfree(bufferSDTHook);
    return ret;
}

static int __init audit_init(void)
{
#ifdef _X86_
    unsigned int orig_cr0;
#else
    unsigned long orig_cr0;
#endif   
    int parseret=0;
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
    //Hook Sys Call Execve
    orig_execve = (void*)sys_call_table[__NR_execve];
    sys_call_table[__NR_execve] = (unsigned long)&hook_execve;
    //Hook Sys Call Creat
    orig_creat = (void*)sys_call_table[__NR_creat];
    sys_call_table[__NR_creat] = (unsigned long)&hook_creat;
    setback_cr0(orig_cr0);
	
    //Initialize netlink
    netlink_init();
    parseret = parsemain("/root/AccessControl.conf");
    if (parseret == -1)
        printk("Configure file not found.\n");
    else if (parseret==-2)
        printk("configure file format wrong.\n");
    else printk("Configure file parse success.\n");
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
    sys_call_table[__NR_execve] = (unsigned long)orig_execve;
    sys_call_table[__NR_creat] = (unsigned long)orig_creat;
    
    setback_cr0(orig_cr0);
 	netlink_release();  	
    printk(KERN_INFO "Module exit.\n");
}
module_init(audit_init);
module_exit(audit_exit);
