#include <linux/module.h>

#pragma once

enum syscall { UNKNOWN, KILL, OPEN, GET_DENTS };

struct unknown {};

struct open_data {
  bool should_fail;
};

struct getdents_data {
  struct linux_dirent64 *__user dirents;
  unsigned int buf_size;
};

union syscall_data {
  struct unknown unknown;
  struct open_data open_data;
  struct getdents_data getdents_data;
};

struct rootkit_cmd {
  enum syscall syscall;
  union syscall_data data;
};

// void free_rootkit_cmd(struct rootkit_cmd cmd);
// void free_rootkit_cmd(struct rootkit_cmd cmd) {
//   switch (cmd.syscall) {
//   case OPEN:
//   case KILL:
//   case UNKNOWN:
//   case GET_DENTS:
//     break;
//   }
// }
