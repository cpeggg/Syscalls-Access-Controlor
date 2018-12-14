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
int main(int argc, char*argv[], char *envp[]){
    int fd,creatfd,execret;
    char buf[0x20];
    printf("\n----------------SYSCALL TEST----------------\n");
    printf("---------------PERSONAL INFO----------------\n");
    printf("UID:%d\t\tPID:%d\t\t\n",getuid(),getgid());
    printf("USERNAME:%s\t\tGROUPNAME:%s\t\t\n",getpwuid(getuid())->pw_name,getgrgid(getgid())->gr_name);
    printf("------------------IO TEST-------------------\n");
    fd = open("/tmp/ioTest",O_RDWR);
    if (fd<0)
        puts("open failed");
    else{
        puts("open success");
        if (read(fd, buf, 0x20)>0)
            puts("read success");
        else
            puts("read failed");
        if (printf("%s",buf)>0)
            puts("write success");
        else
            puts("write failed");
    }
    creatfd=creat("/tmp/creatTest",0777);
    if (creatfd<0)
        puts("creat failed");
    else
        puts("creat success");
    execret=execve("/tmp/execTest",NULL,NULL);
    if (execret)
        puts("execve failed");
    else
        puts("execve success");
    return 0;
}

