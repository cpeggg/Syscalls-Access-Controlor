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
    char programname[256];
    unsigned int syscall;
    unsigned int fpFlag;
    char string[256];
};
extern struct accesscontrolList programs[256];
extern unsigned int programTop;//indicate the amount of rules stored in memory

int getAC(unsigned int syscall, unsigned int *flag, const char* filename){
    int i;
    int ret=0;
    char fullname[256]={0};
    *flag=0;
    // go through every rules
    for (i=0;i<programTop;i++){
        //Check 1: syscall and programname
        if (programs[i].syscall==syscall && !strcmp(current->comm,programs[i].programname)){
            get_fullname(filename, fullname);
            //Check 2:String compare
            if (!strcmp(programs[i].string,"(All)") || strstr(fullname, programs[i].string)){
                *flag=1;
                return programs[i].fpFlag-1;//Since we store 0/1 in memory to indicate p and f, however this function return -1 as f and 0 as p, so we minus 1 as result
            }
        }
    }
    return ret;
}
int ACRead(int fd, void *buf, size_t count){
    int ret, hasresult;
    // If there has rules, go check it, else permission as result
    if (programTop>0){
        ret = getAC(0, &hasresult, buf);
        if (hasresult) return ret;
    }
    return 0;
}
int ACWrite(int fd, const void *buf, size_t count){
    int ret, hasresult;
    if (programTop>0){
        ret = getAC(1, &hasresult, buf);
        if (hasresult) return ret;
    }
    return 0;
}
int ACOpen(const char* pathname, int flags, mode_t mode){
    int ret, hasresult;
    if (programTop>0){
        ret = getAC(2, &hasresult, pathname);
        if (hasresult) return ret;
    }
    return 0;
}
int ACExecve(const char *filename, char *const argv[], char* const envp[]){
    int ret, hasresult;
    if (programTop>0){
        ret = getAC(59, &hasresult, filename);
        if (hasresult) return ret;
    }
    return 0;
}
int ACCreat(const char* pathname, mode_t mode){
    int ret, hasresult;
    if (programTop>0){
        ret = getAC(85, &hasresult, pathname);
        if (hasresult) return ret;
    }
    return 0;
}
