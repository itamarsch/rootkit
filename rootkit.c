// hello_world.c
#include <linux/kprobes.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World Kernel Module");


struct kretprobe kp;

static int syscall_entry(struct kretprobe_instance* k , struct pt_regs* regs){
  struct pt_regs* syscall_regs = (struct pt_regs*)regs->di;

  int syscall_number = *(int*) &syscall_regs->orig_ax;
  /* printk(KERN_INFO "Syscal ran: %d\n",syscall_number); */
  if (syscall_number == 83) {
    printk(KERN_INFO "Mkdir ran: %d\n",syscall_number);
  }

  /* char* __user pathname = (char* )syscall_regs->di; */
  /* char buf[256]; */
  /* long n = strncpy_from_user(buf, pathname, 256); */
  /* printk(KERN_INFO "%ld %s\n",n,buf); */
  /* if (strcmp(buf,"hello") == 0) { */
  /*   int n = copy_to_user(pathname, "hi", 3); */
  /*   printk(KERN_INFO "Copying hi :) %d\n",n); */
  /* } */

  

  return 0;
}

static int syscall_exit(struct kretprobe_instance* k , struct pt_regs* regs) {
  return 0;
}

static int __init hello_world_init(void) {
  kp.kp.symbol_name = "x64_sys_call";
  kp.data_size = 0;
  kp.maxactive = 1000;

  kp.entry_handler = &syscall_entry;
  kp.handler = &syscall_exit;


  if(register_kretprobe(&kp) != 0) {
    printk(KERN_INFO "Failed kprobing on mkdir");
  }

  return 0; 
}

static void __exit hello_world_exit(void) {

  unregister_kretprobe(&kp);

  printk(KERN_INFO "Goodbye, world! The module is being unloaded.\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
