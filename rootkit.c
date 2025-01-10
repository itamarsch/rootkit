// hello_world.c
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/dirent.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>    // {clear,set}_pte_bit(), set_pte()
#include "asm/unistd_64.h"
#include "get_kln.h"

// Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World Kernel Module");

typedef asmlinkage long (mkdir_t)(char* __user, umode_t);

mkdir_t* original_mkdir;
kln_p k;  
void** table;

static asmlinkage long my_mkdir(char* __user name, umode_t mode) {
  printk(KERN_INFO "My mkdir!!\n");

  return 0;
}


static void disable_page_protection(void) {

    unsigned long value;
    asm volatile("mov %%cr0,%0" : "=r" (value));
    if (value & 0x00010000) {
            value &= ~0x00010000;
            asm volatile("mov %0,%%cr0": : "r" (value));
    }
}

static void enable_page_protection(void) {

    unsigned long value;
    asm volatile("mov %%cr0,%0" : "=r" (value));
    if (!(value & 0x00010000)) {
            value |= 0x00010000;
            asm volatile("mov %0,%%cr0": : "r" (value));
    }
}

static int __init hello_world_init(void) {
  
  k =  get_kln_p();
  table = (void**) k("sys_call_table");


  printk(KERN_INFO "%p, %p",table[__NR_mkdir], &my_mkdir);

  original_mkdir = table[__NR_mkdir];
  disable_page_protection(); 
  table[__NR_mkdir] = (void*) &my_mkdir; 
  enable_page_protection(); 
  printk(KERN_INFO "%p",table[__NR_mkdir]);
  ((mkdir_t*) table[__NR_mkdir])("",0);

  printk("TABLE %p", original_mkdir);

  return 0; 
}

static void __exit hello_world_exit(void) {


  disable_page_protection(); 
  table[__NR_mkdir] = (unsigned long*) original_mkdir; 
  enable_page_protection(); 
  printk(KERN_INFO "Goodbye, world! The module is being unloaded.\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
