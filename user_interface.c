/*************************************************************************
	> File Name: user_interface.c
	> Author: 
	> Mail: 
	> Created Time: Wed 26 Dec 2018 08:53:27 PM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <malloc.h>
#include <unistd.h>
struct accesscontrolList{
    struct accesscontrolList* next;
    char programname[256];
    unsigned int syscall;
    unsigned int fpFlag;
    char string[256];
};
struct accesscontrolList programs[256]={0};
unsigned int programTop=0;
char* filecontent=NULL;
int parse(char *ptr){
	char* base, *end;
    unsigned int syscall,fpFlag;
    char id[256];
	char str[256];
    int i;
    base=ptr;
    end=strstr(ptr,"\n");
    *end='\0';
    ptr=end+1;
    //printk("parse: %s",base);
    if (!strstr(base,"Access Control Configuration File"))
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
        if (4!=sscanf(base,"%256s %u %u %256s",id,&syscall,&fpFlag,str)){
            if (!strstr(base,"#")){
                programTop=0;
                return -2;
            }
            else continue;
        }
        strncpy(programs[programTop].programname,id,256);
        programs[programTop].syscall=syscall;
        programs[programTop].fpFlag=fpFlag;
        strncpy(programs[programTop++].string,str,256);
    }while (end);
    return 0;
}
void parsefile(char* path){
    FILE* confs;
    
    /*read original configuration file*/
    filecontent=malloc(0x1000);
    memset(filecontent,0,0x1000);
    confs=fopen(path,"r");
    if (confs==NULL){
        perror("fopen");
        exit(-1);
    }
    if (fread(filecontent,1,0x1000,confs)<0){
        perror("fread");
        exit(-1);
    }
    if (parse(filecontent)<0){
        printf("PARSEFILE ERROR");
        exit(-2);
    }
    free(filecontent);
    filecontent=NULL;
    fclose(confs);
}
void writeback(char* path){
    FILE* confs;
    int i;
    confs=fopen(path,"w");
    if (confs==NULL){
        perror("fopen");
        exit(-1);
    }
    fwrite("Access Control Configuration File\n",1,strlen("Access Control Configuration File\n"),confs);
    fwrite("# program_name syscall_num forbid/permit(0/1) string\n",1,strlen("# program_name syscall_num forbid/permit(0/1) string\n"),confs);
    fwrite("# ex: 1001 0 0 /tmp/ioTest\n",1,strlen("# ex: 1001 0 0 /tmp/ioTest\n"),confs);
    for (i=0;i<programTop;i++)
        fprintf(confs,"%s %u %u %s\n",programs[i].programname,programs[i].syscall, programs[i].fpFlag,programs[i].string);
}
char* stringg(int syscall){
    switch (syscall){
        case __NR_open:
        case __NR_read:return "Audit content";
        case __NR_write:
        case __NR_execve:
        case __NR_creat:return "Audit path";
        default: puts("syscall error");return NULL;
    }
}
char* fpFlagtoStr(int flag){
    return (flag==1)?"Permit":"Forbid";
}
int StrtofpFlag(char* str){
    if (str[0]=='p'||str[0]=='P')
        return 1;
    return 0;
}
char* syscallnumtoStr(int syscall){
    switch (syscall){
        case __NR_open:return "open";
        case __NR_read:return "read";
        case __NR_write:return "write";
        case __NR_execve:return "execve";
        case __NR_creat:return "creat";
        default:puts("syscall error");return NULL;
    }
}
int Strtosyscallnum(char* str){
    switch (str[0]){
        case 'o':case 'O':return __NR_open;
        case 'r':case 'R':return __NR_read;
        case 'w':case 'W':return __NR_write;
        case 'e':case 'E':return __NR_execve;
        case 'c':case 'C':return __NR_creat;
        default:puts("syscall error");return -1;
    }
}
void Show(){
    int i;
    puts("Current policy:");
    for (i=0;i<programTop;i++){
        printf("Policy %d:\n",i);
        printf("\tProgram Name: %s\n",programs[i].programname);
        printf("\tSyscall: %s\n",syscallnumtoStr(programs[i].syscall));
        printf("\tPermit/Forbid: %s\n",fpFlagtoStr(programs[i].fpFlag));
        printf("\t%s: %s\n", stringg(programs[i].syscall), programs[i].string);
    }
}
void Add(){
    char buf[0x20];
    if (programTop>255) {puts("Policy pool full.");return;}
    puts("Input the Program Name:");
    scanf("%256s",programs[programTop].programname);
    puts("Input the Syscall(open/read/write/execve/creat)");
    scanf("%20s",buf);
    programs[programTop].syscall=Strtosyscallnum(buf);
    puts("Is it permitted or forbiddened?(p/f)");
    scanf("%20s",buf);
    programs[programTop].fpFlag=StrtofpFlag(buf);
    printf("Input the %s:\n",stringg(programs[programTop].syscall));
    scanf("%256s",programs[programTop++].string);
}
void Delete(){
    char buf[0x10];
    int index,i;
    do{
        puts("Input the index you want to delete:");
        scanf("%10s",buf);
        index=atoi(buf);
        if (index>=0 && index<programTop)
            break;
        printf("Invalid Index");
    }while(1);
    for (i=index;i<programTop;i++){
        strcpy(programs[i].programname,programs[i+1].programname);
        programs[i].syscall=programs[i+1].syscall;
        programs[i].fpFlag=programs[i+1].fpFlag;
        strcpy(programs[i].string,programs[i+1].string);
    }
    programTop--;
}
int main(){
    char buf[0x10];
    int choice;
    /*parse file*/
    parsefile("/root/AccessControl.conf");
    /*user interface*/
    do{
        printf("\n\n------------Access Control User Interface---------------\n\n");
        printf("1. Show current policy\n");
        printf("2. Add policy\n");
        printf("3. Delete policy\n");
        printf("4. Write back and exit.\n");
        printf("Input your choice:");
        scanf("%10s",buf);
        choice=atoi(buf);
        switch (choice){
            case 1:Show();break;
            case 2:Add();break;
            case 3:Delete();break;
            case 4:break;
            default:puts("Invalid choice.");
        }
    }while (choice!=4);
    /*write back configuration*/    
    writeback("/root/AccessControl.conf");
    printf("writeback success.\n");
}
