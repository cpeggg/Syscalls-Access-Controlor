/*************************************************************************
	> File Name: auditdaemon_ref.c
	> Author: 
	> Mail: 
	> Created Time: Wed 28 Nov 2018 09:43:27 PM PST
 ************************************************************************/

#include<stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <fcntl.h>
#include <asm/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pwd.h>
#include <stdarg.h>
#define TM_FMT "%Y-%m-%d %H:%M:%S"

#define NETLINK_TEST 29
#define MAX_PAYLOAD 1024  /* maximum payload size*/
int sock_fd;
struct msghdr msg;
struct nlmsghdr *nlh = NULL;
struct sockaddr_nl src_addr, dest_addr;
struct iovec iov;

FILE *logfile;

void Log(char *commandname,int uid, int pid, int syscall, ...)//char *file_path, int flags,int ret)
{
	char logtime[64];
	char username[32];
	struct passwd *pwinfo;
	time_t t=time(0);
    int ret;
	//Used for read
	char *content;
    int count;
    int fd;
    //Used for open
    char openresult[20];
    char opentype[16];
    char* file_path;
    int flags;
    //Used for execve
    char *execve_path;
    //Used for creat;
    int mode;

	va_list pArgs;  //定义va_list类型的指针pArgs，用于存储参数地址
    va_start(pArgs, syscall); //初始化pArgs指针，使其指向第一个可变参数。该宏第二个参数是变参列表的前一个参数，即最后一个固定参数
   
    //若函数有多个可变参数，则依次调用va_arg宏获取各个变参
    

	if (logfile == NULL)	return;
	pwinfo = getpwuid(uid);
	strcpy(username,pwinfo->pw_name);
	strftime(logtime, sizeof(logtime), TM_FMT, localtime(&t) );

	switch (syscall){
    case 0:
        fd = va_arg(pArgs, int);
        content = va_arg(pArgs, char*);
        count = va_arg(pArgs, int);
        ret = va_arg(pArgs, int);
		if (ret > 0) snprintf(openresult, 20, "success, fd=%d", fd);
	        else strcpy(openresult,"failed");
        fprintf(logfile,"%s(%d) %s(%d) %s \"%s\" %d %s\n",username,uid,commandname,pid,logtime,content,count, openresult);
        printf("%s(%d) %s(%d) %s \"%s\" %d %s\n",username,uid,commandname,pid,logtime,content,count, openresult);
        break;	
    case 1:
        fd = va_arg(pArgs, int);
        content = va_arg(pArgs, char*);
        count = va_arg(pArgs, int);
        ret = va_arg(pArgs, int);
        if (ret > 0) snprintf(openresult, 20, "success, fd=%d", fd);
            else strcpy(openresult,"failed");
        fprintf(logfile,"%s(%d) %s(%d) %s \"%s\" %d %s\n",username,uid,commandname,pid,logtime,content,count, openresult);
        printf("%s(%d) %s(%d) %s \"%s\" %d %s\n",username,uid,commandname,pid,logtime,content,count, openresult);
        break;
    case 2:
        file_path = va_arg(pArgs, char*);
        flags = va_arg(pArgs, int); 
        ret = va_arg(pArgs, int);
	    if (ret > 0) snprintf(openresult, 20, "success, fd=%d", ret);
	        else strcpy(openresult,"failed");
        if (flags & O_WRONLY ) strcpy(opentype, "Write");
	        else if (flags & O_RDWR ) strcpy(opentype, "Read/Write");
    	        else strcpy(opentype,"Read");
        fprintf(logfile,"%s(%d) %s(%d) %s \"%s\" %s %s\n",username,uid,commandname,pid,logtime,file_path,opentype, openresult);
        printf("%s(%d) %s(%d) %s \"%s\" %s %s\n",username,uid,commandname,pid,logtime,file_path,opentype, openresult);
        break;
    case 59:
        execve_path = va_arg(pArgs, char*);
        ret = va_arg(pArgs, int);
        if (ret > 0) strcpy(openresult,"success");
	        else strcpy(openresult,"failed");
		fprintf(logfile,"%s(%d) %s(%d) %s \"%s\" %s\n",username,uid,commandname,pid,logtime,execve_path, openresult);
        printf("%s(%d) %s(%d) %s \"%s\" %s\n",username,uid,commandname,pid,logtime,execve_path, openresult);	
        break;
    case 85:
        file_path = va_arg(pArgs, char*);
        mode = va_arg(pArgs, int);
        ret = va_arg(pArgs, int);
        if (ret > 0) snprintf(openresult, 20, "success, fd=%d", ret);
            else strcpy(openresult, "failed");
        fprintf(logfile,"%s(%d) %s(%d) %s \"%s\" %d %s\n",username,uid,commandname,pid,logtime,file_path,mode, openresult);
        printf("%s(%d) %s(%d) %s \"%s\" %d %s\n",username,uid,commandname,pid,logtime,file_path,mode, openresult);

        break;
    }
	va_end(pArgs);  //将指针pArgs置为无效，结束变参的获取
}


void sendpid(unsigned int pid)
{
	//Send message to initialize
	memset(&msg, 0, sizeof(msg));
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = pid;  //self pid
	src_addr.nl_groups = 0;  //not in mcast groups
	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;   //For Linux Kernel
	dest_addr.nl_groups = 0; //unicast

	/* Fill the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = pid;  /* self pid */
	nlh->nlmsg_flags = 0;
	/* Fill in the netlink message payload */
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	//printf(" Sending message. ...\n");
	sendmsg(sock_fd, &msg, 0);
}

void killdeal_func()
{
	printf("The process is killed! \n");
	close(sock_fd);
	if (logfile != NULL)
		fclose(logfile);
	if (nlh != NULL)
	 	free(nlh);
	exit(0);
}

void intdeal_func(){
    printf("The process is interrupted! \n");
    close(sock_fd);
    if (logfile != NULL)
        fclose(logfile);
    if (nlh != NULL)
        free(nlh);
    exit(0);
}
void LogRead(struct nlmsghdr *nlh){
    unsigned int uid, pid, ret, count,fd;
    char* content;
    char* commandname;
    uid = *( 1 + (unsigned int *)NLMSG_DATA(nlh)  );
    pid = *( 2 + (int *)NLMSG_DATA(nlh)   );
    fd = *(3 + (int *)NLMSG_DATA(nlh));
    ret = *( 4 + (int *)NLMSG_DATA(nlh)   );
    count = *( 5 + (int *)NLMSG_DATA(nlh) );
    commandname = (char *)( 6 + (int *)NLMSG_DATA(nlh) );
    content = (char *)( 6 + 16/4 + (int *)NLMSG_DATA(nlh) );
    Log(commandname, uid,pid, 0, fd, content, count, ret);
    return ;
}
void LogWrite(struct nlmsghdr *nlh){
    unsigned int uid, pid, ret, count, fd;
    char* content;
    char* commandname;
    uid = *( 1 + (unsigned int *)NLMSG_DATA(nlh)  );
    pid = *( 2 + (int *)NLMSG_DATA(nlh)  );
    fd = *(3 + (int *)NLMSG_DATA(nlh));
    ret = *( 4 + (int *)NLMSG_DATA(nlh)   );
    count = *( 5 + (int *)NLMSG_DATA(nlh) );
    commandname = (char *)( 6 + (int *)NLMSG_DATA(nlh) );
    content = (char *)( 6 + 16/4 + (int *)NLMSG_DATA(nlh) );
    Log(commandname, uid,pid, 1, fd, content, count, ret);
    return ;
}
void LogOpen(struct nlmsghdr *nlh){
    unsigned int uid, pid,flags,ret;
    char *file_path;
    char *commandname;
    uid = *( 1 + (unsigned int *)NLMSG_DATA(nlh)  );
    pid = *( 2 + (int *)NLMSG_DATA(nlh)   );
    flags = *( 3 + (int *)NLMSG_DATA(nlh)   );
    ret = *( 4 + (int *)NLMSG_DATA(nlh)   );
    commandname = (char *)( 5 + (int *)NLMSG_DATA(nlh) );
    file_path = (char *)( 5 + 16/4 + (int *)NLMSG_DATA(nlh) );
    Log(commandname, uid,pid, 2, file_path,flags,ret);
    return ;
}
void LogExecve(struct nlmsghdr *nlh){
    unsigned int uid, pid, ret;
    char *file_path;
    char *commandname;
    uid = *( 1 + (unsigned int *)NLMSG_DATA(nlh)  );
    pid = *( 2 + (int *)NLMSG_DATA(nlh)   );
    ret = *( 3 + (int *)NLMSG_DATA(nlh)   );
    commandname = (char *)( 4 + (int *)NLMSG_DATA(nlh) );
    file_path = (char *)( 4 + 16/4 + (int *)NLMSG_DATA(nlh) );
    Log(commandname, uid,pid, 59, file_path, ret);
    return ;
}

void LogCreat(struct nlmsghdr *nlh){
    unsigned int uid, pid, ret,mode;
    char *file_path;
    char *commandname;
    uid = *( 1 + (unsigned int *)NLMSG_DATA(nlh)  );
    pid = *( 2 + (int *)NLMSG_DATA(nlh)   );
    mode = *( 3 + (int *)NLMSG_DATA(nlh)   );
    ret = *( 4 + (int *)NLMSG_DATA(nlh)   );
    commandname = (char *)( 5 + (int *)NLMSG_DATA(nlh)  );
    file_path = (char *)( 5 + 16/4 + (int *)NLMSG_DATA(nlh)  );
    Log(commandname, uid,pid, 85, file_path, mode, ret);
    return ;
}

int main(int argc, char *argv[]){
	char buff[110];
	//void killdeal_func();
	char logpath[32];
	if (argc == 1) strcpy(logpath,"./log");
	else if (argc == 2) strncpy(logpath, argv[1],32);
	else {
		printf("commandline parameters error! please check and try it! \n");
		exit(1);
	}
	

	signal(SIGTERM,killdeal_func);
    signal(SIGINT,intdeal_func);
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    
    sendpid(getpid());

	/* open the log file at the begining of daemon, in case of this operation causes deadlock */
	logfile=fopen(logpath, "w+");
	if (logfile == NULL) {
		printf("Waring: can not create log file\n");
		exit(1);
	}
	//Loop to get message
	while(1)	{	//Read message from kernel
		unsigned int uid, pid,flags,ret;
		char * file_path;
		char * commandname;
        int syscall=-1;
		recvmsg(sock_fd, &msg, 0);
        syscall = *( (unsigned int *)NLMSG_DATA(nlh)  );
        switch (syscall){
            case 0:
                printf("\n[*]DEBUG: LogRead()\n");
                LogRead(nlh);
                break;
            case 1:
                printf("\n[*]DEBUG: LogWrite()\n");
                LogWrite(nlh);
                break;
            case 2:
                printf("\n[*]DEBUG: LogOpen()\n");
                LogOpen(nlh);
                break;
            case 59:
                printf("\n[*]DEBUG: LogExecve()\n");
                LogExecve(nlh);
                break;
            case 85:
                printf("\n[*]DEBUG: LogCreat()\n");
                LogCreat(nlh);
                break;
            default:
                printf("\n[-]ERROR: wrong packet format, NLMSG_DATA(nlh): ");
                write(1,NLMSG_DATA(nlh),0x100);
                break;
        }
      
    }
	close(sock_fd);
	free(nlh);
	fclose(logfile);
	return 0;
}


