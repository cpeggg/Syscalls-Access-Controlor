#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kallsyms.h>

#ifdef _X86_
struct idt_descriptor
{
	unsigned short off_low;
	unsigned short sel; 
	unsigned char none, flags;
	unsigned short off_high;
};          
#else
struct idt_descriptor
{
    unsigned int off_low;
    unsigned int sel;
    unsigned char none, flags;
    unsigned int off_high;
};
#endif
#ifdef _X86_
void *get_system_call(void)
{
   unsigned char idtr[6];
   unsigned long base;
   struct idt_descriptor desc;
   asm ("sidt %0" : "=m" (idtr));
   base = *((unsigned long *) &idtr[2]);
   memcpy(&desc, (void *) (base + (0x80*8)), sizeof(desc));
   return((void*)(((int)desc.off_high << 16) + desc.off_low));
}
#endif
void *get_sys_call_table(void)
{
#ifdef _X86_
    void *system_call = get_system_call();
    unsigned char *p;
    unsigned long sct;
    int count = 0;
    p = (unsigned char *) system_call;
    while (!((*p == 0xff) && (*(p+1) == 0x14) && (*(p+2) == 0x85))){
        p++;
        if (count++ > 500) {
            count = -1;
            break;
        		}
    	   }
    if (count != -1){
        p += 3;
        sct = *((unsigned long *) p);
         }
	else     sct = 0;
   return((void *) sct);
#else
   static unsigned long *p_sys_call_table;
   /* Aquire system calls table address */
   p_sys_call_table = (unsigned long *) kallsyms_lookup_name("sys_call_table");
   return p_sys_call_table;
#endif
} 

// clear WP bit of CR0, and return the original value
unsigned int clear_and_return_cr0(void)
{
#ifdef _X86_
    unsigned int cr0 = 0;
    unsigned int ret;
	asm volatile ("movl %%cr0, %%eax":"=a"(cr0));
    ret = cr0;
    cr0 &= 0xfffeffff;
    asm volatile ("movl %%eax, %%cr0"::"a"(cr0));
#else
    unsigned long ret=0;
    unsigned long cr0=read_cr0();
    ret = cr0;
    cr0 &= (~0x10000);
    write_cr0(cr0);
    asm("cli;");

#endif
    return ret;	
}

// set CR0 with new value
#ifdef _X86_
void setback_cr0(unsigned int val)
{
	asm volatile ("movl %%eax, %%cr0" : : "a"(val));
}
#else
void setback_cr0(unsigned long val)
{
    asm("sti;");
    write_cr0(val);
}
#endif
