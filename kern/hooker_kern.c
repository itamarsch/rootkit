
#include "linux/printk.h"
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>

#include "./retrieve_kallsyms.c"

void make_function_writable(void *addr) {
  unsigned int cr0_register;

  asm volatile("mov %%cr0, %0" : "=r"(cr0_register));

  cr0_register &= ~(1 << 16);

  asm volatile("mov %0, %%cr0" : : "r"(cr0_register));
}

void modified(void) { printk(KERN_INFO "world\n"); }

void hello(void) { printk(KERN_INFO "Hello\n"); }

static int __init hooker_init(void) {

  kln_p kln_pointer = get_kln_p();
  printk(KERN_INFO "kln_pointer = 0x%lx\n", (unsigned long)kln_pointer);

  int (*set_memory_rox)(unsigned long addr, int numpages) =
      (int (*)(unsigned long addr, int numpages))kln_pointer("set_memory_rox");
  printk(KERN_INFO "set_memory_rox = 0x%lx\n", (unsigned long)set_memory_rox);

  void (*modified_addr)(void) = &modified;

  make_function_writable(modified_addr);

  memmove(modified_addr + 3, modified_addr, 0x19);
  printk(KERN_INFO "Move");

  void (*modified_addr_new)(void) = (void (*)(void))(modified_addr + 3);
  unsigned long base = (unsigned long)modified_addr_new & PAGE_MASK;
  int pages = PAGE_ALIGN(0x19) / PAGE_SIZE;

  if (set_memory_rox((unsigned long)base, pages) != 0) {
    pr_err("set_memory_ro failed\n");
    return 0;
  }

  pr_info("set_memory_rw succeeded\n");

  (modified_addr_new)();

  return 0;
}

static void __exit hooker_exit(void) { return; }

module_init(hooker_init);
module_exit(hooker_exit);

MODULE_LICENSE("GPL");
