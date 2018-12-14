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
int main(int argc, char*argv[], char *envp[]){
    //execve("/bin/sh",argv,envp);
    execve("/root/TestAudit",argv,envp);
    char buf[0x10];
    int fileFd=open("./test",O_RDWR|O_CREAT,0664);
    read(fileFd,buf,0x10);
    write(1,buf,0x10);//SYSCALL TEST
    lseek(fileFd,0,SEEK_SET);
    read(fileFd,buf,0x10);
    write(1,buf,0x10);//SYSCALL TEST
    close(fileFd);

    int creatFd=creat("/root/TestAuditCreat",0664);
    write(creatFd,buf,0x10);//SYSCALL TEST
    close(creatFd);

    int rootFd=open("/root/TestAudit",O_RDWR);
    return -22;
}

