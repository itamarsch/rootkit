#include <linux/module.h>

void hideme(void);
void showme(void);
void hide_handler(struct pt_regs *syscall_regs);
