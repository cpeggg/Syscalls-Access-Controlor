/*************************************************************************
	> File Name: test.c
	> Author: 
	> Mail: 
	> Created Time: Sun 25 Nov 2018 11:41:01 PM PST
 ************************************************************************/
// Used for test each syscall's functionality
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(){
    char buf[0x10];
    int fileFd=open("./test",O_RDWR|O_CREAT,0664);
    read(fileFd,buf,0x10);
    write(1,buf,0x10);//SYSCALL TEST
    lseek(fileFd,0,SEEK_SET);
    read(fileFd,buf,0x10);
    write(1,buf,0x10);//SYSCALL TEST
    close(fileFd);

    int creatFd=creat("./test_creat",0664);
    write(creatFd,buf,0x10);//SYSCALL TEST
    close(creatFd);

    int rootFd=open("/root/TestAudit",O_RDWR);

    return 0;
}

