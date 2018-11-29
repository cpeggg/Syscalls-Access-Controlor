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

#define TM_FMT "%Y-%m-%d %H:%M:%S"

#define NETLINK_TEST 29
#define MAX_PAYLOAD 1024  /* maximum payload size*/
int sock_fd;
struct msghdr msg;
struct nlmsghdr *nlh = NULL;
struct sockaddr_nl src_addr, dest_addr;
struct iovec iov;

FILE *logfile;

void Log(char *commandname,int uid, int pid, char *file_path, int flags,int ret,int syscall)
{
	char logtime[64];
	char username[32];
	struct passwd *pwinfo;
	time_t t=time(0);
    //Used for open
    char openresult[10];
    char opentype[16];


	if (logfile == NULL)	return;
	pwinfo = getpwuid(uid);
	strcpy(username,pwinfo->pw_name);
	strftime(logtime, sizeof(logtime), TM_FMT, localtime(&t) );

	switch (syscall){
    case 0:
        break;
    case 1:
        break;
    case 2:
	   
	    
	    if (ret > 0) strcpy(openresult,"success");
	        else strcpy(openresult,"failed");
        if (flags & O_WRONLY ) strcpy(opentype, "Write");
	        else if (flags & O_RDWR ) strcpy(opentype, "Read/Write");
    	        else strcpy(opentype,"Read");
        fprintf(logfile,"%s(%d) %s(%d) %s \"%s\" %s %s\n",username,uid,commandname,pid,logtime,file_path,opentype, openresult);
        printf("%s(%d) %s(%d) %s \"%s\" %s %s\n",username,uid,commandname,pid,logtime,file_path,opentype, openresult);
        break;
    case 9:
        break;
    case 10:
        break;
    case 59:
        break;
    case 85:
        break;
    case 216:
        break;
    case 257:
        break;
    }

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
    return ;
}
void LogWrite(struct nlmsghdr *nlh){
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
    Log(commandname, uid,pid, file_path,flags,ret,2);
    return ;
}
void LogMmap(struct nlmsghdr *nlh){
    return ;
}
void LogMprotect(struct nlmsghdr *nlh){
    return ;
}
void LogExecve(struct nlmsghdr *nlh){
    return ;
}
void LogCreat(struct nlmsghdr *nlh){
    return ;
}
void LogRemapFilePages(struct nlmsghdr *nlh){
    return ;
}
void LogOpenat(struct nlmsghdr *nlh){
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
                printf("[*]DEBUG: LogRead()\n");
                LogRead(nlh);
                break;
            case 1:
                printf("[*]DEBUG: LogWrite()\n");
                LogWrite(nlh);
                break;
            case 2:
                printf("[*]DEBUG: LogOpen()\n");
                LogOpen(nlh);
                break;
            case 9:
                printf("[*]DEBUG: LogMmap()\n");
                LogMmap(nlh);
                break;
            case 10:
                printf("[*]DEBUG: LogMprotect()\n");
                LogMprotect(nlh);
                break;
            case 59:
                printf("[*]DEBUG: LogExecve()\n");
                LogExecve(nlh);
                break;
            case 85:
                printf("[*]DEBUG: LogCreat()\n");
                LogCreat(nlh);
                break;
            case 216:
                printf("[*]DEBUG: LogRemapFilePages()\n");
                LogRemapFilePages(nlh);
                break;
            case 257:
                printf("[*]DEBUG: LogOpenat()\n");
                LogOpenat(nlh);
                break;
            default:
                printf("[-]ERROR: wrong packet format, NLMSG_DATA(nlh): ");
                write(1,NLMSG_DATA(nlh),0x100);
                break;
        }
      
    }
	close(sock_fd);
	free(nlh);
	fclose(logfile);
	return 0;
}


