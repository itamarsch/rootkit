#include <linux/kprobes.h>
#include <linux/module.h>

#pragma once

enum syscall { UNKNOWN, KILL, OPEN, GET_DENTS };

struct unknown_metadata {};

struct open_metadata {
  bool should_fail;
};

struct get_dents_metadata {
  struct linux_dirent64 *__user entries;
  unsigned int buf_size;
};

union metadata {
  struct unknown_metadata unknown_metadata;
  struct open_metadata open_metadata;
  struct get_dents_metadata get_dents_metadata;
};

struct syscall_metadata {
  enum syscall syscall;
  union metadata metadata;
};

void free_syscall_metadata(struct syscall_metadata syscall_metadata);

void free_syscall_metadata(struct syscall_metadata syscall_metadata) {
  switch (syscall_metadata.syscall) {
  case OPEN:
  case KILL:
  case UNKNOWN:
  case GET_DENTS:
    break;
  }
}
