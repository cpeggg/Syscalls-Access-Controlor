#include <linux/string.h>
#include <linux/mm.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>

#define TASK_COMM_LEN 16
#define NETLINK_TEST 29
#define AUDITPATH "/root/TestAudit"
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
	memset(tmp_path,0,MAX_LENGTH);
	memset(local_path,0,MAX_LENGTH);

	if (*pathname == '/') {
		strcpy(fullname,pathname);
		return;
	}

	while (tmp_dentry != NULL)
	{
		if (!strcmp(tmp_dentry->d_iname,"/"))
			break;
		strcpy(tmp_path,"/");
		strcat(tmp_path,tmp_dentry->d_iname);
		strcat(tmp_path,local_path);
		strcpy(local_path,tmp_path);

		tmp_dentry = tmp_dentry->d_parent;
	}
	strcpy(fullname,local_path);
	strcat(fullname,"/");
	strcat(fullname,pathname);
	return;
}

int AuditOpen(const char *pathname,int flags, int ret)
{
	char commandname[TASK_COMM_LEN];
	char fullname[256];
   unsigned int size;   // = strlen(pathname) + 32 + TASK_COMM_LEN;
   void * buffer; // = kmalloc(size, 0);     
	memset(fullname, 0, 256);
	get_fullname(pathname, fullname);

   if (strncmp(fullname,AUDITPATH,15) != 0) return 1; 

	strncpy(commandname,current->comm,TASK_COMM_LEN);

	size = strlen(fullname) + 16 + TASK_COMM_LEN + 1;
	buffer = kmalloc(size, 0);
	memset(buffer, 0, size);
   *((int*)buffer) = current->cred->uid.val;
   *((int*)buffer + 1) = current->pid;
   *((int*)buffer + 2) = flags;
   *((int*)buffer + 3) = ret;
   strcpy( (char*)( 4 + (int*)buffer ), commandname);
   strcpy( (char*)( 4 + TASK_COMM_LEN/4 +(int*)buffer ), fullname);

	netlink_sendmsg(buffer, size);
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
