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
#define IOTEST "/tmp/ioTest"
#define CREATEST "/tmp/creatTest"
#define EXECTEST "/tmp/execTest"
#define MAX_LENGTH 256
extern void get_fullname(const char *pathname, char* fullname);
int ACRead(int fd, void *buf, size_t count){

    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->uid.val==1003 && strstr(buf,CONTROLSTR))
        return -1;
    return 0;
}
int ACWrite(int fd, const void *buf, size_t count){
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->uid.val==1003 && strstr(buf,CONTROLSTR))
        return -1;

    return 0;
}
int ACOpen(const char* pathname, int flags, mode_t mode){
    char fullname[256];
    if (current->cred->uid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, IOTEST))
            return -1;
    }
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, IOTEST))
            return -1;
    }

    return 0;
}
int ACExecve(const char *filename, char *const argv[], char* const envp[]){
    char fullname[256];
    if (current->cred->uid.val==1001){
        get_fullname(filename, fullname);
        if (strstr(fullname, EXECTEST))
            return -1;
    }
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001){
        get_fullname(filename, fullname);
        if (strstr(fullname, EXECTEST))
            return -1;
    }
    return 0;
}
int ACCreat(const char* pathname, mode_t mode){
    char fullname[256];
    if (current->cred->uid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, CREATEST))
            return -1;
    }
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->gid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, CREATEST))
            return -1;
    }
    return 0;
}
