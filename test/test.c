/*************************************************************************
	> File Name: test.c
	> Author: 
	> Mail: 
	> Created Time: Thu 01 Nov 2018 02:54:02 AM PDT
 ************************************************************************/
#include <linux/module.h>  /* Needed by all kernel modules */
#include <linux/kernel.h>  /* Needed for loglevels (KERN_WARNING, KERN_EMERG, KERN_INFO, etc.) */
#include <linux/init.h>    /* Needed for __init and __exit macros. */
#include <linux/moduleparam.h>
#include <linux/unistd.h>  /* sys_call_table __NR_* system call function indices */
#include <linux/fs.h>      /* filp_open */
#include <linux/slab.h>    /* kmalloc */

#include <asm/paravirt.h> /* write_cr0 */
//#include <asm/uaccess.h>  /* get_fs, set_fs */
#include <linux/utsname.h>
#include <asm/cacheflush.h>
#include <linux/semaphore.h>
#include <linux/kallsyms.h>

#define PROC_V    "/proc/version"
#define BOOT_PATH "/boot/System.map-"
#define MAX_VERSION_LEN   256
MODULE_LICENSE("GPL");
unsigned long *syscall_table = NULL;
asmlinkage int (*original_open)(const char *pathname, int flags, int mode);

asmlinkage int new_open(const char *pathname, int flags, int mode){
        printk(KERN_ALERT "[+] open() hooked.");
        return original_open(pathname, flags, mode);

}
void *find_sys_call_table(void){
    static unsigned long *p_sys_call_table;
    /* Aquire system calls table address */
    p_sys_call_table = (unsigned long *) kallsyms_lookup_name("sys_call_table");
    return p_sys_call_table;
}
static int __init moduleInit(void){
    syscall_table=find_sys_call_table();
    printk(KERN_EMERG "Syscall table address: %p\n", syscall_table);
    if (syscall_table != NULL) {
                write_cr0 (read_cr0 () & (~ 0x10000));
                original_open = (void *)syscall_table[__NR_open];
                syscall_table[__NR_open] = (unsigned long)&new_open;
                write_cr0 (read_cr0 () | 0x10000);
                printk(KERN_EMERG "[+] onload: sys_call_table hooked\n");
            
    } else {
                printk(KERN_EMERG "[-] onload: syscall_table is NULL\n");
            
    }
        return 0;

}

static void __exit moduleClear(void){
    if (syscall_table != NULL) {
                write_cr0 (read_cr0 () & (~ 0x10000));
                syscall_table[__NR_open] = (unsigned long)original_open;
                write_cr0 (read_cr0 () | 0x10000);
                printk(KERN_EMERG "[+] onunload: sys_call_table unhooked\n");
            
    } else {
                printk(KERN_EMERG "[-] onunload: syscall_table is NULL\n");
            
    }


}

module_init(moduleInit);
module_exit(moduleClear);
