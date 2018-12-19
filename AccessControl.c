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

#define CONTROLSTR "USER_SECRET"
#define IOTEST "/tmp/ioTest"
#define CREATEST "/tmp/creatTest"
#define EXECTEST "/tmp/execTest"
#define MAX_LENGTH 256
extern void get_fullname(const char *pathname, char* fullname);
struct accesscontrolList{
    struct accesscontrolList* next;
    unsigned int id;
    unsigned int syscall;
    unsigned int fpFlag;
    char string[256];
};
extern struct accesscontrolList users[256];
extern unsigned int userTop;
extern struct accesscontrolList groups[256];
extern unsigned int groupTop;
int getuserAC(unsigned int syscall, unsigned int *flag, const char* filename){
    int i;
    char fullname[256]={0};
    *flag=0;
    for (i=0;i<userTop;i++){
        if (current->cred->uid.val==users[i].id && users[i].syscall==syscall){
            get_fullname(filename, fullname);
            if (strstr(fullname, users[i].string)){
                *flag=1;
                return users[i].fpFlag-1;
            }
        }
    }
    return 0;
}
int getgroupAC(unsigned int syscall, unsigned int *flag, const char *filename){
    int i;
    char fullname[256]={0};
    *flag=0;
    for (i=0;i<groupTop;i++){
        if (current->cred->gid.val==groups[i].id && groups[i].syscall==syscall){
            get_fullname(filename, fullname);
            if (strstr(fullname, groups[i].string)){
                *flag=1;
                return groups[i].fpFlag-1;
            }
        }
    }
    return 0;
}
int ACRead(int fd, void *buf, size_t count){
    int ret, hasresult;
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->uid.val==1003 && strstr(buf,CONTROLSTR))
        return -1;
    if (userTop>0){
        ret = getuserAC(0, &hasresult, buf);
        if (hasresult) return ret;
    }
    if (groupTop>0){
        ret = getgroupAC(0, &hasresult, buf);
        if (hasresult) return ret;
    }
    return 0;
}
int ACWrite(int fd, const void *buf, size_t count){
    int ret, hasresult;
    if (current->cred->uid.val==1004)
        return 0;
    if (current->cred->uid.val==1003 && strstr(buf,CONTROLSTR))
        return -1;
    if (userTop>0){
        ret = getuserAC(1, &hasresult, buf);
        if (hasresult) return ret;
    }
    if (groupTop>0){
        ret = getgroupAC(1, &hasresult, buf);
        if (hasresult) return ret;
    }
    return 0;
}
int ACOpen(const char* pathname, int flags, mode_t mode){
    char fullname[256];
    int ret, hasresult;
    if (current->cred->uid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, IOTEST))
            return -1;
    }
    if (current->cred->uid.val==1004)
        return 0;
    if (userTop>0){
        ret = getuserAC(2, &hasresult, pathname);
        if (hasresult) return ret;
    }
    if (current->cred->gid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, IOTEST))
            return -1;
    }
    if (groupTop>0){
        ret = getgroupAC(2, &hasresult, pathname);
        if (hasresult) return ret;
    }
    return 0;
}
int ACExecve(const char *filename, char *const argv[], char* const envp[]){
    char fullname[256];
    int ret, hasresult;
    if (current->cred->uid.val==1001){
        get_fullname(filename, fullname);
        if (strstr(fullname, EXECTEST))
            return -1;
    }
    if (current->cred->uid.val==1004)
        return 0;
    if (userTop>0){
        ret = getuserAC(59, &hasresult, filename);
        if (hasresult) return ret;
    }
    if (current->cred->gid.val==1001){
        get_fullname(filename, fullname);
        if (strstr(fullname, EXECTEST))
            return -1;
    }
    if (groupTop>0){
        ret = getgroupAC(59, &hasresult, filename);
        if (hasresult) return ret;
    }
    return 0;
}
int ACCreat(const char* pathname, mode_t mode){
    char fullname[256];
    int ret, hasresult;
    if (current->cred->uid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, CREATEST))
            return -1;
    }
    if (current->cred->uid.val==1004)
        return 0;
    if (userTop>0){
        ret = getuserAC(85, &hasresult, pathname);
        if (hasresult) return ret;
    }
    if (current->cred->gid.val==1001){
        get_fullname(pathname, fullname);
        if (strstr(fullname, CREATEST))
            return -1;
    }
    if (groupTop>0){
        ret = getgroupAC(85, &hasresult, pathname);
        if (hasresult) return ret;
    }
    return 0;
}
