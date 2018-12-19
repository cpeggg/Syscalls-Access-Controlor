#include <linux/string.h>
#include <linux/mm.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/fdtable.h>

#define TASK_COMM_LEN 16
#define NETLINK_TEST 29
#define AUDITPATH "/root/TestAudit"
#define AUDITCREATPATH "/root/TestAuditCreat"
#define CONTENTAUDIT "USER_SECRET"
#define MAX_LENGTH 256
static u32 pid=0;
static struct sock *nl_sk = NULL;

//发送netlink消息message
int netlink_sendmsg(const void *buffer, unsigned int size)  
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int len = NLMSG_SPACE(1200);
	if((!buffer) || (!nl_sk) || (pid == 0)) 	return 1;
	skb = alloc_skb(len, GFP_ATOMIC); 	//分配一个新的sk_buffer
	if (!skb){
		printk(KERN_ERR "net_link: allocat_skb failed.\n");
		return 1;
	}
	nlh = nlmsg_put(skb,0,0,0,1200,0);
#ifdef _X86_
	NETLINK_CB(skb).pid = 0;      /* from kernel */
#else
    NETLINK_CB(skb).portid = 0;
#endif
	//下面必须手动设置字符串结束标志\0，否则用户程序可能出现接收乱码
	memcpy(NLMSG_DATA(nlh), buffer, size);
	//使用netlink单播函数发送消息
	if( netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT) < 0){
	//如果发送失败，则打印警告并退出函数
		printk(KERN_ERR "net_link: can not unicast skb \n");
		return 1;
	}
	return 0;
}
void get_fullname(const char *pathname,char *fullname)
{
	struct dentry *tmp_dentry = current->fs->pwd.dentry;
	char tmp_path[MAX_LENGTH];
	char local_path[MAX_LENGTH];
	memset(local_path,0,MAX_LENGTH);

	if (*pathname == '/') {
		strncpy(fullname,pathname,256);
		return;
	}

	while (tmp_dentry != NULL)
	{
		if (!strcmp(tmp_dentry->d_iname,"/"))
			break;
        snprintf(tmp_path,256,"/%s%s",tmp_dentry->d_iname,local_path);
		strncpy(local_path,tmp_path,256);

		tmp_dentry = tmp_dentry->d_parent;
	}
	snprintf(fullname, 256, "%s/%s", local_path, pathname);
	return;
}
int AuditWrite(const char* content, int fd, size_t count, ssize_t ret, int ACret){
    char commandname[TASK_COMM_LEN];
    unsigned int size;
    void* buffer;
    if (strcmp(current->comm, "syscall_test") )
        return 1;
    strncpy(commandname, current->comm, TASK_COMM_LEN);
    size = 8 + strlen(content) + 16 + TASK_COMM_LEN + 1;
    buffer = kzalloc(size, 0);
    *((int*)buffer) = 1;
    *((int*)buffer + 1) = current->cred->uid.val;
    *((int*)buffer + 2) = current->pid;
    *((int*)buffer + 3) = fd;
    *((int*)buffer + 4) = ret;
    *((int*)buffer + 5) = count;
    strcpy( (char*)( 6 + (int*)buffer  ), commandname );
    strcpy( (char*)( 6 + TASK_COMM_LEN/4 +(int*)buffer  ), content );
    netlink_sendmsg(buffer, size);
    kfree(buffer);
    return 0;
}
int AuditRead(const char* content, int fd, size_t count, ssize_t ret, int ACret){
    char commandname[TASK_COMM_LEN];
    unsigned int size;
    void* buffer;
    void*needle;
/*
    char *tmp;
    char *pathname;
    struct file* file;
    struct path* path;
*/
    //in case the root/user process related to read dmesg
    if (current->cred->uid.val==0 || !strcmp(current->comm, "in:imklog") || !strcmp(current->comm, "gnome-terminal-") || !(needle=strstr(content,CONTENTAUDIT))) 
        return 1;
    
    /*
    file = fcheck_files(current->files, fd);
    if (!file) return -ENOENT;
    path = &file->f_path;
    path_get(path);
    tmp = (char *)__get_free_page(GFP_KERNEL);
    if (!tmp) {
            path_put(path);
            return -ENOMEM;
    }
    pathname = d_path(path, tmp, PAGE_SIZE);
    path_put(path);
    printk(KERN_DEBUG"pathname: %s",pathname);
    if (IS_ERR(pathname)) {
            free_page((unsigned long)tmp);
            return PTR_ERR(pathname);
    }
    // do something here with pathname 
    free_page((unsigned long)tmp);
    */





    strncpy(commandname,current->comm,TASK_COMM_LEN);
	size = 8 + strlen(content) + 16 + TASK_COMM_LEN + 1;
	buffer = kzalloc(size, 0);
    *((int*)buffer) = 0;
    *((int*)buffer + 1) = current->cred->uid.val;
    *((int*)buffer + 2) = current->pid;
    *((int*)buffer + 3) = fd;
    *((int*)buffer + 4) = ret;
	*((int*)buffer + 5) = count;
    strcpy( (char*)( 6 + (int*)buffer  ), commandname );
    strcpy( (char*)( 6 + TASK_COMM_LEN/4 +(int*)buffer  ), content );
	netlink_sendmsg(buffer, size);
    kfree(buffer);
    
    return 0;
}
int AuditExecve(const char *filename, char *const argv[],char *const envp[], int ret, int ACret){
    char commandname[TASK_COMM_LEN];
    char fullname[256]={0};
    unsigned int size;
    void* buffer;
    get_fullname(filename,fullname);
    if (strncmp(fullname,AUDITPATH,15) != 0) return 1;
	
	strncpy(commandname,current->comm,TASK_COMM_LEN);
	size = 4 + strlen(fullname) + 16 + TASK_COMM_LEN + 1;
	buffer = kzalloc(size, 0);
    *((int*)buffer) = 59;
    *((int*)buffer + 1) = current->cred->uid.val;
    *((int*)buffer + 2) = current->pid;
    *((int*)buffer + 3) = ret;
    strcpy( (char*)( 4 + (int*)buffer  ), commandname );
    strcpy( (char*)( 4 + TASK_COMM_LEN/4 +(int*)buffer  ), fullname );
	netlink_sendmsg(buffer, size);
    kfree(buffer);
    return 0;
}

int AuditOpen(const char *pathname,int flags, int ret, int ACret)
{
	char commandname[TASK_COMM_LEN];
	char fullname[256];
    unsigned int size;   // = strlen(pathname) + 32 + TASK_COMM_LEN;
    void * buffer; // = kmalloc(size, 0);     
	memset(fullname, 0, 256);
	get_fullname(pathname, fullname);
    // Access control
    if (strncmp(fullname,AUDITPATH,15) != 0) return 1; 
    // Security Audition
	strncpy(commandname,current->comm,TASK_COMM_LEN);
	size = 4 + strlen(fullname) + 16 + TASK_COMM_LEN + 1;
	buffer = kzalloc(size, 0);
    *((int*)buffer) = 2;
    *((int*)buffer + 1) = current->cred->uid.val;
    *((int*)buffer + 2) = current->pid;
    *((int*)buffer + 3) = flags;
    *((int*)buffer + 4) = ret;
    strcpy( (char*)( 5 + (int*)buffer  ), commandname );
    strcpy( (char*)( 5 + TASK_COMM_LEN/4 +(int*)buffer  ), fullname );
	netlink_sendmsg(buffer, size);
    kfree(buffer);
    return 0;
}

int AuditCreat(const char*pathname, mode_t mode, int ret, int ACret){
    char commandname[TASK_COMM_LEN];
    char fullname[256];
    unsigned int size;
    void * buffer;
    memset(fullname, 0,256);
    get_fullname(pathname, fullname);
    if (strncmp(fullname, AUDITCREATPATH, 15) !=0 ) return 1;
    strncpy(commandname, current->comm, TASK_COMM_LEN);
    size = 4 + strlen(fullname) + 16 + TASK_COMM_LEN + 1;
    buffer = kzalloc(size, 0);
    *((int*)buffer) = 85;
    *((int*)buffer + 1) = current->cred->uid.val;
    *((int*)buffer + 2) = current->pid;
    *((int*)buffer + 3) = mode;
    *((int*)buffer + 4) = ret;
    strcpy( (char*)( 5 + (int*)buffer  ), commandname );
    strcpy( (char*)( 5 + TASK_COMM_LEN/4 +(int*)buffer  ), fullname );
    netlink_sendmsg(buffer, size);
    kfree(buffer);
    return 0;
}
void nl_data_ready(struct sk_buff *__skb)
 {
	struct sk_buff *skb;
        struct nlmsghdr *nlh;
	skb = skb_get (__skb);
//	nlh = (struct nlmsghdr *)skb->data;

	if (skb->len >= NLMSG_SPACE(0)) {
		 nlh = nlmsg_hdr(skb);
//		if( pid != 0 ) printk("Pid != 0 \n ");
		pid = nlh->nlmsg_pid; /*pid of sending process */ 
		//printk("net_link: pid is %d, data %s:\n", pid, (char *)NLMSG_DATA(nlh));
		printk("net_link: pid is %d\n", pid);
		kfree_skb(skb);	
	}
	return;
}






void netlink_init(void) {
#ifdef _X86_
    nl_sk = netlink_kernel_create(&init_net,NETLINK_TEST, 0, nl_data_ready,NULL,THIS_MODULE);
#else
    struct netlink_kernel_cfg cfg = {
        .input = nl_data_ready,
    };
    nl_sk = netlink_kernel_create(&init_net,NETLINK_TEST, &cfg);
#endif
    if (!nl_sk) 
	{
		printk(KERN_ERR "net_link: Cannot create netlink socket.\n");
		if (nl_sk != NULL)
    		sock_release(nl_sk->sk_socket);
	}	else  printk("net_link: create socket ok.\n");
}

void netlink_release(void) {
    if (nl_sk != NULL)  
 		sock_release(nl_sk->sk_socket);
}
