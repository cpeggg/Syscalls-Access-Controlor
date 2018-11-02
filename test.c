/*************************************************************************
	> File Name: test.c
	> Author: 
	> Mail: 
	> Created Time: Thu 01 Nov 2018 04:16:40 AM PDT
 ************************************************************************/

#include<stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

void callsys(void){
    system("/bin/sh");
}
int main()
{
    char buf[0x20];
    int fd=open("123",O_RDONLY);
    read(fd,buf,0x20);
    write(1,buf,0x20);
    printf("\n%p\n",callsys);
    printf("\n%p\n",&callsys);
    printf("%d\n",EACCES);
    return 0;
}
