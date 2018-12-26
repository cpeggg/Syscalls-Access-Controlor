/*************************************************************************
	> File Name: test.c
	> Author: cpegg
	> Mail: cpeggg@gmail.com
	> Created Time: Sun 25 Nov 2018 11:41:01 PM PST
 ************************************************************************/
// Used for test each syscall's functionality
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#define IOTEST "/tmp/ioTest"
#define EXECTEST "/tmp/execTest"
#define CREATEST "/tmp/creatTest"
int main(int argc, char*argv[], char *envp[]){
    int fd,creatfd,execret;
    char buf[0x20];
    char strProcessPath[1024]={0};
    char* strProcessName;
    printf("\n----------------SYSCALL TEST----------------\n");
    printf("---------------PERSONAL INFO----------------\n");
	if(readlink("/proc/self/exe", strProcessPath,1024) <=0)
        return -1;
	strProcessName = strrchr(strProcessPath, '/');
    printf("Current Process Name: %s\n",++strProcessName);
    printf("------------------IO TEST-------------------\n");

    fd = open(IOTEST,O_RDWR|O_CREAT, 0777);
    if (fd<0)
        puts("open failed");
    else{
        puts("open success");
        if (read(fd, buf, 0x20)>=0)
            puts("read success");
        else
            puts("read failed");
        if (printf("%s",buf)>0)
            puts("write success");
        else
            puts("write failed");
    }
    creatfd=creat(CREATEST, 0777);
    if (creatfd<0)
        puts("creat failed");
    else
        puts("creat success");
    execret=execve(EXECTEST,NULL,NULL);
    if (execret)
        puts("execve failed");
    else
        puts("execve success");
    return 0;
}

