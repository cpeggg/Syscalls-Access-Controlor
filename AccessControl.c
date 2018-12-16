/*************************************************************************
	> File Name: AccessContro.c
	> Author: 
	> Mail: 
	> Created Time: Fri 14 Dec 2018 03:55:28 PM CST
 ************************************************************************/
/*return 0: Granted
  return -1: Forbiddened*/
#include <linux/string.h>
#include <linux/mm.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/fdtable.h>
#define CONTROLSTR "USER's SECRET"

int ACRead(int fd, void *buf, size_t count){
    printk(KERN_DEBUG"ACRead");
    
    if (current->cred->uid.val==1001)
        return -1;
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001)
        return -1;
    if (current->cred->uid.val==1003 && strstr(buf,CONTROLSTR))
        return -1;
    return 0;
}
int ACWrite(int fd, const void *buf, size_t count){
    printk(KERN_DEBUG"ACWrite");
    if (current->cred->uid.val==1001)
        return -1;
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001)
        return -1;
    if (current->cred->uid.val==1003 && strstr(buf,CONTROLSTR))
        return -1;

    return 0;
}
int ACOpen(const char* pathname, int flags, mode_t mode){
    printk(KERN_DEBUG"ACOpen");
    if (current->cred->uid.val==1001)
        return -1;
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001)
        return -1;

    return 0;
}
int ACExecve(const char *filename, char *const argv[], char* const envp[]){

    printk(KERN_DEBUG"ACExecve.");
    if (current->cred->uid.val==1001)
        return -1;
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001)
        return -1;
    return 0;
}
int ACCreat(const char* pathname, mode_t mode){
    printk(KERN_DEBUG"ACCreat");
    if (current->cred->uid.val==1001)
        return -1;
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001)
        return -1;
    return 0;
}
