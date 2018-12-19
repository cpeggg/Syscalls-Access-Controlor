/*************************************************************************
	> File Name: parseconf.c
	> Author: 
	> Mail: 
	> Created Time: Mon 17 Dec 2018 10:42:22 AM CST
 ************************************************************************/
#include <linux/fs.h>
//#include <asm/segment.h>
//#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/buffer_head.h>
struct accesscontrolList{
    struct accesscontrolList* next;
    unsigned int id;
    unsigned int syscall;
    unsigned int fpFlag;
    char string[256];
};
struct accesscontrolList users[256]={0};
unsigned int userTop=0;
struct accesscontrolList groups[256]={0};
unsigned int groupTop=0;
int read_conf(const char *path, char* filecontent) 
{
    struct file *filp = NULL;
    unsigned long long offset = 0;
    int ret = -1;
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(get_ds());
    //mm_segment_t can secure system from accessing data in kernel memory
    //vfs_read require filecontent to be a userspace pointer
    filp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(filp)) {
        return -1;
    }
    ret = vfs_read(filp, filecontent, 0x1000, &offset);
    if (ret<0){
        return -1;
    }
    set_fs(oldfs);
    return ret;
}
int parse(char* ptr){
    char* base, *end;
    unsigned int id,syscall,fpFlag;
    char str[256];
    int u_or_g=0;
    int i;
    base=ptr;
    end=strstr(ptr,"\n");
    *end='\0';
    ptr=end+1;
    //printk("parse: %s",base);
    if (!strstr(base,"users ac"))
        return -2;
    do {
        base=ptr;
        end=strstr(ptr,"\n");
        if (!end) break;
        *end='\0';
        ptr=end+1;
        //printk("parse: %s",base);
        if (*base=='\0')
            break;
        if (4!=sscanf(base,"%u %u %u %256s",&id,&syscall,&fpFlag,str)){
            if (strstr(base,"groups ac")){
                u_or_g=1;
                continue;
            }
            else if (!strstr(base,"#")){
                userTop=0;
                groupTop=0;
                return -2;
            }
            else continue;
        }
        if (u_or_g==0){
            users[userTop].id=id;
            users[userTop].syscall=syscall;
            users[userTop].fpFlag=fpFlag;
            strncpy(users[userTop++].string,str,256);
        }
        else{
            groups[groupTop].id=id;
            groups[groupTop].syscall=syscall;
            groups[groupTop].fpFlag=fpFlag;
            strncpy(groups[groupTop++].string,str,256);
        }
    }while (end);
    printk("USER:");
    for (i=0;i<userTop;i++)
        printk("%u %u %u %s",users[i].id,users[i].syscall, users[i].fpFlag, users[i].string);
    printk("GROUPS:");
    for (i=0;i<groupTop;i++)
        printk("%u %u %u %s",groups[i].id,groups[i].syscall,groups[i].fpFlag,groups[i].string);
    return 0;
}
int parsemain(const char *path){
    char *filecontent=kzalloc(0x1000,GFP_ATOMIC);
    char *ptr=filecontent;
    int ret=-1;
    ret = read_conf(path, filecontent);
    //printk(KERN_DEBUG"filecontent: %s",filecontent);
    if (ret<0)
        return -1;
    else 
        return parse(ptr);
}
