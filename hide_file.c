#include "syscall_metadata.h"
#include <linux/module.h>

#define NUM_STRINGS 1
#define MAX_LENGTH 20

char hidden_filenames[NUM_STRINGS][MAX_LENGTH] = {"secret_file.txt"};

static bool ends_with(const char *str, const char *suffix) {
  if (!str || !suffix) {
    return false; // Null pointers can't be checked
  }

  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  if (suffix_len > str_len) {
    return false; // Suffix is longer than the string
  }

  // Compare the end of the string with the suffix
  return strcmp(str + (str_len - suffix_len), suffix) == 0;
}

#define PATH_BUFFER_SIZE 1024

void open_enter_handler(char *__user filename,
                        struct syscall_metadata *syscall_metadat);
void open_enter_handler(char *__user filename,
                        struct syscall_metadata *syscall_metadata) {
  syscall_metadata->metadata.open_metadata.should_fail = false;

  char buf[PATH_BUFFER_SIZE];
  int n = strncpy_from_user(buf, filename, PATH_BUFFER_SIZE);
  if (n) {
    for (int i = 0; i < NUM_STRINGS; i++) {
      if (ends_with(buf, hidden_filenames[i])) {
        syscall_metadata->metadata.open_metadata.should_fail = true;
        printk(KERN_INFO "tried accessing file: %s\n", hidden_filenames[i]);
      }
    }
  }
}

void open_exit_handler(struct pt_regs *syscall_regs,
                       struct syscall_metadata syscall_metadata);
void open_exit_handler(struct pt_regs *syscall_regs,
                       struct syscall_metadata syscall_metadata) {
  if (syscall_metadata.metadata.open_metadata.should_fail) {
    syscall_regs->ax = -2;
  }
}
