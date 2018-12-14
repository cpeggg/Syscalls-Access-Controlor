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
    printf("----------------SYSCALL TEST----------------\n");
    printf("---------------PERSONAL INFO----------------\n");
    printf("UID:%d\t\tPID:%d\t\t\n",getuid(),getgid());
    printf("USERNAME:%s\t\tGROUPNAME:%s\t\t\n",getpwuid(getuid())->pw_name,getgrgid(getgid())->gr_name);
    printf("--------------------------------------------\n");
    return 0;
}

